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

#ifndef UDP2TCP_H_INCLUDED
#define UDP2TCP_H_INCLUDED

#include <errno.h>
#include <arpa/inet.h>

#define PROJECT_NAME "udp2tcp"
#define PROJECT_VER "0.0.1"
#define PROJECT_SIG PROJECT_NAME " v." PROJECT_VER ", by dark"

/* defines a dafe value for the network data handling buffers */
#define LOCAL_BUFFER_SIZE 1500
#define INTERRUPTED_BY_SIGNAL (errno == EINTR || errno == ECHILD)

/* types */
typedef enum{UDP2TCP,TCP2UDP} dir_t;
typedef enum{ERROR=-1, /* an error has occurred, data integrity and/or state info are undefined */
	     IN_BUF_IS_OK=0, /* data outputted by filter are equal to those at input, so just look at in_buf (out_buf data undefined) */
	     OUT_BUF_DYN=1, /* out_buf is dynamically allocated, so you can free it if you like (edit out_buf_len, in that case) */
 	     OUT_BUF_STAT=2 /* out_buf is statically allocated (don't ask), so use it and then discard the pointer.
			       Please **don't give** the pointer back to the filter at the next call.
			       Ah, and out_buf_len still defines the length of the valid data (remember to drop this value ASAP). */
} filter_ret_t;
typedef filter_ret_t(*filter_t)(const void* in_buf, const size_t in_buf_len,
				void** out_buf, size_t* out_buf_len,
				void** ancillary_data, size_t* ancillary_len, dir_t direction);

/* helper functions */
int sock_createandbind(const int port);
void loadTCPinfo(struct sockaddr** info, socklen_t* info_len, char* IP, int port);
int do_child(int udp_sock_fd,void*buf,size_t buflen,const struct sockaddr* sender, const socklen_t sender_l,
	     const struct sockaddr* recipient, const socklen_t recipient_l, filter_t filter);
const char* filter_ret2str(const filter_ret_t) __attribute__((pure)) ;

/* filters */
filter_ret_t filter_dns(const void* in_buf, const size_t in_buf_len,void** out_buf, size_t* out_buf_len,
			void** ancillary_data, size_t* ancillary_len, dir_t direction);

/* look-and-feel and information messages */
void print_usage();

#endif /* #ifndef UDP2TCP_H_INCLUDED */
