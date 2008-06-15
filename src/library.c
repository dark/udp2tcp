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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "logging.h"
#include "udp2tcp.h"

/* global variables */
extern char *program_name;
extern log_t logger;

void print_usage(){
  fprintf(stderr,"\nUsage: %s [-vvv] [-p <UDP_port>] [-h <TCP_dest_IP>] [-P <TCP_port>] [-f <filtername>] [-l <logger>]\n",
	  program_name);
  fprintf(stderr,
	  "  -v   Increase the program verbosity: display warnings\n"
	  "  -vv  Increase again the verbosity: display warnings and information messages\n"
	  "  -vvv Increase the verbosity even more: display warnings, infos and debug messages\n"
	  "  -p   The local UDP port on which listen for incoming datagrams (default: %d)\n"
	  "  -h   The remote TCP port to which forward the data (default: %s)\n"
	  "  -P   The remote TCP host to which forward the data (default: %d)\n"
	  "  -f   The filter to apply to the data in both directions (default: none)\n"
	  "       Use 'list' to enumerate all the filters\n"
	  "  -l   The logger facility to use (default: console)\n"
	  "       Use 'list' to enumerate all the logging facilities\n"
	  ,DFLT_UDP_PORT, DFLT_TCP_HOST, DFLT_TCP_PORT);
}


int handle_commandline(int argc, char* argv[], int* port_in, char** host_out, int* port_out,
		       filter_t* filter, int* verbosity, log_t* logger){
  int ret;
  char *end_ptr=NULL;
  log_provider_t log_provider = -1;
  
  /* initialization */
  *port_in =-1;
  *port_out=-1;
  *host_out=NULL;
  *filter=NULL;
  *verbosity=0;

  opterr=0;
  while ((ret = getopt (argc, argv, "vp:h:P:f:l:")) != -1){
    switch (ret)
      {
      case 'v':
	(*verbosity)++;
	break;

      case 'p':
	if(*port_in == -1){
	  *port_in=strtol(optarg, &end_ptr, 10);
	  if ( (*port_in)<0 || (*port_in)>65535 || (*end_ptr)!='\0'){
	    fprintf(stderr, "Invalid numeric option in argument '%s'\n", optarg);
	    return 1;
	  }
	}
	else{
	  fprintf(stderr, "Local UDP port defined multiple times: it was already %d\n", *port_in);
	  return 1;
	}

	break;

      case 'h':
	if(*host_out == NULL){
	  *host_out = strdup(optarg);
	  if (*host_out == NULL){
	    fprintf(stderr, "Error while allocating memory for host_out\n");
	    return 1;
	  }
	}
	else{
	  fprintf(stderr, "Remote TCP host IP defined multiple times: it was already %s", *host_out);
	  return 1;
	}
	break;

      case 'P':
	if(*port_out == -1){
	  *port_out=strtol(optarg, &end_ptr, 10);
	  if ( (*port_out)<0 || (*port_out)>65535 || (*end_ptr)!='\0'){
	    fprintf(stderr, "Invalid numeric option in argument '%s'\n", optarg);
	    return 1;
	  }
	}
	else{
	  fprintf(stderr, "Local UDP port defined multiple times: it was already %d\n", *port_in);
	  return 1;
	}
	break;

      case 'f':
	if(*filter == NULL){
	  if (strcmp(optarg,"list")==0){
	    // show a list of all the available filters
	    fprintf(stderr,"\n-- This is the list of all the supported filters: --\n");
	    filter_enumerate();
	    exit(0);
	  }

	  *filter = filter_name2impl(optarg);
	  if(*filter == NULL){
	    fprintf(stderr, "Error: filter '%s' unknown (see 'list')\n", optarg);
	    return 1;
	  }
	}
	else{
	  fprintf(stderr, "Multiple filters requested\n");
	  return 1;
	}
	break;

      case 'l':
	if(log_provider == -1){
	  if (strcmp(optarg,"list")==0){
	    // show a list of all the available log providers
	    fprintf(stderr,"\n-- This is the list of all the available log providers: --\n");
	    logging_enumerate();
	    exit(0);
	  }

	  log_provider = logging_str2provider(optarg);
	  if(log_provider == -1){
	    fprintf(stderr, "Error: log provider '%s' unknown (see 'list')\n", optarg);
	    return 1;
	  }
	}
	else{
	  fprintf(stderr, "Multiple log providers requested\n");
	  return 1;
	}
	break;

      case '?':
	if (optopt == 'p' || optopt == 'h' || optopt == 'P' || optopt == 'd' || optopt == 'l')
	  fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	else if (isprint (optopt))
	  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	else
	  fprintf (stderr,
		   "Unknown option character `\\x%x'.\n",
		   optopt);

	return 1;
	
      default:
	fprintf (stderr, "Application bug, notify mantainer.\n"
		 "Received unexpected '%d'\n", ret);
	return 1;
      }
  }

  /* ** handle default values ** */
  if (*port_in == -1)
    *port_in = DFLT_UDP_PORT;
  if (*host_out == NULL)
    *host_out= DFLT_TCP_HOST;
  if (*port_out == -1)
    *port_out= DFLT_TCP_PORT;
  if (log_provider == -1)
    log_provider = DFLT_LOG_PROVIDER;

  *logger = logging_start(log_provider, logging_verbosity2level(*verbosity) );

  return 0;
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
