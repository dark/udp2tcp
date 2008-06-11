/*
 udp2tcp
 Copyright (C) 2008 ~ Marco Leogrande
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "logging.h"
#include "udp2tcp.h"

/* global variables */
extern char *program_name;
extern log_t logger;

void print_usage(){
  fprintf(stderr,"FIXME\n");
}

int sock_createandbind(const int port){
  /* accept a valid port number (0-65535), returns the created listening socket */
  int sock;
  struct sockaddr_in server_data;
  const int enable_reuseaddr=1;

  if ( (sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
    F_ERROR("socket() failed with status %d: %s\n",sock, strerror(errno));
    exit(1);
  }

  if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable_reuseaddr, sizeof(enable_reuseaddr)) < 0){
    F_ERROR("setsockopt() failed: %s\n", strerror(errno));
    exit(1);
  }

  memset(&server_data, 0x0, sizeof(server_data));
  server_data.sin_family = PF_INET;
  server_data.sin_addr.s_addr = htonl(INADDR_ANY);
  server_data.sin_port = htons((uint16_t)port);

  if( bind (sock, (struct sockaddr*) &server_data,  sizeof(server_data)) != 0){
    F_ERROR("bind() failed: %s\n", strerror(errno));
    exit(1);
  }

  F_DEBUG("Input UDP socket bound successfully\n");

  return sock;
}

void loadTCPinfo(struct sockaddr** info, socklen_t* info_len, char* IP, int port){
  /* use struct sockaddr_in */
  struct sockaddr_in* local_info = malloc(sizeof(struct sockaddr_in));
  *info_len = sizeof(struct sockaddr_in);

  memset(local_info, 0x0, *info_len);

  local_info->sin_family = PF_INET;
  if( inet_pton(PF_INET, IP, &local_info->sin_addr) <=0 ){
    F_ERROR("inet_pton() failed: %s\n", strerror(errno));
    exit(1);
  }
  local_info->sin_port = htons((uint16_t)port);

  /* copy informations back to caller */
  *info = (struct sockaddr*)local_info;
}

const char* filter_ret2str(const filter_ret_t r){
  /* returns a reference to a statically-allocated buffer
     that describes the provided filter_ret_t */
  switch (r){
  case ERROR:
    return ("ERROR");
  case IN_BUF_IS_OK:
    return ("IN buffer OK");
  case OUT_BUF_STAT:
    return ("Static OUT buffer");
  case OUT_BUF_DYN:
    return ("Dynamic OUT buffer");
  }

  return ("APPLICATION BUG");
}
