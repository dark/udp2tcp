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
#include <stdlib.h>
#include <string.h>
#include "filters.h"
#include "logging.h"

/* global variables */
extern char *program_name;
extern log_t logger;

/* == static variables == */
static struct {
  char* f_name;
  filter_t f_impl;
  char* description;
} filters[] = {
  {"dns",filter_dns,"Translate DNS packets between the TCP and the UDP format"},
  {NULL, NULL, NULL} /* NULL-termination */
};
  

/* == helper functions == */
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

filter_t filter_name2impl(const char* name){
  int i=0;
  while(filters[i].f_name!=NULL){
    if (strcmp(name,filters[i].f_name) == 0)
      return filters[i].f_impl;
    
    i++;
  }
  return NULL;
}

void filter_enumerate(){
  int i=0;
  while(filters[i].f_name!=NULL){
    fprintf(stderr, "%s - %s\n", filters[i].f_name, filters[i].description);
    i++;
  }
}


/* == implemented filters == */
filter_ret_t filter_dns(const void* in_buf, const size_t in_buf_len,
			void** out_buf, size_t* out_buf_len,
			void** ancillary_data, size_t* ancillary_len, dir_t direction){
  uint16_t length;

  switch (direction){
  case UDP2TCP:
    /* adding length field */
    length = htons((uint16_t)in_buf_len);
    if ( (*out_buf = realloc(*out_buf, in_buf_len+2)) == NULL){
      F_ERROR("realloc() failed growing out_buf size to %d\n",in_buf_len+2);
      return ERROR;
    }
    memmove((*out_buf)+2, in_buf, in_buf_len);
    *out_buf_len=in_buf_len+2;
    memcpy(*out_buf, &length, sizeof(length));
    return OUT_BUF_DYN;

  case TCP2UDP:
    /* removing length field */
    if (in_buf_len <= 2){
      F_ERROR("message too short (%d bytes long)\n",in_buf_len);
      return ERROR;
    }
    /* since I'll return a static reference, free the out_buf, if already allocated */
    free(*out_buf);
    *out_buf = (void*)in_buf+2;
    *out_buf_len = in_buf_len-2;
    return OUT_BUF_STAT;
  }

  return 0;
}
