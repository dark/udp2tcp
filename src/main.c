/*
 udp2tcp
 Copyright (C) 2008 Marco Leogrande
 
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

#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "filters.h"
#include "logging.h"
#include "udp2tcp.h"

/* global variables */
char *program_name;
log_t logger;

/* SIGCHLD handler */
void handler_sigchld(int signum){
  pid_t pid;
  int status;

  if (signum!=SIGCHLD) /* avoid trivial errors */
    return;

  while ( (pid = waitpid(-1, &status,WNOHANG)) >0 )
    F_DEBUG("Child %d exited with status %d\n", pid, status);

  /* because of Solaris way of doing things... */
  if ( signal(SIGCHLD, handler_sigchld) == SIG_ERR)
    F_ERROR("Unable to set the handler for SIGCHLD, this WILL lead to the proliferation of zombie children");
}

int main (int argc, char *argv[]){
  int ret;
  int sock_fd;

  /* info from commandline */
  int port_in= -1,
    port_out = -1;
  char* host_out = NULL;
  filter_t filter = NULL;
  int verbosity = 0;

  struct sockaddr* remote_info=NULL;
  socklen_t remote_info_len=0;

  char buffer[LOCAL_BUFFER_SIZE];
  ssize_t n_bytes_read;
  struct sockaddr from;
  socklen_t fromlen=sizeof(from);

  pid_t pid;

  /* globals initialization */
  program_name = basename(argv[0]);

  /* ** handle command line ** */
  fprintf(stderr,"Starting %s\n", PROJECT_SIG);
  ret = handle_commandline(argc, argv, &port_in, &host_out, &port_out, &filter, &verbosity, &logger);
  if (ret!=0){
    /* an error has occurred */
    print_usage();
    return ret;
  }
  F_INFO("Will forward from UDP %d to TCP %s:%d\n",port_in,host_out,port_out);

  /* ** setup the listening socket ** */
  sock_fd=sock_createandbind(port_in);

  /* ** drop privileges ** */
  setuid(getuid());
  F_DEBUG("Dropped privileges to uid %d, euid %d\n",getuid(),geteuid());

  /* ** fill the struct that children need ** */
  loadTCPinfo(&remote_info, &remote_info_len, host_out, port_out);

  /* ** set the SIGCHLD handler ** */
  if ( signal(SIGCHLD, handler_sigchld) == SIG_ERR)
    F_ERROR("Unable to set the handler for SIGCHLD, this WILL lead to the proliferation of zombie children");

  /* ** while there are incoming messages, fork
     and let the child handle the incoming datagram ** */
  while(1){
    n_bytes_read = recvfrom(sock_fd,buffer,LOCAL_BUFFER_SIZE,0,&from,&fromlen);

    if (n_bytes_read<0){
      F_WARN("Recvfrom returned %d\n",n_bytes_read);
      continue;
    }

    pid = fork();
    if (pid==0){
      /* child */
      F_DEBUG("Child successfully spawned (PID %d)\n",getpid());
      ret = do_child(sock_fd, buffer, n_bytes_read, &from, fromlen, remote_info, remote_info_len, filter);
      exit(ret);
    } else if (pid<0){
      /* parent, fork error */
      F_WARN("fork() returned %d\n", pid);
    } else {
      /* parent, fork successful */
      ;
    }
  } /* while(1) */

  /* should I free all the freeable? */

  return 0;
}
