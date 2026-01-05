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
 * Planet Ring MSG functions for Dreamcast
 */

#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "planetring_common.h"
#include "planetring_sql.h"

/*
 * Function: snd_table_ts
 * --------------------
 *
 * Function that sends table state for
 * each user to the newly joined user
 * 
 *  *s:       ptr to server data struct
 *  *pl:      ptr to player data struct
 *  table_id: id of table
 *
 *  returns: void
 *
 */
void snd_table_ts(server_data_t *s, player_t *pl, uint8_t table_id) {
  char ts_msg[MAX_PKT_SIZE];
  int i=0,pkt_size=0,socket_desc=0;
  struct sockaddr_in client = { 0 };

  attraction_t **a = get_attraction_from_location(s, pl->location);

  if (a == NULL || (table_id > s->m_tables))
    return;
  
  socket_desc = get_udp_sock_from_location(s, pl->location);

  client = pl->addr;
  client.sin_port = htons(CLIENT_UDP_PORT);

  //Send all players TS to user
  for(i=0;i<(a[table_id]->max_pl);i++) {
    if (a[table_id]->p_l[i] != NULL) {
      memset(ts_msg, 0, sizeof(ts_msg));
      pkt_size = sprintf(ts_msg, "TS:%d:%d:1:%s:%d:%d:",
			 table_id,
			 i,
			 a[table_id]->p_l[i]->username,
			 ntohl(a[table_id]->p_l[i]->addr.sin_addr.s_addr),
			 i);
      ts_msg[++pkt_size] = '\0';
      sendto(socket_desc, ts_msg, (size_t)pkt_size, 0, (struct sockaddr*)&client, (socklen_t)sizeof(struct sockaddr_in));
    }
  }

  return;
}

/*
 * Function: snd_player_ts
 * --------------------
 *
 * Function that sends the newly joined user's
 * table state to everybody else on the table.
 * 
 *  *s:       ptr to server data struct
 *  *pl:      ptr to player data struct
 *  table_id: id of table
 *  position: where the player sits
 *  status:   what is the players status
 *
 *  returns: void
 *
 */
void snd_player_ts(server_data_t *s, player_t *pl, uint8_t table_id, int position, int status) {
  char ts_msg[MAX_PKT_SIZE];
  int i=0,pkt_size=0, socket_desc=0;
  struct sockaddr_in client = { 0 };

  attraction_t **a = get_attraction_from_location(s, pl->location);

  if (a == NULL || table_id > s->m_tables)
    return;

  socket_desc = get_udp_sock_from_location(s, pl->location);
  
  //TS
  memset(ts_msg, 0, sizeof(ts_msg));
  pkt_size = sprintf(ts_msg, "TS:%d:%d:%d:%s:%d:%d:",
		     table_id,
		     position,
		     status,
		     pl->username,
		     ntohl(pl->addr.sin_addr.s_addr),
		     position);
  ts_msg[++pkt_size] = '\0';

  //Send TS, only to other in that table
  for(i=0;i<(a[table_id]->max_pl);i++) {
    if (a[table_id]->p_l[i] != NULL) {
      if (a[table_id]->p_l[i] != pl) {
	client = a[table_id]->p_l[i]->addr;
	client.sin_port = htons(CLIENT_UDP_PORT);
	sendto(socket_desc, ts_msg, (size_t)pkt_size, 0, (struct sockaddr*)&client, (socklen_t)sizeof(struct sockaddr_in));
      }
    }
  }
  return;
}

/*
 * Function: snd_tr_to_attraction
 * --------------------
 *
 * Function that sends table information to
 * users running around in a attraction
 * 
 *  *s:       ptr to server data struct
 *  *a:       ptr to attraction
 *  location: location of the player
 *
 *  returns: void
 *
 */
void snd_tr_to_attraction(server_data_t* s, attraction_t *a, uint8_t location) {
  char tr_msg[MAX_PKT_SIZE], tr_usr[1024], tr_cac[512], tr_gen[512];
  struct sockaddr_in client = { 0 };
  int i = 0, pkt_tr_size = 0,socket_desc = 0;
  
  if (a == NULL)
    return;

  socket_desc = get_udp_sock_from_location(s, location);
  
  memset(tr_msg, 0, sizeof(tr_msg));
  memset(tr_usr, 0, sizeof(tr_usr));
  memset(tr_cac, 0, sizeof(tr_cac));
  memset(tr_gen, 0, sizeof(tr_gen));

  if (a->pl_in == 0) {
      sprintf(tr_usr, "%d", 0);
  } else {
    for(i=0;i<(a->max_pl);i++) {
      if (a->p_l[i] != NULL) {
	sprintf(tr_cac, "%s%d,", tr_cac, a->p_l[i]->c_a_c);
	sprintf(tr_gen, "%s%d,", tr_gen, a->p_l[i]->gender);
      }
    }
    sprintf(tr_usr, "%s%s0", tr_cac, tr_gen);
  }
  pkt_tr_size = sprintf(tr_msg, "TR:%d:%d:%d:%d:%s:%d:",
			a->id_tbl,
			a->status,
			a->pl_in,
			a->max_pl,
			tr_usr,
			a->min_pl);
  tr_msg[++pkt_tr_size] = '\0';
  
  //Send TR to all in that attraction
  for(i=0;i<(s->m_cli);i++) {
    if(s->p_l[i] != NULL) {
      if ((s->p_l[i]->online == 1) &&
	  (s->p_l[i]->location == location) &&
	  (s->p_l[i]->status != PLAYING_GAME)) {
	client = s->p_l[i]->addr;
	client.sin_port = htons(CLIENT_UDP_PORT);
	sendto(socket_desc, tr_msg, (size_t)pkt_tr_size, 0, (struct sockaddr*)&client, (socklen_t)sizeof(struct sockaddr_in));
      }
    }
  }
  return;
}

/*
 * Function: snd_trs_to_player
 * --------------------
 *
 * Function that sends table information to
 * specific user that requested it
 * 
 *  *pl:       ptr to player data struct
 *
 *  returns: void
 *
 */
void snd_trs_to_player(player_t *pl) {
  char tr_msg[MAX_PKT_SIZE], tr_usr[1024], tr_cac[512], tr_gen[512];
  int i = 0, j = 0, pkt_tr_size = 0, socket_desc = 0;
  struct sockaddr_in client = { 0 };
  uint8_t location = pl->location;
  server_data_t *s = (server_data_t*)pl->data;
  attraction_t **a = get_attraction_from_location(s, location);

  if (a == NULL)
    return;

  socket_desc = get_udp_sock_from_location(s, location);

  for(i=0;i<(s->m_tables); i++) {
    memset(tr_msg, 0, sizeof(tr_msg));
    memset(tr_usr, 0, sizeof(tr_usr));
    memset(tr_cac, 0, sizeof(tr_cac));
    memset(tr_gen, 0, sizeof(tr_gen));

    if (a[i]->pl_in == 0) {
      sprintf(tr_usr, "%s", "0,0,0");
    } else {
      for(j=0;j<(a[i]->max_pl);j++) {
	if (a[i]->p_l[j] != NULL) {
	  sprintf(tr_cac, "%s%d,", tr_cac, a[i]->p_l[j]->c_a_c);
	  sprintf(tr_gen, "%s%d,", tr_gen, a[i]->p_l[j]->gender);
	}
      }
      sprintf(tr_usr, "%s%s0", tr_cac, tr_gen);
    }
    pkt_tr_size = sprintf(tr_msg, "TR:%d:%d:%d:%d:%s:%d:",
			  i,
			  a[i]->status,
			  a[i]->pl_in,
			  a[i]->max_pl,
			  tr_usr,
			  a[i]->min_pl);
    tr_msg[++pkt_tr_size] = '\0';

    //Send TR to user
    client = pl->addr;
    client.sin_port = htons(CLIENT_UDP_PORT);
    sendto(socket_desc, tr_msg, (size_t)pkt_tr_size, 0, (struct sockaddr*)&client, (socklen_t)sizeof(struct sockaddr_in));
  }
  return;
}

/*
 * Function: leave_table_in_attraction
 * --------------------
 *
 * Function that processes when a player
 * leaves a table in a attraction, aswell
 * used in disconnects and time outs.
 * 
 *  *pl:       ptr to player data struct
 *  table_id:  id of table
 *
 *  returns: void
 *
 */
void leave_table_in_attraction(player_t *pl, uint8_t table_id) {
  int i=0,j=0,max_players=0;
  server_data_t *s = (server_data_t *)pl->data;

  if ((pl->location == DOOR ||
       pl->location == INFO ||
       pl->location == PLANET))
    return;
  
  attraction_t **a = get_attraction_from_location(s, pl->location);

  if (a == NULL || table_id > s->m_tables)
    return;
  
  max_players = a[table_id]->max_pl;
  
  for(i=0;i<max_players;i++) {
    if(a[table_id]->p_l[i] != NULL) {
      if((strcmp(pl->username, a[table_id]->p_l[i]->username) == 0)) {
	planetring_info("User %s left table id %d seat %d", pl->username, table_id, i); 
	a[table_id]->pl_in--;
	a[table_id]->p_l[i] = NULL;
	
	//No more users in table? set status to EMPTY
	if (a[table_id]->pl_in == 0)
	  a[table_id]->status = EMPTY;

	//If table is not in game state and not empty set to WAIT
	if (a[table_id]->status != IN_GAME &&
	    a[table_id]->pl_in != 0)
	  a[table_id]->status = WAIT;

	//Free like a bird...
	pl->status = RUNNING_AROUND;
	
	//Rearrange array
	for (j=1;j<max_players;j++) {
	  if (a[table_id]->p_l[j-1] == NULL) {
	    a[table_id]->p_l[j-1] = a[table_id]->p_l[j];
	    a[table_id]->p_l[j] = NULL;
	  }
	}
	
	//Send TR of current players in table through UDP
	snd_tr_to_attraction(s, a[table_id], pl->location);
	//Send TS of player through UDP
	snd_player_ts(s, pl, table_id, i, 0);
	//We are done
	return;
      }
    }
  }
  return;
}

/*
 * Function: join_table_in_attraction
 * --------------------
 *
 * Function that processes when a player
 * joins a table in a attraction
 * 
 *  *pl:       ptr to player data struct
 *  table_id:  id of table
 *
 *  returns:
 *          1 => OK
 *          0 => FAILED
 */
int join_table_in_attraction(player_t *pl, uint8_t table_id) {
  int i=0,max_players=0;
  server_data_t *s = (server_data_t*)pl->data;
  attraction_t **a = get_attraction_from_location(s, pl->location);

  if ((a == NULL) || (table_id > s->m_tables))
    return 0;
  
  //Game already started when user tried to join, race condition
  if ((a[table_id]->status == IN_GAME) || (a[table_id]->status == CLOSE))
    return 0;
  
  max_players = a[table_id]->max_pl;

  for (i=0;i<max_players;i++) {
    if(a[table_id]->p_l[i] == NULL) {
      planetring_info("User %s joined table id %d seat %d", pl->username, table_id, i); 
      a[table_id]->p_l[i] = pl;
      a[table_id]->pl_in++;

      //Set table status
      if (a[table_id]->pl_in == max_players)
	a[table_id]->status = CLOSE;
      else
	a[table_id]->status = WAIT;

      //Set player status
      pl->status = SITTING_IN_TABLE;
      pl->tbl_id = table_id;
      
      //Send TR of current players in table through UDP
      snd_tr_to_attraction(s, a[table_id], pl->location);
      //Send TS of player to peers through UDP
      snd_player_ts(s, pl, table_id, i, 1);
      //Send TS of peers to player through UDP
      snd_table_ts(s, pl, table_id);
      return 1;
    }
  }
  return 0;
}

/*
 * UDP MESSAGES
 */

/*
 * Function: pr_table_query_udp
 * --------------------
 *
 * Function that sends table information
 * to player.
 * 
 *  *s:        ptr to server data struct
 *  nr_parsed: nr strings parsed
 *  tok_array: array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_table_query_udp(server_data_t *s, int nr_parsed, char **tok_array) {
  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
     
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not find user for %s", tok_array[0]);
    return 0;
  }
     
  snd_trs_to_player(pl);
  return 0;
}

/*
 * Function: pr_user_query_alive_udp
 * --------------------
 *
 * Function that sets keepalive to new value
 * sends out the user profile list.
 * 
 *  *s:          ptr to server data struct
 *  *client:     ptr to server UDP client struct
 *  socket_desc: UDP socket
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_user_query_alive_udp(server_data_t *s, struct sockaddr_in *client, int socket_desc, int nr_parsed, char **tok_array) {
  char s_msg[MAX_PKT_SIZE];
  int i=0;
  int pkt_size=0;
  time_t seconds = 0;
  seconds = time(NULL);
  memset(s_msg, 0, sizeof(s_msg));
  
  if (nr_parsed < 1) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
     
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL)
    return 0;

  //Set new keepalive stamp
  pl->keepalive = (uint32_t)seconds;
  
  pkt_size = sprintf(s_msg, "UP:%d:%s:%s:%d:%d:%d:1:1:%d:%d:",
		     pl->location,
		     pl->username,
		     pl->city,
		     pl->gender,
		     pl->c_a_c,
		     ntohl(pl->addr.sin_addr.s_addr),
		     pl->status,
		     pl->microphone);
  s_msg[++pkt_size] = '\0';

  //Send UP to all in the location, will update the user and the rest in that location
  for(i=0;i<(s->m_cli); i++) {
    if(s->p_l[i] != NULL) {
      if ((s->p_l[i]->online == 1)) {
	client = &s->p_l[i]->addr;
	client->sin_port = htons(CLIENT_UDP_PORT);
	sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)client, (socklen_t)sizeof(struct sockaddr_in));
      }
    }
  }

  if (pl->status == RUNNING_AROUND) {
    if ((pl->location >= SPLASH) && (pl->location <= DOROBO)) {
      snd_trs_to_player(pl);
    }
  }
  
  return 0;
}

/*
 * Function: pr_message_udp
 * --------------------
 *
 * Function that sends out priv. msg
 * between players.
 * 
 *  *s:          ptr to server data struct
 *  *client:     ptr to server UDP client struct
 *  socket_desc: UDP socket
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_message_udp(server_data_t *s, struct sockaddr_in *client, int socket_desc, int nr_parsed, char **tok_array) {
  char s_msg[MAX_PKT_SIZE];
  int pkt_size=0;
  memset(s_msg, 0, sizeof(s_msg));

  client->sin_port = htons(CLIENT_UDP_PORT);
  
  if (nr_parsed < 3) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[1]) > 16) || (strlen(tok_array[2]) > 4000)) {
    pkt_size = sprintf(s_msg, "ME:");
    sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)client, (socklen_t)sizeof(struct sockaddr_in));
    return 0;
  }
    
  //Fetch user
  player_t *pl = get_user_from_nick(s, tok_array[1]);
  if (pl == NULL) {
    pkt_size = sprintf(s_msg, "ME:");
    sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)client, (socklen_t)sizeof(struct sockaddr_in));
    planetring_error("Could not get user %s", tok_array[1]);
    return 0;
  }

  //User need to exsist and online - Dont want to store priv msg.
  if ((pl != NULL) && (pl->online == 0)) {
    pkt_size = sprintf(s_msg, "ME:");
    sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)client, (socklen_t)sizeof(struct sockaddr_in));
  } else {
    pkt_size = sprintf(s_msg, "MR:%s:%s:%s:", tok_array[0], tok_array[1], tok_array[2]);
    client = &pl->addr;
    sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)client, (socklen_t)sizeof(struct sockaddr_in));
  }
  
  return 0;
}

/*
 * Function: pr_ranking_pool_udp
 * --------------------
 *
 * Function that sends the ranking pool
 * after a game
 * 
 *  *s:          ptr to server data struct
 *  *client:     ptr to server UDP client struct
 *  socket_desc: UDP socket
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_ranking_pool_udp(server_data_t *s, struct sockaddr_in *client, int socket_desc, int nr_parsed, char **tok_array) {
  char s_msg[MAX_PKT_SIZE];
  int rank=0;
  rank_t *rp = NULL;
  int pkt_size=0;
  memset(s_msg, 0, sizeof(s_msg));
  
  if (nr_parsed < 3) {
    planetring_error("Invalid number of inputs");
    return 0;
  }
  
  player_t *pl = get_user_from_addr(s, client);
  if (pl == NULL)
    return 0;

  uint8_t pos = (uint8_t)str2int(tok_array[0]);
  uint8_t page = (uint8_t)str2int(tok_array[1]);
  uint8_t rankmode = (uint8_t)str2int(tok_array[2]);
  uint8_t table_id = pl->tbl_id;
  
  attraction_t **a = get_attraction_from_location(s, pl->location);

  if ((a == NULL) ||
      (table_id > s->m_tables) ||
      (pos > a[table_id]->max_pl))
    return 0;
  
  rp = a[table_id]->rank_pool[pos];
  if (rp == NULL) {
    planetring_error("Ranking item is NULL");
    return 0;
  }
  
  if (rp->in_use == 1) {
    if (page == 0) {
      rank = get_rank_for_player(s->db,pl,rp->username,rankmode);
      //SOAR and BALLON - WORKS
      //Over all,Result(not sent),Score,#,Total,Ratio,Rank
      if (pl->location == SOAR && rankmode == 0) {
	pkt_size = sprintf(s_msg, "RP:1:%s:0:%d:%d:1:%d:%d:%d:",
			   rp->username,
			   rp->p1col2,
			   rp->p1col3,
			   rp->p1col4,
			   rp->p1col5,
			   rank);
	//BALLBUBBLE - WORKS
	//Over all,Result(not sent),Score,Average (Tot balls/matches),Total,High
	//P1 Result, Score, Average, Total, High
	//P2 #, Win, Ratio, Rank
	//                  #  W        Rat Ra  A To Hig
	//"RP:1:Tester:0:0:77:120:1:1:1:300:11:12:13:25:
      } else if (pl->location == BALLBUBBLE) {
	pkt_size = sprintf(s_msg, "RP:1:%s:0:0:%d:%d:1:1:1:%d:%d:%d:%d:%d:",
			   rp->username,
			   rp->p2col1,
			   rp->p2col2,
			   rp->p2col3,
			   rank,
			   rp->p1col3,
			   rp->p1col4,
			   rp->p1col5);
	//DOROBO - WORKS
	//P1: Over all, #, Score, High Score, Opponent, Rank
	//P2: #, TOTAL, NAVIGATOR, ATTACKER, BUDDIES, RATIO
	//P1-P2          # T  B   R   N    A  # HIGH RANK   
	//RP:1:Tester:0:20:1:120:200:300:100:20:1000:1:
      } else if (pl->location == DOROBO) {
	pkt_size = sprintf(s_msg, "RP:1:%s:0:%d:%d:%d:%d:%d:%d:%d:%d:%d:",
			   rp->username,
			   rp->p1col1,
			   rp->p1col2,
			   rp->p2col1,
			   rp->p2col2,
			   rp->p2col3,
			   rp->p2col4,
			   rp->p1col1,
			   rp->p1col3,
			   rank);
	//SPLASH - WORKS
	//Over all, #, Win, Lost, Ratio, Rank
	//Record, Sunk, Lost, Hit, Shot, Hit Ratio
      } else if (pl->location == SPLASH) {
	pkt_size = sprintf(s_msg, "RP:1:%s:0:%d:%d:%d:%d:%d:1:1:%d:%d:%d:%d:",
			   rp->username,
			   rp->p1col1,
			   rp->p1col2,
			   rp->p1col3,
			   rp->p1col4,
			   rank,
			   rp->p2col1,
			   rp->p2col2,
			   rp->p2col3,
			   rp->p2col4);
	//REST OF SOAR - WORKS
	//Over all,Result,Time,#,Best time,Rank
      } else {
	pkt_size = sprintf(s_msg, "RP:1:%s:0:%d:%d:1:%d:%d:",
			   rp->username,
			   rp->p1col2,
			   rp->p1col3,
			   rp->p1col4,
			   rank);
      }
      sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)client, (socklen_t)sizeof(struct sockaddr_in));
    }
    //Page 2 - Dummy page
    if (page == 2) {
      pkt_size = sprintf(s_msg, "RP:1:%s:2:1:1:1:1:1:1:1:1:",rp->username);
      sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)client, (socklen_t)sizeof(struct sockaddr_in));
    } 
  } else {
    planetring_error("Got ranking struct which was not in use...");
  }
  
  return 0;
}

/*
 * Function: pr_profile_provider_udp
 * --------------------
 *
 * Function that sends the profile provider
 * 
 *  *s:          ptr to server data struct
 *  *client:     ptr to server UDP client struct
 *  socket_desc: UDP socket
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_profile_provider_udp(server_data_t *s, struct sockaddr_in *client, int socket_desc, int nr_parsed, char **tok_array) {
  char s_msg[MAX_PKT_SIZE];
  int pkt_size=0;
  memset(s_msg, 0, sizeof(s_msg));
  
  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs");
    return 0;
  }
  if ((strlen(tok_array[0]) > 16) ||
      (strlen(tok_array[1]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
     
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL)
    return 0;

  player_t* pl_req = get_user_from_nick(s, tok_array[1]);
  if (pl_req == NULL)
    return 0;

  pkt_size = sprintf(s_msg, "PP:%d:%s:%s:%d:%d:%d:%d:%d:",
		     pl_req->location,
		     pl_req->username,
		     pl_req->city,
		     pl_req->gender,
		     pl_req->language,
		     get_age(pl_req->birthday),
		     pl_req->c_a_c,
		     get_zodiac(pl_req->birthday));

  s_msg[++pkt_size] = '\0';
  sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)client, (socklen_t)sizeof(struct sockaddr_in));
  return 0;
}

/*
 * TCP MESSAGES
 */

/*
 * Function: pr_submit_user
 * --------------------
 *
 * Function that checks the requested username
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_submit_user(server_data_t *s, char* msg, int nr_parsed, char **tok_array) {
  int pkt_size=0;
  int ret=0;
  if (nr_parsed < 1) {
    planetring_error("Invalid number of inputs");
    return 0;
  }
  
  if ((strlen(tok_array[0]) < 3) || (strlen(tok_array[0]) > 16)) {
    planetring_error("Invalid username");
    return 0;
  }
  
  ret = is_player_in_planetring_db(s->db, tok_array[0]);
  
  if (ret == 2) 
    pkt_size = sprintf(msg,"NP:");
  else if (ret == 1) 
    pkt_size = sprintf(msg,"CP:");
  else
    return 0;
   
  msg[++pkt_size] = '\0';

  return (uint16_t)pkt_size;
}

/*
 * Function: pr_submit_user
 * --------------------
 *
 * Function sanity checks registration input from player
 * 
 *  tok_array:   array of parsed strings
 *
 *  returns: 
 *           1 => OK
 *           0 => FAILED
 */
int pr_check_user_form(char **tok_array) {
  //Check user,passwd,city length
  if ((strlen(tok_array[0]) > 16) ||
      (strlen(tok_array[1]) > 16) ||
      (strlen(tok_array[5]) > 255))
    return 0;
  //Check protectcode
  if (strlen(tok_array[2]) != 10)
    return 0;
  //Check gender
  if (strlen(tok_array[4]) != 1)
    return 0;
  //Color and Clothes
  if (strlen(tok_array[6]) < 2)
    return 0;
  //Language
  if (strlen(tok_array[7]) < 1)
    return 0;
  //Birthday
  if (strlen(tok_array[8]) < 8)
    return 0;
  //Int Kind Looks
  if ((strlen(tok_array[9]) > 4) ||
      (strlen(tok_array[10]) > 4) ||
      (strlen(tok_array[11]) > 4))
    return 0;
  //Mic.
  if (strlen(tok_array[12]) != 1)
    return 0;

  return 1;
}

/*
 * Function: pr_new_user
 * --------------------
 *
 * Function that processes player registration
 * 
 *  *s:          ptr to server data struct
 *  *cli         ptr to connect player data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_new_user(server_data_t *s, player_t *cli, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size=0;
  int ret=0,i=0;
  struct sockaddr_in sa;
  char *passwd = NULL;

  if  (s->c_cli == s->m_cli) {
    planetring_error("Server full");
    pkt_size = sprintf(msg,"NE:");
    msg[++pkt_size] = '\0';
    return (uint16_t)pkt_size;
  }
  
  if (nr_parsed != 13) {
    planetring_error("Invalid number of inputs");
    return 0;
  }
  
  if (pr_check_user_form(tok_array) != 1) {
    planetring_error("Invalid registration");
    return 0;
  }
  
  player_t *pl = (player_t *)malloc(sizeof(player_t));
  memset(pl->city, 0, 256);
  memset(pl->username, 0, MAX_UNAME_LEN);
  
  strlcpy(pl->username, tok_array[0], strlen(tok_array[0])+1);
  passwd = tok_array[1];
  strlcpy(pl->city, tok_array[5], strlen(tok_array[5])+1);
  
  pl->gender = (uint8_t)str2int(tok_array[4]);
  pl->c_a_c = (uint32_t)str2int(tok_array[6]);
  pl->language = (uint32_t)str2int(tok_array[7]);
  pl->birthday = (uint32_t)str2int(tok_array[8]);
  pl->intelligence = (uint8_t)str2int(tok_array[9]);
  pl->kindness = (uint8_t)str2int(tok_array[10]);
  pl->looks = (uint8_t)str2int(tok_array[11]);
  pl->microphone = (uint8_t)str2int(tok_array[12]);
  pl->status = RUNNING_AROUND;
  pl->location = DOOR;

  pl->sock = cli->sock;
  pl->addr = cli->addr;
  //Set server data here, not loaded from DB
  pl->data = s;

  pl->c_d = planetring_profile_dates();

  //Add user
  for(i=0;i<(s->m_cli);i++) {
    if(s->p_l[i] == NULL) {
      ret = write_player_to_planetring_db(s->db,
					  pl->username,
					  passwd,
					  pl->city,
					  pl->gender,
					  pl->c_a_c,
					  pl->language,
					  pl->birthday,
					  pl->intelligence,
					  pl->kindness,
					  pl->looks,
					  pl->c_d);
      if (ret != 1) {
	planetring_error("Could not store information");
	free(pl);
	return 0;
      }
      s->p_l[i] = pl;
      s->c_cli++;
      planetring_info("Added client: %s", pl->username);
      break;
    }
  }

  if (ret != 1) {
    planetring_error("Can't add player to player list, bad server!...bad!");
    return 0;
  }

  pl->keepalive = (uint32_t)time(NULL);
  pl->online = 1;
    
  inet_pton(AF_INET, s->pr_ip, &(sa.sin_addr));
  pkt_size = sprintf(msg, "OK:%d:%d:", ntohl(sa.sin_addr.s_addr), s->pr_port);
  msg[++pkt_size] = '\0';
  
  return (uint16_t)pkt_size;
}

/*
 * Function: pr_check_user
 * --------------------
 *
 * Function that validates user and password for
 * an existing user
 * 
 *  *s:          ptr to server data struct
 *  *cli         ptr to connect player data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_check_user(server_data_t *s, player_t *cli, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size=0;
  int ret=0;
  struct sockaddr_in sa = { 0 };

  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs got %d", nr_parsed);
    return 0;
  }
  
  if ((strlen(tok_array[0]) > 16) || (strlen(tok_array[1]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }

  ret = validate_player_login(s->db, tok_array[0], tok_array[1]);

  if (ret != 1) {
    planetring_error("User not allowed to login");
    pkt_size = sprintf(msg, "EP:");
    return (uint16_t)pkt_size;
  }

  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not find user %s", tok_array[0]);
    return 0;
  }

  pl->status = RUNNING_AROUND;
  pl->microphone = (uint8_t)str2int(tok_array[2]);

  pl->keepalive = (uint32_t)time(NULL);
  pl->online = 1;

  pl->sock = cli->sock;
  pl->addr = cli->addr;
 
  //Remove ghost
  sanity_check(pl);
  
  inet_pton(AF_INET, s->pr_ip, &(sa.sin_addr)); 
  pkt_size = sprintf(msg, "OK:%d:%d:", ntohl(sa.sin_addr.s_addr), s->pr_port);
  msg[++pkt_size] = '\0';
  
  return (uint16_t)pkt_size;
}

/*
 * Function: pr_move_to
 * --------------------
 *
 * Function that moves the player to different
 * or same server depending on location
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_move_to(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size=0;
  struct sockaddr_in sa = { 0 };
  
  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs");
    return 0;
  }
  
  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
  
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }
  
  pl->location = (uint8_t)str2int(tok_array[1]);
  
  inet_pton(AF_INET, s->pr_ip, &(sa.sin_addr));
  pkt_size += sprintf(msg, "GO:%d:%d:", ntohl(sa.sin_addr.s_addr), s->pr_port);

  return (uint16_t)pkt_size;
}

/*
 * Function: pr_entry_request
 * --------------------
 *
 * Function that OK movement of player
 * 
 *  *s:          ptr to server data struct
 *  *cli         ptr to connect player data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_entry_request(server_data_t *s, player_t * cli, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size=0;
    
  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs");
    return 0;
  }
  
  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
  
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }
 
  //New socket for each attraction....geesh
  pl->sock = cli->sock;
  pl->addr = cli->addr;
  pl->location = (uint8_t)str2int(tok_array[1]);

  memset(cli->username, 0, MAX_UNAME_LEN);
  sprintf(cli->username, tok_array[0]);

  //Here you need to set maxtables and visitors to prevent coliding tables
  pkt_size = sprintf(msg, "OK:%d:0",s->m_tables);
  msg[++pkt_size] = '\0';
     
  return (uint16_t)pkt_size;
}

/*
 * Function: pr_attraction_query
 * --------------------
 *
 * Function that replies the total amount
 * of tables in that attraction
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_attraction_query(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size=0;
  
  if (nr_parsed < 1) {
    planetring_error("Invalid number of inputs");
    return 0;
  }
  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
    
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }

  //Set the player to running around
  pl->status = RUNNING_AROUND;

  pkt_size = sprintf(msg, "AR:%d:0",s->m_tables);
  msg[++pkt_size] = '\0';

  return (uint16_t)pkt_size;
}

/*
 * Function: pr_table_join
 * --------------------
 *
 * Function that handles if a player
 * wants to join a table
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_table_join(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size = 0;
  uint8_t table_id = 0;
  
  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  table_id = (uint8_t)str2int(tok_array[1]);

  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
    
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }
    
  if (join_table_in_attraction(pl, table_id) == 1) {
    pkt_size = sprintf(msg, "TA:");
    msg[++pkt_size] = '\0';
    return (uint16_t)pkt_size;
  } else {
    //I guess time out
    planetring_error("User could not join table %d", pl->username, table_id);
    return 0;
  }
  
  return 0;
}

/*
 * Function: pr_table_leave
 * --------------------
 *
 * Function that handles if a player
 * wants to leave a table
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_table_leave(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size = 0;
  
  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
    
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }
  
  leave_table_in_attraction(pl, (uint8_t)str2int(tok_array[1]));

  pkt_size = sprintf(msg, "TL:");
  msg[++pkt_size] = '\0';

  return (uint16_t)pkt_size;
}

/*
 * Function: pr_game_query
 * --------------------
 *
 * Function that handles if a player
 * wants to start a game
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_game_query(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  size_t pkt_size=0;
  uint8_t table_id=0;
  attraction_t **a = NULL;
  int i=0, empty_slots=0;

  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }

  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }

  table_id = (uint8_t)str2int(tok_array[1]);
    
  a = get_attraction_from_location(s, pl->location);
  if (a == NULL) {
    planetring_error("Could not get attraction from location %d", pl->location);
    return 0;
  }

  if (a[table_id]->status == IN_GAME) {
    planetring_info("Game already starting");
    return 0;
  }
  if (a[table_id]->pl_in == 0) {
    planetring_info("What happened? Starting game but zero people in table");
    return 0;
  }

  //Set game in progress
  a[table_id]->status = IN_GAME;
  
  sprintf(msg, "GS:%d:2:%d:%d:1:%d",table_id, a[table_id]->pl_in, a[table_id]->max_pl, a[table_id]->min_pl);
  //Build pkt
  for (i=0;i<(a[table_id]->max_pl);i++) {
    if (a[table_id]->p_l[i] != NULL) {
      if (i == 0)
	sprintf(msg, "%s:1,%s,%d", msg, a[table_id]->p_l[i]->username, ntohl(a[table_id]->p_l[i]->addr.sin_addr.s_addr));
      else
	sprintf(msg, "%s,1,%s,%d", msg, a[table_id]->p_l[i]->username, ntohl(a[table_id]->p_l[i]->addr.sin_addr.s_addr));
    }
  }
  empty_slots = (a[table_id]->max_pl)-(a[table_id]->pl_in);
  for(i=0; i<empty_slots; i++) {
    strncat(msg, ",0,0,0", 6);
  }
 
  strncat(msg, ":", 1);
  pkt_size = strlen(msg);
  msg[++pkt_size] = '\0';

  //Send TS if location is DOROBO or SPLASH
  if (pl->location == DOROBO || pl->location == SPLASH) {
    for (i=0;i<(a[table_id]->max_pl);i++) {
      if (a[table_id]->p_l[i] != NULL)
	snd_table_ts(s, a[table_id]->p_l[i], table_id);
    }
  }
  
  //Send GS first
  for (i=0;i<(a[table_id]->max_pl);i++) {
    if (a[table_id]->p_l[i] != NULL) {
      write(a[table_id]->p_l[i]->sock, msg, pkt_size);
      a[table_id]->p_l[i]->status = PLAYING_GAME;
    }
  }
  
  snd_tr_to_attraction(s, a[table_id], pl->location);
  
  //Set ranking item to not in use
  for (i=0;i<(a[table_id]->max_pl);i++) {
    memset(a[table_id]->rank_pool[i]->username, 0, MAX_UNAME_LEN);
    a[table_id]->rank_pool[i]->in_use = 0;
  }
    
  return 0;
}

/*
 * Function: pr_ring_out
 * --------------------
 *
 * Function that handles if a player
 * leaves the game
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_ring_out(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  char s_msg[MAX_PKT_SIZE];
  int pkt_size=0, socket_desc=0, i=0;
  struct sockaddr_in client = { 0 };
  memset(s_msg, 0, sizeof(s_msg));
    
  if (nr_parsed < 1) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }

  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }

  pkt_size = sprintf(s_msg, "UO:%s:", tok_array[0]);
  s_msg[++pkt_size] = '\0';

  //Send UO to all
  for(i=0;i<(s->m_cli); i++) {
    if(s->p_l[i] != NULL) {
      if ((s->p_l[i]->online == 1)) {
	client = s->p_l[i]->addr;
	client.sin_port = htons(CLIENT_UDP_PORT);
	socket_desc = get_udp_sock_from_location(s, s->p_l[i]->location);
	sendto(socket_desc, s_msg, (size_t)pkt_size, 0, (struct sockaddr*)&client, (socklen_t)sizeof(struct sockaddr_in));
      }
    }
  }

  sanity_check(pl);
  pl->online = 0;

  return (uint16_t)pkt_size;
}

/*
 * Function: pr_game_exit
 * --------------------
 *
 * Function that handles if a player
 * leaves a mini game
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_game_exit(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size = 0;
  uint8_t table_id = 0;
  int i=0;
  attraction_t **a = NULL;
  
  if (nr_parsed < 2) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }

  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }

  table_id = (uint8_t)str2int(tok_array[1]);
    
  a = get_attraction_from_location(s, pl->location);
  if (a == NULL) {
    planetring_error("Could not get attraction from location %d", pl->location);
    return 0;
  }
    
  pkt_size = sprintf(msg, "GX:%s:", tok_array[0]);
  msg[++pkt_size] = '\0';
    
  for (i=0;i<(a[table_id]->max_pl);i++) {
    if (a[table_id]->p_l[i] != NULL) {
      write(a[table_id]->p_l[i]->sock, msg, (size_t)pkt_size);
    }
  }

  //Set status to ready
  pl->status = RUNNING_AROUND;
  
  leave_table_in_attraction(pl, table_id);
  return 0;
}

/*
 * Function: pr_ranking_end
 * --------------------
 *
 * Function that stores the ranking
 * after a mini game has ended
 * 
 *  *s:          ptr to server data struct
 *  *cli         ptr to connect player data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_ranking_end(server_data_t *s, player_t* cli, char *msg, int nr_parsed, char **tok_array) {
  int ret=0;
  uint8_t table_id=0,rankmode=0;
  uint16_t nfo_1=0,nfo_2=0,nfo_3=0,nfo_4=0,nfo_5=0,nfo_6=0;
  
  if (nr_parsed < 6) {
    planetring_error("Invalid number of inputs");
    return 0;
  } 
      
  if (cli->username == NULL) {
    planetring_info("Did not get a username");
    return 0;
  }
     
  player_t *pl = get_user_from_nick(s, cli->username);
  if (pl == NULL) {
    planetring_error("Could not get user %s", cli->username);
    return 0;
  }
  
  table_id = (uint8_t)str2int(tok_array[1]);
  rankmode = (uint8_t)str2int(tok_array[2]);

  if (pl->location == BALLBUBBLE) {
    if (nr_parsed < 7)
      return 0;
    if (nr_parsed == 7) {
      nfo_1 = (uint16_t)str2int(tok_array[3]);
      nfo_2 = (uint16_t)str2int(tok_array[4]);
      nfo_3 = (uint16_t)str2int(tok_array[5]);
      nfo_4 = (uint16_t)str2int(tok_array[6]);
    } else if (nr_parsed == 8) {
      nfo_1 = (uint16_t)str2int(tok_array[4]);
      nfo_2 = (uint16_t)str2int(tok_array[5]);
      nfo_3 = (uint16_t)str2int(tok_array[6]);
      nfo_4 = (uint16_t)str2int(tok_array[7]);
    } else if (nr_parsed == 9) {
      nfo_1 = (uint16_t)str2int(tok_array[5]);
      nfo_2 = (uint16_t)str2int(tok_array[6]);
      nfo_3 = (uint16_t)str2int(tok_array[7]);
      nfo_4 = (uint16_t)str2int(tok_array[8]);
    } else {
      planetring_info("Got location %d with nr_parsed %d", pl->location, nr_parsed);
      return 0;
    }
  } else if (pl->location == SPLASH) {
    if (nr_parsed != 10)
      return 0;
    nfo_1 = (uint16_t)str2int(tok_array[4]);
    nfo_2 = (uint16_t)str2int(tok_array[5]);
    nfo_3 = (uint16_t)str2int(tok_array[6]);
    nfo_4 = (uint16_t)str2int(tok_array[7]);
    nfo_5 = (uint16_t)str2int(tok_array[8]);
    nfo_6 = (uint16_t)str2int(tok_array[9]);
  } else if (pl->location == DOROBO) {
    if (nr_parsed != 8)
      return 0;
    nfo_1 = (uint16_t)str2int(tok_array[4]);
    nfo_2 = (uint16_t)str2int(tok_array[5]);
    nfo_3 = (uint16_t)str2int(tok_array[6]);
    nfo_4 = (uint16_t)str2int(tok_array[7]);
  } else if (pl->location == SOAR) {
    if (nr_parsed < 8)
      return 0;
    //1-2 players
    if (nr_parsed == 8) {
      nfo_1 = (uint16_t)str2int(tok_array[4]);
      nfo_2 = (uint16_t)str2int(tok_array[5]);
      nfo_3 = (uint16_t)str2int(tok_array[6]);
      nfo_4 = (uint16_t)str2int(tok_array[7]);
      //3 players
    } else if (nr_parsed == 9) {
      nfo_1 = (uint16_t)str2int(tok_array[5]);
      nfo_2 = (uint16_t)str2int(tok_array[6]);
      nfo_3 = (uint16_t)str2int(tok_array[7]);
      nfo_4 = (uint16_t)str2int(tok_array[8]);
      //4 players
    } else if (nr_parsed == 10) {
      nfo_1 = (uint16_t)str2int(tok_array[6]);
      nfo_2 = (uint16_t)str2int(tok_array[7]);
      nfo_3 = (uint16_t)str2int(tok_array[8]);
      nfo_4 = (uint16_t)str2int(tok_array[9]);
      //5 players
    } else if (nr_parsed == 11) {
      nfo_1 = (uint16_t)str2int(tok_array[7]);
      nfo_2 = (uint16_t)str2int(tok_array[8]);
      nfo_3 = (uint16_t)str2int(tok_array[9]);
      nfo_4 = (uint16_t)str2int(tok_array[10]);
      //6 players
    } else if (nr_parsed == 12) {
      nfo_1 = (uint16_t)str2int(tok_array[8]);
      nfo_2 = (uint16_t)str2int(tok_array[9]);
      nfo_3 = (uint16_t)str2int(tok_array[10]);
      nfo_4 = (uint16_t)str2int(tok_array[11]);
      //7 players
    } else if (nr_parsed == 13) {
      nfo_1 = (uint16_t)str2int(tok_array[9]);
      nfo_2 = (uint16_t)str2int(tok_array[10]);
      nfo_3 = (uint16_t)str2int(tok_array[11]);
      nfo_4 = (uint16_t)str2int(tok_array[12]);
      //8 players
    } else if (nr_parsed == 14) {
      nfo_1 = (uint16_t)str2int(tok_array[10]);
      nfo_2 = (uint16_t)str2int(tok_array[11]);
      nfo_3 = (uint16_t)str2int(tok_array[12]);
      nfo_4 = (uint16_t)str2int(tok_array[13]);
    } else {
      planetring_info("Got location %d with nr_parsed %d", pl->location, nr_parsed);
      return 0;
    }
  } else {
    planetring_error("Not a valid location");
    return 0;
  }

  planetring_info("Got ranking RANKMODE:%d (%d:%d:%d:%d:(%d:%d)) from user %s in location: %d",
		  rankmode,
		  nfo_1,
		  nfo_2,
		  nfo_3,
		  nfo_4,
		  nfo_5,
		  nfo_6,
		  pl->username,
		  pl->location);

  //Don't store 0 values in SOAR
  if (nfo_4 == 0 && pl->location == SOAR && rankmode != 0)
    return 0;

  ret = update_rank(s,
		    pl,
		    table_id,
		    rankmode,
		    nfo_1,
		    nfo_2,
		    nfo_3,
		    nfo_4,
		    nfo_5,
		    nfo_6);
  if (ret != 1)
    planetring_error("Failed to store user: %s ranking", pl->username);

  return 0;
}

/*
 * Function: pr_profile_query
 * --------------------
 *
 * Function that return the profile
 * information for the player.
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_profile_query(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  int pkt_size=0;
    
  if (nr_parsed < 1) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[0]) > 16)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
  
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }
  
  pkt_size = sprintf(msg, "PR:%s:%d:%s:%d:%d:%d:%d:%d:%d:%d:%d:",
		     pl->username,
		     pl->gender,
		     pl->city,
		     pl->c_a_c,
		     pl->language,
		     pl->birthday,
		     pl->intelligence,
		     pl->kindness,
		     pl->looks,
		     pl->c_d,
		     planetring_profile_dates());
  msg[++pkt_size] = '\0';

  return (uint16_t)pkt_size;
}

/*
 * Function: pr_profile_update
 * --------------------
 *
 * Function that updates the profile
 * information for the player.
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_profile_update(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  int ret=0;
  
  if (nr_parsed < 10) {
    planetring_error("Invalid number of inputs");
    return 0;
  }

  if ((strlen(tok_array[0]) > 16) ||
      (strlen(tok_array[1]) > 16) ||
      (strlen(tok_array[3]) > 256)) {
    planetring_error("Think about the buffers....");
    return 0;
  }
  
  
  player_t *pl = get_user_from_nick(s, tok_array[0]);
  if (pl == NULL) {
    planetring_error("Could not get user %s", tok_array[0]);
    return 0;
  }

  strlcpy(pl->username, tok_array[0], strlen(tok_array[0])+1);
  pl->gender = (uint8_t)str2int(tok_array[2]);
  memset(pl->city, 0, 256);
  strlcpy(pl->city, tok_array[3], strlen(tok_array[3])+1);

  pl->c_a_c = (uint32_t)str2int(tok_array[4]);
  pl->language = (uint32_t)str2int(tok_array[5]);
  pl->birthday = (uint32_t)str2int(tok_array[6]);
  pl->intelligence = (uint8_t)str2int(tok_array[7]);
  pl->kindness = (uint8_t)str2int(tok_array[8]);
  pl->looks = (uint8_t)str2int(tok_array[9]);
    
  ret = update_player_in_planetring_db(s->db,
				       pl->username,
				       tok_array[1],
				       pl->city,
				       pl->gender,
				       pl->c_a_c,
				       pl->language,
				       pl->birthday,
				       pl->intelligence,
				       pl->kindness,
				       pl->looks);
  
  if (ret != 1) {
    planetring_error("Could not update information for user: %s", pl->username);
    return 0;
  }
    
  return 0;
}

/*
 * Function: pr_ranking_start
 * --------------------
 *
 * Function that doesn't do anything
 * right now, is triggered when a mini game starts
 * 
 *  *s:          ptr to server data struct
 *  *msg:        ptr to outgoing client msg
 *  nr_parsed:   nr strings parsed
 *  tok_array:   array of parsed strings
 *
 *  returns: pkt size
 *
 */
uint16_t pr_ranking_start(server_data_t *s, char *msg, int nr_parsed, char **tok_array) {
  uint8_t table=0, rankmode=0;
   
  if (nr_parsed < 3) {
    planetring_error("Invalid number of inputs");
    return 0;
  }
  
  table = (uint8_t)str2int(tok_array[1]);
  rankmode = (uint8_t)str2int(tok_array[2]);
  
  planetring_info("Player joined a game in table:%d with rankmode:%d", table, rankmode);
  
  return 0;
}
