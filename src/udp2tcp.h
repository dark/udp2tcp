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

#ifndef UDP2TCP_H_INCLUDED
#define UDP2TCP_H_INCLUDED

#include <errno.h>
#include "filters.h"

#define PROJECT_NAME "udp2tcp"
#define PROJECT_VER "0.0.2-alpha"
#define PROJECT_SIG PROJECT_NAME " v." PROJECT_VER ", by dark"

/* defines a dafe value for the network data handling buffers */
#define LOCAL_BUFFER_SIZE 1500
#define INTERRUPTED_BY_SIGNAL (errno == EINTR || errno == ECHILD)

/* default values */
#define DFLT_UDP_PORT 53
#define DFLT_TCP_HOST "127.0.0.1"
#define DFLT_TCP_PORT 53

/* helper functions */
int handle_commandline(int argc, char* argv[],
		       int* port_in, char** host_out, int* port_out, filter_t* filter, int* verbosity);
int sock_createandbind(const int port);
void loadTCPinfo(struct sockaddr** info, socklen_t* info_len, char* IP, int port);
int do_child(int udp_sock_fd,void*buf,size_t buflen,const struct sockaddr* sender, const socklen_t sender_l,
	     const struct sockaddr* recipient, const socklen_t recipient_l, filter_t filter);

/* look-and-feel and information messages */
void print_usage();

#endif /* #ifndef UDP2TCP_H_INCLUDED */
