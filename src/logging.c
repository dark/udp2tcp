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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include "logging.h"
#include "udp2tcp.h"

/* global variables */
extern char *program_name;

/* == types == */
// callback: function type to log data
#ifdef DEBUG
typedef void (*logf_do_t)(const log_t,const log_level_t level,const char* filename,
			  const int line,const char *format, va_list ap);
#else
typedef void (*logf_do_t)(const log_t,const log_level_t level,const char *format, va_list ap);
#endif

// callback: function type to stop logging
typedef void (*logf_close_t)(const log_t);

struct _log_t{
  log_provider_t p_type;
  log_level_t cutoff_level;

  logf_do_t logf_do;
  logf_close_t logf_close;
};

/* == static variables == */
static struct {
  char* l_name;
  log_level_t level;
  char* description;
} levels[] = {
  {"console",CONSOLE,"Print data on console/terminal"},
  {"log",    LOGFILE,"Dump all the informations on a text file"},
  {"syslog", SYSLOG, "Use the system logger"},
  {NULL, -1, NULL} /* NULL-termination */
};


/* == static functions == */
/* generic */
static log_t logging_alloc(log_provider_t p_type, log_level_t cutoff,
			   logf_do_t f1, logf_close_t f2){
  log_t l = malloc( sizeof(struct _log_t) );
  if (!l)
    return NULL;

  l->p_type=p_type;
  l->cutoff_level=cutoff;
  l->logf_do=f1;
  l->logf_close=f2;

  return l;
}

static void logging_free(log_t l){
  free(l);
}

/* console */
static void console_do(const log_t l,const log_level_t level,
#ifdef DEBUG
		       const char* filename, const int line, 
#endif
		       const char *format, va_list ap){

#ifdef DEBUG
  printf("%s[%d]@%s:%d> ", program_name,getpid(),filename,line);
#else
  printf("%s[%d]> ", program_name,getpid());
#endif

  vprintf(format,ap);
}

static void console_close(const log_t l){
  logging_free(l);
}

/* syslog */
static void syslog_do(const log_t l,const log_level_t level,
#ifdef DEBUG
		      const char* filename, const int line, 
#endif
		      const char *format, va_list ap){
  vsyslog(level,format,ap);
}

static void syslog_close(const log_t l){
  logging_free(l);
}

/* == public functions implementation == */
log_t logging_start(log_provider_t p, log_level_t cutoff_level){
  switch(p){
  case CONSOLE:
    return logging_alloc(CONSOLE, cutoff_level, console_do, console_close);

  case LOGFILE:
    return NULL;

  case SYSLOG:
    openlog(program_name,LOG_PID,LOG_USER);
    return logging_alloc(SYSLOG, cutoff_level, syslog_do, syslog_close);

  default:
    return NULL;
  }
}

void logging_stop(const log_t l){
  l->logf_close(l);
}


void logging_do(const log_t l,const log_level_t level,
#ifdef DEBUG
		const char* filename, const int line, 
#endif
		const char *format, ... ){
  va_list ap;

  if(level < l->cutoff_level)
    return;
  
  va_start(ap, format);
#ifdef DEBUG
  l->logf_do(l,level,filename,line,format,ap);
#else
  l->logf_do(l,level,format,ap);
#endif
  va_end(ap);
}

/* ** helper functions ** */
log_provider_t logging_str2provider(char* p) {
  int i=0;
  while(levels[i].l_name!=NULL){
    if (strcmp(p,levels[i].l_name) == 0)
      return levels[i].level;
    
    i++;
  }
  return -1;
}

log_level_t logging_verbosity2level(int verbosity){
  if (verbosity<=0)
    return L_ERROR;
  else if (verbosity == 1)
    return L_WARN;
  else if (verbosity == 2)
    return L_INFO;
  else
    return L_DEBUG;
}

void logging_enumerate(){
  int i=0;
  while(levels[i].l_name!=NULL){
    fprintf(stderr, "%s - %s\n", levels[i].l_name, levels[i].description);
    i++;
  }
}
