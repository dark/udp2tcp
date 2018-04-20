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

#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED

typedef struct _log_t* log_t;
typedef enum{CONSOLE=0,LOGFILE,SYSLOG} log_provider_t;
typedef enum{L_DEBUG=0,L_INFO,L_WARN,L_ERROR} log_level_t;

#ifdef DEBUG
#define F_DEBUG(fmt,...) do{logging_do(logger,L_DEBUG,__FILE__,__LINE__,fmt, ## __VA_ARGS__);}while(0)
#define F_INFO(fmt,...)  do{logging_do(logger,L_INFO ,__FILE__,__LINE__,fmt, ## __VA_ARGS__);}while(0)
#define F_WARN(fmt,...)  do{logging_do(logger,L_WARN ,__FILE__,__LINE__,fmt, ## __VA_ARGS__);}while(0)
#define F_ERROR(fmt,...) do{logging_do(logger,L_ERROR,__FILE__,__LINE__,fmt, ## __VA_ARGS__);}while(0)
#else
#define F_DEBUG(fmt,...) do{logging_do(logger,L_DEBUG,fmt, ## __VA_ARGS__);}while(0)
#define F_INFO(fmt,...)  do{logging_do(logger,L_INFO ,fmt, ## __VA_ARGS__);}while(0)
#define F_WARN(fmt,...)  do{logging_do(logger,L_WARN ,fmt, ## __VA_ARGS__);}while(0)
#define F_ERROR(fmt,...) do{logging_do(logger,L_ERROR,fmt, ## __VA_ARGS__);}while(0)
#endif /* #ifdef DEBUG */

/* logger-related public prototypes */
log_t logging_start(log_provider_t, log_level_t cutoff_level);
void logging_stop(const log_t l);
#ifdef DEBUG
void logging_do(const log_t l,const log_level_t level,const char* filename,
		const int line, const char *format, ... );
#else
void logging_do(const log_t l,const log_level_t level,const char *format, ... );
#endif /* #ifdef DEBUG */

/* helper functions */
log_provider_t logging_str2provider(char* p) __attribute__((pure,warn_unused_result));
log_level_t logging_verbosity2level(int verbosity) __attribute__((pure));
void logging_enumerate();

#endif /* #ifndef LOGGING_H_INCLUDED */
