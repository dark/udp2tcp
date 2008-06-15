/*
 udp2tcp
 Copyright (C) 2008 Marco Leogrande
 $Id$
 
 This file is part of udp2tcp.
 
 udp2tcp is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 udp2tcp is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "filters.h"
#include "logging.h"
#include "udp2tcp.h"

/* global variables */
extern char *program_name;
extern log_t logger;

/* == these are the functions of the child == */
int do_child(int udp_sock_fd,void *buf,size_t buflen,const struct sockaddr* sender, const socklen_t sender_l,
	     const struct sockaddr* recipient, const socklen_t recipient_l, filter_t filter){
  int tcp_sock_fd;
  /* input buffer */
  char local_buf[LOCAL_BUFFER_SIZE];
  ssize_t read_bytes;
  /* output buffer */
  void *out_buf=NULL;
  size_t out_buf_len=0;
  /* filter-specific ancillary data */
  void *extra_data=NULL;
  size_t extra_data_len=0;
  /* filter related data */
  filter_ret_t f_last_ret=IN_BUF_IS_OK;
  /* temporary pointer and length (they just hold references, no allocation is made) */
  void *tmp_ptr_to_data = NULL;
  ssize_t tmp_len_of_data = 0;

  F_DEBUG("This child handles the response for a %d byte(s) input buffer\n",buflen);

  /* apply filter to input data (if needed) */
  if(filter!=NULL){
    f_last_ret = filter(buf, buflen, &out_buf, &out_buf_len, &extra_data, &extra_data_len, UDP2TCP);
    F_DEBUG("Filter UDP2TCP exit: %d byte(s) IN, %d byte(s) OUT, %d extra byte(s), status: %s\n",
	    buflen, out_buf_len, extra_data_len, filter_ret2str(f_last_ret));

    switch (f_last_ret){
    case ERROR:
      F_WARN("UDP2TCP filter returned error, destroying child\n");
      return 1;

    case IN_BUF_IS_OK:
    case OUT_BUF_STAT:
    case OUT_BUF_DYN:
      /* no action is needed in these cases */
      break;
    }
  }

  /* open and connect TCP stream */
  if( (tcp_sock_fd=socket(PF_INET, SOCK_STREAM, 0)) < 0){
    F_ERROR("socket() failed: %s\n", strerror(errno));
    return 1;
  }
  if (connect(tcp_sock_fd, recipient, recipient_l)!=0){
    F_ERROR("connect() failed: %s\n", strerror(errno));
    return 1;
  }

  F_DEBUG("Successfully connected to the remote TCP endpoint\n");

  /* forward the received datagram over TCP */
  if ( write(tcp_sock_fd, 
	     (f_last_ret==IN_BUF_IS_OK? buf: out_buf),
	     (f_last_ret==IN_BUF_IS_OK? buflen: out_buf_len)
	     ) != (f_last_ret==IN_BUF_IS_OK? buflen: out_buf_len) ){
    F_ERROR("first write() failed: %s\n", strerror(errno));
    return 1;
  }

  /* while there is data, send it back */
  while( 1 ){
    /* read data */
  again:
    if ( (read_bytes=read(tcp_sock_fd, local_buf, LOCAL_BUFFER_SIZE)) <0 ){
      if (INTERRUPTED_BY_SIGNAL)
	goto again;
      else{
	F_ERROR("read() failed and returned %d: %s\n", read_bytes, strerror(errno));
	return read_bytes;
      }
    }

    if (read_bytes==0){
      F_DEBUG("EOF reached, closing connection\n");
      break;
    }
    F_DEBUG("%d bytes received from TCP\n", read_bytes);

    /* apply filter to data, if needed */
    if(filter!=NULL){
      if (f_last_ret==OUT_BUF_STAT){
	/* discard pointer value if was a statical reference */
	out_buf=NULL;
	out_buf_len=0;
      }

      f_last_ret = filter(local_buf, read_bytes, &out_buf, &out_buf_len, &extra_data, &extra_data_len, TCP2UDP);
      F_DEBUG("Filter TCP2UDP exit: %d byte(s) IN, %d byte(s) OUT, %d extra byte(s), status: %s\n",
	      read_bytes, out_buf_len, extra_data_len, filter_ret2str(f_last_ret));
      switch (f_last_ret){
      case ERROR:
	F_WARN("TCP2UDP filter returned error, destroying child\n");
	return 1;
	
      case IN_BUF_IS_OK:
	/* use data read directly from TCP */
	tmp_ptr_to_data=local_buf;
	tmp_len_of_data=read_bytes;
	break;

      case OUT_BUF_STAT:
      case OUT_BUF_DYN:
	/* use data filtered */
	tmp_ptr_to_data=out_buf;
	tmp_len_of_data=out_buf_len;
	break;
      }

    }
    else {
      /* if filtering is not needed, just forward the received data */
      tmp_ptr_to_data=local_buf;
      tmp_len_of_data=read_bytes;
    }

    /* send data back via UDP */
    if( sendto(udp_sock_fd, tmp_ptr_to_data, tmp_len_of_data, 0,
	       sender, sender_l) != tmp_len_of_data ) {
      F_ERROR("sendto() failed: %s\n", strerror(errno));
      return 1;
    }
    F_DEBUG("%d byte(s) successfully sent on UDP\n",tmp_len_of_data);
  }

  /* close connection and return successfully */
  if (close(tcp_sock_fd) != 0){
    F_WARN("close() failed: %s\n", strerror(errno));
    return 1;
  }
    
  return 0;
}
