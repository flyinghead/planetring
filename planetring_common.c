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
 * Planet Ring Common functions for Dreamcast
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include "planetring_common.h"
#include "planetring_msg.h"

/*
 * Function: keepalive_check
 * --------------------
 *
 * Checks how long it has been since a new
 * keepalive has been sent by a player
 * If more than 120 sec, user has probably
 * timed out.
 * 
 *  *s: ptr to server data struct
 *
 *  returns: void
 *
 */
void keepalive_check(server_data_t *s) {
  int i=0;
  uint32_t now=0;
  time_t seconds=0;
  seconds = time(NULL);
  now = (uint32_t)(seconds);

  for(i=0;i<(s->m_cli); i++) {
    if(s->p_l[i] != NULL) {
      if ((s->p_l[i]->online == 1)) {
	//Time between keepalive should not be more then 2min (120 sec)
	if ((now - (s->p_l[i]->keepalive)) > 120) {
	  planetring_info("User %s is not sending keepalive, last was %d ago", s->p_l[i]->username, (now - (s->p_l[i]->keepalive)));
	  leave_table_in_attraction(s->p_l[i], (uint8_t)s->p_l[i]->tbl_id);
	  s->p_l[i]->online = 0;
	}
      }
    }
  }
  return;
}

/*
 * Function: sanity_check
 * --------------------
 *
 * Check if player is in any attraction,
 * if then remove. Function is used for
 * sanity checks, for example after a 
 * time out then a quick reconnect.
 * 
 *  *pl: ptr to player data struct
 *
 *  returns: void
 *
 */
void sanity_check(player_t *pl) {
  server_data_t *s = (server_data_t*)pl->data;
  int i=0;
  //Remove user from last attraction if timed out
  for(i=0;i<s->m_tables;i++) {
    leave_table_in_attraction(pl, (uint8_t)i);
  }
  return;
}

/*
 * Function: concat_dig
 * --------------------
 *
 * Concat at top 3 digits
 * For example, 11 1 => 111
 * 
 *  x: digit(s)
 *  y: digit(s)
 *
 *  returns: concat digits (max 255)
 *
 */
uint8_t concat_dig(uint8_t x, uint8_t y) {
  unsigned pow = 10;
  while(y >= pow)
    pow *= 10;
  return (uint8_t)(x * pow + y);
}

/*
 * Function: get_zodiac
 * --------------------
 *
 * Returns zodiac nr
 * 
 *  birthday: 32 bit birthday (19000101)
 *
 *  returns: int
 *
 */
uint8_t get_zodiac(uint32_t birthday) {
  int i=0;
  uint8_t dig[4];
  //Get the last 4 digits from uint32_t
  while(i < 4) {
    dig[i] = (uint8_t)(birthday%10);
    birthday = birthday/10;
    i++;
  }
  uint8_t month = concat_dig(dig[3], dig[2]);
  uint8_t day = concat_dig(dig[1], dig[0]);
  //Return zodiac nr, switch on month to speed it up
  switch(month) {
  case 1:
    if (day <= 20) return 9;
    if (day >= 21) return 10;
  case 2:
    if (day <= 19) return 10;
    if (day >= 20) return 11;
  case 3:
    if (day <= 20) return 11;
    if (day >= 21) return 0;
  case 4:
    if (day <= 20) return 0;
    if (day >= 21) return 1;
  case 5:
     if (day <= 21) return 1;
     if (day >= 22) return 2;
  case 6:
    if (day <= 21) return 2;
    if (day >= 22) return 3;
  case 7:
    if (day <= 22) return 3;
    if (day >= 23) return 4;
  case 8:
     if (day <= 21) return 4;
     if (day >= 22) return 5;
  case 9:
    if (day <= 23) return 5;
    if (day >= 24) return 6;
  case 10:
    if (day <= 23) return 6;
    if (day >= 24) return 7;
  case 11:
    if (day <= 22) return 7;
    if (day >= 23) return 8;
  case 12:
    if (day <= 22) return 8;
    if (day >= 23) return 9;
  }

  return 0;
}  

/*
 * Function: get_age
 * --------------------
 *
 * Returns age of player
 * 
 *  birthday: birthday (19000101)
 *
 *  returns: int
 *
 */
int get_age(uint32_t birthday) {
  int age = 0;
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  age = (int)(timeinfo->tm_year + 1900) - (int)(birthday/10000);
  
  return age;
}   

/*
 * Function: get_attraction_from_location
 * --------------------
 *
 * Returns attraction struct from location
 * 
 *  *s:        ptr to server data struct
 *  location:  id of attraction 10->13
 *
 *  returns: ptr to attraction struct
 *
 */
attraction_t** get_attraction_from_location(server_data_t *s, uint8_t location) {
  switch ((int)location) {
  case 10:
    return (attraction_t **)(s->splash);
  case 11:
    return (attraction_t **)(s->ball_bubble);
  case 12:
    return (attraction_t **)(s->soar);
  case 13:
    return (attraction_t **)(s->dorobo);
  default:
    return NULL;
  }
}

/*
 * Function: get_attraction_from_socket
 * --------------------
 *
 * Returns attraction struct from socket
 * 
 *  *s:        ptr to server data struct
 *  socket:    socket
 *
 *  returns: ptr to attraction struct
 *
 */
attraction_t** get_attraction_from_socket(server_data_t *s, int socket) {
  if (socket == s->splash_sock)
    return (attraction_t **)(s->splash);
  if (socket == s->bb_sock)
    return (attraction_t **)(s->ball_bubble);
  if (socket == s->soar_sock)
    return (attraction_t **)(s->soar);
  if (socket == s->doro_sock)
    return (attraction_t **)(s->dorobo);

  return NULL;
}

/*
 * Function: get_user_from_nick
 * --------------------
 *
 * Returns player struct from username
 * 
 *  *s:        ptr to server data struct
 *  *u_name:   username
 *
 *  returns: ptr to player struct
 *
 */
player_t* get_user_from_nick(server_data_t *s, char* u_name) {
  int i=0;
  int max_clients = s->m_cli;

  for(i=0;i<max_clients;i++) {
    if (s->p_l[i] != NULL) {
      if ((strncmp(s->p_l[i]->username, u_name, 16) == 0)) {
	return s->p_l[i];
      }
    }
  }
  return NULL;
}

/*
 * Function: get_user_from_addr
 * --------------------
 *
 * Returns player struct from addr
 * 
 *  *s:        ptr to server data struct
 *  *addr:     sockaddr_in
 *
 *  returns: ptr to player struct
 *
 */
player_t* get_user_from_addr(server_data_t *s, struct sockaddr_in *addr) {
  int i=0;
  int max_clients = s->m_cli;

  for(i=0;i<max_clients;i++) {
    if (s->p_l[i] != NULL) {
      if((s->p_l[i]->addr.sin_addr.s_addr == addr->sin_addr.s_addr)) {
	return s->p_l[i];
      }
    }
  }
  return NULL;
}

/*
 * Function: get_udp_sock_from_location
 * --------------------
 *
 * Returns UDP socket from location
 * 
 *  *s:        ptr to server data struct
 *  location:  id of location
 *
 *  returns: socket
 *
 */
int get_udp_sock_from_location(server_data_t *s, uint8_t location) {
  switch (location) {
  case INFO:
    return s->info_sock;
  case PLANET:
    return s->planet_sock;
  case SPLASH:
    return s->splash_sock;
  case BALLBUBBLE:
    return s->bb_sock;
  case SOAR:
    return s->soar_sock;
  case DOROBO:
    return s->doro_sock;
  default:
    return 0;
  }
}

/*
 * Function: planetring_profile_dates
 * --------------------
 *
 * Returns dates in special PR way
 * 
 *
 *  returns: profile date
 *
 */
unsigned int planetring_profile_dates(void) {
  time_t t = (int)time(NULL);
  struct tm *timePtr = localtime(&t);
  int year =  (timePtr->tm_year + 1900);
  int month = (timePtr->tm_mon + 1);
  int day =  timePtr->tm_mday;
  
  unsigned int dt = (unsigned int)
    ( (((year) & 0x7FF) << 20) | (((month) & 0xF) << 16) | (((day) & 0x1F) << 11) );

  return dt;
}

/*
 * LOGGING FUNCTIONS
 */
void planetring_error(const char* format, ... ) {
  va_list args;
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char td_str[64];
  const char* s_str;
  
  memset(td_str, 0, sizeof(td_str));
  snprintf(td_str, sizeof(td_str), "[%04d/%02d/%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  fprintf(stderr,"%s",td_str);
  s_str = "[PLANETRING - Server] [ERROR] - ";
  
  fprintf(stdout,"%s",s_str);
  va_start(args,format);
  vfprintf(stdout,format,args);
  va_end(args);
  fprintf(stdout,"\n");
  fflush(stdout);
}

void planetring_info(const char* format, ... ) {
  va_list args;
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char td_str[64];
  const char* s_str;

  memset(td_str, 0, sizeof(td_str));
  snprintf(td_str, sizeof(td_str), "[%04d/%02d/%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  fprintf(stdout,"%s",td_str);

  s_str = "[PLANETRING - Server] [INFO] - "; 

  fprintf(stdout,"%s",s_str);
  va_start(args,format);
  vfprintf(stdout,format,args);
  va_end(args);
  fprintf(stdout,"\n");
  fflush(stdout);
}

/*
 * Function: get_planetring_config
 * --------------------
 *
 * Function that parses the config
 * 
 *  *s:        ptr to server data struct
 *  *fn:       filename of config
 *
 *  returns:
 *           1 => OK
 *           0 => FAILED
 */
int get_planetring_config(server_data_t *s, char *fn) {
  planetring_info("Reading config...");
  FILE *file = fopen(fn,"r");
  int pr_max_cli=0, i=0;
  int pr_port=0;
  char pr_ip[16], buf[1024], pr_db_path[256], pr_teml_path[256];
  memset(buf, 0, sizeof(buf));
  memset(pr_ip, 0, sizeof(pr_ip));
  memset(pr_db_path, 0, sizeof(pr_db_path));
  memset(pr_teml_path, 0, sizeof(pr_teml_path));
  
  if (file != NULL) {
    while (fgets(buf, sizeof(buf), file) != NULL) {
      sscanf(buf, "PLANETRING_PORT=%d", &pr_port);
      sscanf(buf, "PLANETRING_MAX_CLIENTS=%d", &pr_max_cli);
      sscanf(buf, "PLANETRING_IP=%15s", pr_ip);
      sscanf(buf, "PLANETRING_DB_PATH=%s", pr_db_path);
      sscanf(buf, "PLANETRING_TEML_PATH=%s", pr_teml_path);
    }
    fclose(file);
  } else {
    planetring_info("Config file planetring.cfg is missing in %s", fn);
    return 0;
  }

  if (pr_ip[0] == '\0') {
    planetring_info("Missing PLANETRING_IP");
    return 0;
  }
  if (pr_db_path[0] == '\0') {
    planetring_info("Missing PLANETRING_DB_PATH");
    return 0;
  }
  if (pr_teml_path[0] == '\0') {
    planetring_info("Missing PLANETRING_TEML_PATH");
    return 0;
  }
  if (pr_port == 0) {
    planetring_info("Missing PLANETRING_PORT");
    return 0;
  }
  if (pr_max_cli == 0) {
    planetring_info("Missing PLANETRING_MAX_CLIENTS - Set to default value");
    s->m_cli = 100;
  } else {
    s->m_cli = pr_max_cli;
  }
   
  strncpy(s->pr_ip, pr_ip, sizeof(pr_ip));
  strncpy(s->pr_db_path, pr_db_path, sizeof(pr_db_path));
  strncpy(s->pr_teml_path, pr_teml_path, sizeof(pr_teml_path));
  s->pr_port = (uint16_t)pr_port;
    
  planetring_info("Loaded Config:");
  planetring_info("\tPLANETRING_IP: %s", s->pr_ip);
  planetring_info("\tPLANETRING_PORT: %d", s->pr_port);
  planetring_info("\tPLANETRING_DB_PATH: %s", s->pr_db_path);
  planetring_info("\tPLANETRING_TEML_PATH: %s", s->pr_teml_path);
  planetring_info("\tPLANETRING_MAX_CLIENTS: %d", s->m_cli);
  //Allocate pointer arrays
  s->p_l = calloc((size_t)s->m_cli, sizeof(player_t *));
  for(i=0;i<(s->m_cli);i++)
    s->p_l[i] = NULL;

  s->m_tables = 5;

  s->c_cli = 0;
  
  s->splash = calloc((size_t)s->m_tables, sizeof(attraction_t *));
  for(i=0;i<(s->m_tables);i++)
    s->splash[i] = NULL;
  
  s->ball_bubble = calloc((size_t)s->m_tables, sizeof(attraction_t *));
  for(i=0;i<(s->m_tables);i++)
    s->ball_bubble[i] = NULL;

  s->soar = calloc((size_t)s->m_tables, sizeof(attraction_t *));
  for(i=0;i<(s->m_tables);i++)
    s->soar[i] = NULL;

  s->dorobo = calloc((size_t)s->m_tables, sizeof(attraction_t *));
  for(i=0;i<(s->m_tables);i++)
    s->dorobo[i] = NULL;

  return 1;
}

/*
 * HELP FUNCTIONS
 */
uint32_t char_to_uint32(char* data) {
  uint32_t val = (uint32_t)((uint8_t)data[0] << 24 | (uint8_t)data[1] << 16 | (uint8_t)data[2] << 8  | (uint8_t)data[3]);
  return val;
}

uint16_t char_to_uint16(char* data) {
  uint16_t val = (uint16_t)((uint8_t)data[0] << 8 | (uint8_t)data[1]);
  return val;
}

int uint32_to_char(uint32_t data, char* msg) {
  msg[0] = (char)(data >> 24);
  msg[1] = (char)(data >> 16);
  msg[2] = (char)(data >> 8);
  msg[3] = (char)data;
  return 4;
}

int uint16_to_char(uint16_t data, char* msg) {
  msg[0] = (char)(data >> 8);
  msg[1] = (char)data;
  return 2;
}

uint32_t strlcpy(char *dst, const char *src, size_t size) {
  char *d = dst;
  const char *s = src;
  size_t n = size;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0) {
    do {
      if ((*d++ = *s++) == 0)
	break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (size != 0)
      *d = '\0';		/* NUL-terminate dst */
    while (*s++);
  }
  
  return (uint32_t)(s - src - 1); 
}

int str2int(char const* str) {
  char *endptr;
  errno = 0;
  long int ret = strtol(str, &endptr, 10);

  if (endptr == str) {
    planetring_error("Not a valid int");
    return 0;
  }
  if (errno == ERANGE) {
    planetring_error("Error parsing in str2int");
    return 0;
  }

  return (int)ret;
}

/*
 *  PRINT TCP DATA
 */
void print_planetring_data(void* pkt,unsigned long pkt_size) {
  unsigned char* pkt_content = (unsigned char*)pkt;
  unsigned long i,j,off;
  char buffer[17];
  buffer[16] = 0;
  off = 0;
  printf("--------------------\n");
  printf("0000 | ");
  for (i = 0; i < pkt_size; i++) {
    if (off == 16) {
      memcpy(buffer,&pkt_content[i - 16],16);
      for (j = 0; j < 16; j++) if (buffer[j] < 0x20) buffer[j] = '.';
      printf("| %s\n%04X | ",buffer,(unsigned int)i);
      off = 0;
    }
    printf("%02X ",pkt_content[i]);
    off++;
  }
  buffer[off] = 0;
  memcpy(buffer,&pkt_content[i - off],off);
  for (j = 0; j < off; j++) if (buffer[j] < 0x20) buffer[j] = '.';
  for (j = 0; j < 16 - off; j++) printf("   ");
  printf("| %s\n",buffer);
}
