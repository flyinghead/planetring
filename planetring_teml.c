/*
 *
 * Copyright 2017 Shuouma <dreamcast-talk.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * Planet Ring TEML for Dreamcast
 */

#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <string.h>
#include "planetring_common.h"
#include "planetring_sql.h"

/*
 * Function: read_teml_file
 * --------------------
 *
 * Function that reads the teml file
 * from disc.
 * 
 *  *f_name: filename to read
 *  *msg:    outgoing client msg
 *
 *  returns: file size
 *
 */
uint32_t read_teml_file(char *f_name, char *msg) {  
  FILE *file;
  uint32_t fileLen;

  file = fopen(f_name, "rb");
  if (!file) {
    planetring_error("Unable to open file %s", f_name);
    return 0;
  }
  
  fseek(file, 0, SEEK_END);
  fileLen = (uint32_t)ftell(file);
  fseek(file, 0, SEEK_SET);

  if (fileLen > MAX_TEML_PKT_SIZE) {
    planetring_error("File size greater then buffer");
    return 0;
  }

  fread(msg, fileLen, 1, file);
  fclose(file);

  return (fileLen-1);
}

/*
 * Function: add_teml_header
 * --------------------
 *
 * Function that adds teml header
 * to pkt.
 * 
 *  *msg:     outgoing client msg
 *  pkt_size: pkt size 
 *
 *  returns: pkt size
 *
 */
uint32_t add_teml_header(char *msg, uint32_t pkt_size) {
  int ret = sprintf(msg, "TEML");
  uint32_to_char(htonl(pkt_size), &msg[ret]);
  pkt_size += 8;
  return pkt_size;
}

/*
 * Function: teml_msg_handler
 * --------------------
 *
 * Function that parses the
 * players input and displays
 * the correct TEML page
 * 
 *  *pl:      ptr to player stryct
 *  *msg:     outgoing client msg
 *  *buf:     incoming client msg
 *  buf_len   size of read incoming msg
 *
 *  returns: pkt size
 *
 */
uint32_t teml_msg_handler(player_t *pl, char *msg, char *buf, int buf_len) {
  uint32_t pkt_size=0;
  server_data_t *s = (server_data_t*)pl->data;
  char teml_fn[512];
  memset(teml_fn,0,sizeof(teml_fn));

  buf[buf_len] = '\0';
  if (buf_len > 0 && buf[buf_len - 1] == '\n')
    buf[--buf_len] = '\0';

  if (buf_len > (sizeof(teml_fn) - strlen(s->pr_teml_path)))
    return 0;
  
  if (strncmp(buf, "FRQ#A_HOME", 10) == 0) {
    sprintf(teml_fn, "%s/%s", s->pr_teml_path, "A_HOME.TEML");
    pkt_size = read_teml_file(teml_fn, &msg[0x08]);
    
  } else if (strncmp(buf,"FRQ#R_HOME", 10) == 0) {
    sprintf(teml_fn, "%s/%s", s->pr_teml_path, "R_HOME.TEML");
    pkt_size = read_teml_file(teml_fn, &msg[0x08]);
    
  } else if (strncmp(buf,"FRQ#N_HOME", 10) == 0) {
    sprintf(teml_fn, "%s/%s", s->pr_teml_path, "N_HOME.TEML");
    pkt_size = read_teml_file(teml_fn, &msg[0x08]);

  } else if (strstr(buf, ".PNG") != NULL) {
    sprintf(teml_fn, "%s/IMG/%s", s->pr_teml_path, &buf[4]);
    pkt_size = read_teml_file(teml_fn, &msg[0x08]);
    pkt_size++;
    
  } else if (strncmp(buf, "FRQ#", 4) == 0) {
    sprintf(teml_fn, "%s%s", s->pr_teml_path, &buf[4]);
    pkt_size = read_teml_file(teml_fn, &msg[0x08]);
    pkt_size++;
  }

  if (pkt_size > 0) {
    return add_teml_header(msg, pkt_size);
  } else {
    memset(teml_fn,0,sizeof(teml_fn));
    sprintf(teml_fn, "%s%s", s->pr_teml_path, "NOT_FOUND.TEML");
    pkt_size = read_teml_file(teml_fn, &msg[0x08]);
    return add_teml_header(msg, pkt_size);
  }
  
  return 0;
}

/*
 * Function: planetring_teml_client_handler
 * --------------------
 *
 * Function that handles the TEML TCP clients
 * 
 *  *data: ptr to player struct
 *
 *  returns: void
 *
 */
void *planetring_teml_client_handler(void *data) {
  player_t *pl = (player_t *)data;
  int sock = pl->sock; 
  ssize_t read_size=0;
  size_t write_size=0;
  char c_msg[MAX_TEML_PKT_SIZE], s_msg[MAX_TEML_PKT_SIZE];
  memset(c_msg, 0, sizeof(c_msg));
  memset(s_msg, 0, sizeof(s_msg));

  struct timeval tv;
  tv.tv_sec = 1800;       /* Timeout in seconds */
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(char *)&tv,sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval));
  
  //Receive a message from client
  while( (read_size = recv(sock , c_msg , sizeof(c_msg) , 0)) > 0 ) {
    write_size = (ssize_t)teml_msg_handler(pl, s_msg, c_msg, (int)read_size);
    if (write_size > 0) {
      write(sock, s_msg, write_size);
    }
    if (write_size < 0) {
      planetring_error("Client with socket %d is not following protocol - Disconnecting", sock);
      close(sock);
      free(pl);
      return 0;
    }
    memset(s_msg, 0, sizeof(s_msg));
    memset(c_msg, 0, sizeof(c_msg));
    fflush(stdout);
  }
  
  planetring_info("[TEML] - Closing socket %d [%s]", sock, inet_ntoa(pl->addr.sin_addr));
  close(sock);
  fflush(stdout);
  
  free(pl);
  return 0;
}
