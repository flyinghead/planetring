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
 * Planet Ring Server for Dreamcast
 */


#include <stdlib.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>   
#include <pthread.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#ifdef DCNET
#include <dcserver/status.h>
#endif
#include "planetring_common.h"
#include "planetring_sql.h"
#include "planetring_msg.h"
#include "planetring_teml.h"

/*
 * Function: init_attractions
 * --------------------
 *
 * Allocates memory for the attractions
 * 
 *  *s: ptr to server data struct
 *
 *  returns: void
 *
 */
void init_attractions(server_data_t *s) {
  int i=0,j=0;
  int max_tables = s->m_tables;
  int max_players = 4;

  for(i=0;i<max_tables;i++) {
    if (s->splash[i] == NULL) { 
      attraction_t *t = (attraction_t *)malloc(sizeof(attraction_t));
      t->id_tbl = (uint8_t)i;
      t->min_pl = 2;
      t->max_pl = 2;
      t->status = 0;
      t->pl_in = 0;
      t->p_l = calloc(2, sizeof(player_t *));
      for(j=0;j<2;j++)
	t->p_l[j] = NULL;
      
      t->rank_pool = calloc(t->max_pl,sizeof(rank_t *));
      for(j=0;j<t->max_pl;j++) {
	t->rank_pool[j] = (rank_t *)calloc(1, sizeof(rank_t));
      }    
      s->splash[i] = t;
    }
  }
  for(i=0;i<max_tables;i++) {
    if (s->ball_bubble[i] == NULL) { 
      attraction_t *t = (attraction_t *)malloc(sizeof(attraction_t));
      t->id_tbl = (uint8_t)i;
      t->min_pl = 2;
      t->max_pl = (uint8_t)max_players;
      t->status = 0;
      t->pl_in = 0;
      t->p_l = calloc((size_t)max_players, sizeof(player_t *));
      for(j=0;j<max_players;j++)
	t->p_l[j] = NULL;
      
      t->rank_pool = calloc(t->max_pl,sizeof(rank_t *));
      for(j=0;j<t->max_pl;j++) {
	t->rank_pool[j] = (rank_t *)calloc(1, sizeof(rank_t));
      }   
      s->ball_bubble[i] = t;
    }
  }
  for(i=0;i<max_tables;i++) {
    if (s->soar[i] == NULL) { 
      attraction_t *t = (attraction_t *)malloc(sizeof(attraction_t));
      t->id_tbl = (uint8_t)i;
      t->min_pl = 1;
      t->max_pl = 8;
      t->status = 0;
      t->pl_in = 0;
      t->p_l = calloc(8, sizeof(player_t *));    
      for(j=0;j<8;j++)
	t->p_l[j] = NULL;

      t->rank_pool = calloc(t->max_pl,sizeof(rank_t *));
      for(j=0;j<t->max_pl;j++) {
	t->rank_pool[j] = (rank_t *)calloc(1, sizeof(rank_t));
      }  
      s->soar[i] = t;
    }
  }
  for(i=0;i<max_tables;i++) {
    if (s->dorobo[i] == NULL) { 
      attraction_t *t = (attraction_t *)malloc(sizeof(attraction_t));
      t->id_tbl = (uint8_t)i;
      t->min_pl = 2;
      t->max_pl = 2;
      t->status = 0;
      t->pl_in = 0;
      t->p_l = calloc(2, sizeof(player_t *));
      for(j=0;j<2;j++)
	t->p_l[j] = NULL;
      
      t->rank_pool = calloc(t->max_pl,sizeof(rank_t *));
      for(j=0;j<t->max_pl;j++) {
	t->rank_pool[j] = (rank_t *)calloc(1, sizeof(rank_t));
      }  
      s->dorobo[i] = t;
    }
  }
}

/*
 * Function: udp_msg_handler
 * --------------------
 *
 * Function that handles incoming UDP pkt
 * 
 *  *buf:        ptr to buffer
 *   buf_len:    size of buffer
 *  *s:          ptr to server data struct
 *  *client:     ptr to client sockaddr struct
 *  socket_desc: udp socket
 *
 *  returns: size of pkt
 *           
 */
int udp_msg_handler(char* buf, int buf_len, server_data_t *s, struct sockaddr_in *client, int socket_desc) {
  char *token = NULL;
  char *cmd = NULL;
  int nr_parsed = 0, i=0;
  char *tok_array[256];

  buf[buf_len] = '\0';
  if (buf_len > 0 && buf[buf_len - 1] == '\n')
    buf[--buf_len] = '\0';
  
  //Check if user is legit
  if ( (get_user_from_addr(s, client) == NULL) )
    return 0;
  
  cmd = strtok (buf,":");
  if (strlen(cmd) > 4 || strlen(cmd) < 2) {
    planetring_error("CMD: %s not supported",cmd);
    return 0;
  }
  
  token = strtok(NULL, ":");
  while(token != NULL) {
    if (nr_parsed == 256) {
      return 0;
    }
    tok_array[nr_parsed] = token;
    token = strtok(NULL, ":");
    nr_parsed++;
  }

  for (i=0;i<nr_parsed;i++) {
    if (tok_array[i] == NULL) {
      planetring_error("Index %d contains NULL", i);
      return 0;
    }
  }
  
  if (memcmp(cmd, "TQ", 2) == 0)
    return pr_table_query_udp(s, nr_parsed, tok_array);
    
  if (memcmp(cmd, "UQ", 2) == 0)
    return pr_user_query_alive_udp(s, client, socket_desc, nr_parsed, tok_array);
    
  if (memcmp(cmd, "MS", 2) == 0)
    return pr_message_udp(s, client, socket_desc, nr_parsed, tok_array);
    
  if (memcmp(cmd, "AL", 2) == 0)
    return pr_user_query_alive_udp(s, client, socket_desc, nr_parsed, tok_array);
    
  if (memcmp(cmd, "RQ", 2) == 0)
    return pr_ranking_pool_udp(s, client, socket_desc, nr_parsed, tok_array);
    
  if (memcmp(cmd, "PL", 2) == 0)
    return pr_profile_provider_udp(s, client, socket_desc, nr_parsed, tok_array);
  
  planetring_error("Unsupported UDP CMD [%s]",cmd);
  return 0;
}

/*
 * Function: tcp_msg_handler
 * --------------------
 *
 * Function that handles incoming TCP pkt
 * 
 *  *cli:        ptr to current player struct
 *  *msg:        ptr to outgoing msg
 *  *buf:        ptr to intcoming msg
 *  buf_len:     size of incoming msg
 *
 *  returns: size of pkt
 *           
 */
int tcp_msg_handler(player_t *cli, char* msg, char* buf, int buf_len) {
  char *token = NULL;
  char *cmd = NULL;
  int nr_parsed = 0, i=0;
  char *tok_array[256];
  server_data_t *s = (server_data_t*)cli->data;

  //The planet ring protocol adds \0 at the end of each pkt, but
  //to be sure lets add it ourself aswell.
  buf[buf_len] = '\0';
  if (buf_len > 0 && buf[buf_len - 1] == '\n')
    buf[--buf_len] = '\0';
  
  
  cmd = strtok (buf,":");
  if (strlen(cmd) > 4 || strlen(cmd) < 2) {
    planetring_error("CMD: %s not supported",cmd);
    return 0;
  }

  //Check if user is legit
  if ((memcmp(cmd, "SU", 2) != 0) &&
      (memcmp(cmd, "NU", 2) != 0) &&
      (memcmp(cmd, "CU", 2) != 0)) {
    if ( (get_user_from_addr(s, &cli->addr) == NULL) )
      return -1;
  }
  
  token = strtok(NULL, ":");
  while(token != NULL) {
    if (nr_parsed == 256) {
      return -1;
    }
    tok_array[nr_parsed] = token;
    token = strtok(NULL, ":");
    nr_parsed++;
  }

  for (i=0;i<nr_parsed;i++) {
    if (tok_array[i] == NULL) {
      planetring_error("Index %d contains NULL", i);
      return 0;
    }
  }
  
  if (memcmp(cmd, "SU", 2) == 0)
    return pr_submit_user(s, msg, nr_parsed, tok_array);
  
  if (memcmp(cmd, "NU", 2) == 0)
    return pr_new_user(s, cli, msg, nr_parsed, tok_array);
    
  if (memcmp(cmd, "CU", 2) == 0)
    return pr_check_user(s, cli, msg, nr_parsed, tok_array);
    
  if (memcmp(cmd, "OT", 2) == 0)
    return pr_move_to(s, msg, nr_parsed, tok_array);
 
  if (memcmp(cmd, "IN", 2) == 0)
    return pr_entry_request(s, cli, msg, nr_parsed, tok_array);

  if (memcmp(cmd, "AQ", 2) == 0)
    return pr_attraction_query(s, msg, nr_parsed, tok_array);
    
  if (memcmp(cmd, "TI", 2) == 0)
    return pr_table_join(s, msg, nr_parsed, tok_array);
    
  if (memcmp(cmd, "TO", 2) == 0)
    return pr_table_leave(s, msg, nr_parsed, tok_array);
    
  if (memcmp(cmd, "GQ", 2) == 0)
    return pr_game_query(s, msg, nr_parsed, tok_array);
 
  if (memcmp(cmd, "RO", 2) == 0)
    return pr_ring_out(s, msg, nr_parsed, tok_array);
 
  if (memcmp(cmd, "TRO", 3) == 0)
    return pr_ring_out(s, msg, nr_parsed, tok_array);
   
  if (memcmp(cmd, "GE", 2) == 0)
    return pr_game_exit(s, msg, nr_parsed, tok_array);
  
  if (memcmp(cmd, "RE", 2) == 0)
    return pr_ranking_end(s, cli, msg, nr_parsed, tok_array); 
   
  if (memcmp(cmd, "PQ", 2) == 0)
    return pr_profile_query(s, msg, nr_parsed, tok_array);
  
  if (memcmp(cmd, "PM", 2) == 0)
    return pr_profile_update(s, msg, nr_parsed, tok_array);

  if (memcmp(cmd, "RS", 2) == 0)
    return pr_ranking_start(s, msg, nr_parsed, tok_array);
      
  planetring_error("Unsupported TCP CMD [%s]",cmd);
  return 0;
}

/*
 * Function: planetring_client_handler
 * --------------------
 *
 * Function that handles incoming TCP connections
 * 
 *  *data:        ptr to player data struct
 *
 *  returns: void
 *           
 */
void *planetring_client_handler(void *data) {
  player_t *pl = (player_t *)data;
  int sock = pl->sock; 
  ssize_t read_size=0;
  int write_size=0;
  char c_msg[MAX_PKT_SIZE], s_msg[MAX_PKT_SIZE];
  player_t *strd_pl = NULL;
  memset(c_msg, 0, sizeof(c_msg));
  memset(s_msg, 0, sizeof(s_msg));

  struct timeval tv;
  tv.tv_sec = 1800;       /* Timeout in seconds */
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(char *)&tv,sizeof(struct timeval));
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval));
    
  //Receive a message from client
  while( (read_size = recv(sock , c_msg , sizeof(c_msg) , 0)) > 0 ) {
    //Parse msg
    write_size = tcp_msg_handler(pl, s_msg, c_msg, (int)read_size);
    if (write_size > 0) {
      write(sock, s_msg, (size_t)write_size);
    }
    if (write_size < 0) {
      close(sock);
      free(pl);
      return 0;
    }
    memset(s_msg, 0, sizeof(s_msg));
    memset(c_msg, 0, sizeof(c_msg));
  }
  
  if(read_size == 0) {
    planetring_info("Client with socket %d [%s] disconnected", sock, inet_ntoa(pl->addr.sin_addr));
  } else if(read_size == -1) {
    planetring_info("Recv failed socket %d [%s]", sock, inet_ntoa(pl->addr.sin_addr));
    //Usually this happens, when a timeout occurs
    strd_pl = get_user_from_nick((server_data_t*)pl->data, pl->username);
    //Check if timed out and never reconnected, set user to offline
    if (strd_pl != NULL) {
      //The Keepalive check should have set the user to offline
      if (strd_pl->online == 0 ) {
	planetring_info("Time out occured set user to offline");
	//Just to be safe
	sanity_check(pl);
      }
    }
  }
  close(sock);
  
  free(pl);
  return 0;
}

/*
 * Function: planetring_teml_server_handler
 * --------------------
 *
 * Function that handles the TEML server
 * 
 *  *data:        ptr to server data struct
 *
 *  returns: void
 *           
 */
void *planetring_teml_server_handler(void *data) {
  server_data_t *s_data = (server_data_t *)data;
  int socket_desc , client_sock , c, optval;
  struct sockaddr_in server = { 0 }, client = { 0 };
  
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1) {
    planetring_info("Could not create socket");
    return 0;
  }

  optval = 1;
  setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
  
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( 8001 );
  
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
    planetring_error("Bind failed. Error");
    return 0;
  }
  planetring_info("Started TCP listener on port: %d", ntohs(server.sin_port));
  
  listen(socket_desc , 3);

  pthread_t thread_id;
  c = sizeof(struct sockaddr_in);
   
  while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
    planetring_info("Connection accepted from %s on socket %d", inet_ntoa(client.sin_addr), client_sock);
    //Store player data
    player_t *pl = (player_t *)malloc(sizeof(player_t));
    memset(pl->username, 0, MAX_UNAME_LEN);
    pl->addr = client;
    pl->sock = client_sock;
    pl->data = s_data;
    
    if( pthread_create( &thread_id , NULL ,  planetring_teml_client_handler , (void*)pl) < 0) {
      planetring_error("Could not create thread");
      return 0;
    }
    pthread_detach(thread_id);
  }
  
  if (client_sock < 0) {
    planetring_error("Accept failed");
    return 0;
  }
  
  return 0;
}

/*
 * Function: planetring_keepalive_server_handler
 * --------------------
 *
 * Function that checks the keepalive value of online
 * players.
 * 
 *  *data:        ptr to server data struct
 *
 *  returns: void
 *           
 */
void *planetring_keepalive_server_handler(void *data) {
  server_data_t *s_data = (server_data_t *)data;

  while(1) {
    keepalive_check(s_data);
    sleep(120);
  }
  
  return 0;
}

/*
 * Function: planetring_udp_server_handler
 * --------------------
 *
 * Function that handles incoming udp pkt
 * for each attraction socket
 * 
 *  *data:        ptr to server data struct
 *
 *  returns: void
 *           
 */
void *planetring_udp_server_handler(void *data) {
  char c_msg[MAX_PKT_SIZE];
  ssize_t read_size = 0;
  int i = 0, socket_desc = 0, fdmax = 0, flags = 0;
  int udp_ports[] = { 7649, 7650, 7651, 7652, 7653, 7654 };
  struct sockaddr_in server = { 0 }, client = { 0 };
  server_data_t *s_data = (server_data_t *)data;
  socklen_t slen = sizeof(client);
  fd_set master, read_fds;
  
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  
  for (i = 0; i < 6; i++) {
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc == -1) {
      planetring_info("Could not create socket");
      return 0;
    }

    flags = fcntl(socket_desc, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(socket_desc, F_SETFL, flags);
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( (uint16_t)udp_ports[i] );
    
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
      planetring_error("Bind failed. Error");
      return 0;
    }

    if (i == 0)
      s_data->info_sock = socket_desc;
    if (i == 1)
      s_data->planet_sock = socket_desc;
    if (i == 2)
      s_data->splash_sock = socket_desc;
    if (i == 3)
      s_data->bb_sock = socket_desc;
    if (i == 4)
      s_data->soar_sock = socket_desc;
    if (i == 5)
      s_data->doro_sock = socket_desc;
    
    planetring_info("Started UDP listener on port: %d", udp_ports[i]);

    FD_SET(socket_desc, &master);
    fdmax = socket_desc;
  }

  while (1) {
    read_fds = master;
    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
      perror("select");
      return 0;
    }

    for(i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_fds)) {
	read_size = recvfrom(i, c_msg, sizeof(c_msg), 0, (struct sockaddr *)&client, &slen);

	if (read_size < 0)
	  planetring_error("ERROR in recvfrom");

	client.sin_port = htons(CLIENT_UDP_PORT);
	udp_msg_handler(c_msg, (int)read_size, s_data, &client, i);
	memset(c_msg, 0, sizeof(c_msg));
      }
    }
  }

  return 0;
}

static int active_games(attraction_t **attraction, int max_tables)
{
	int count = 0;
	for (int i = 0; i < max_tables; i++)
		if (attraction[i]->status == IN_GAME)
			count++;
	return count;
}

#ifdef DCNET
static void *planetring_status_updater(void *data)
{
	server_data_t *server = (server_data_t *)data;
	for (;;)
	{
		int playerCount = 0;
		for (int i = 0; i < server->m_cli; i++)
			if (server->p_l[i] != NULL && server->p_l[i]->online == 1)
				playerCount++;
		int gameCount = 0;
		gameCount += active_games(server->splash, server->m_tables);
		gameCount += active_games(server->ball_bubble, server->m_tables);
		gameCount += active_games(server->soar, server->m_tables);
		gameCount += active_games(server->dorobo, server->m_tables);

		statusUpdate("planetring", playerCount, gameCount);
		statusCommit("planetring");

		sleep(statusGetInterval());
	}
	return NULL;
}
#endif

/*
 * Function: main
 * --------------------
 *
 * Main function
 *
 *  returns: int
 *           
 */
int main(int argc , char *argv[]) {
  int socket_desc , client_sock , c, optval;
  struct sockaddr_in server , client;
  server_data_t s_data;

  //Read cfg file
  if (!get_planetring_config(&s_data, argc >= 2 ? argv[1] : "planetring.cfg"))
    return 0;

  //OK - Connect to DB
  s_data.db = open_planetring_db(s_data.pr_db_path);
  if (s_data.db == NULL) {
    planetring_error("Could not connect to database");
    return 0;
  }

  //Load player to memory
  if (!load_players_to_array(&s_data)) {
    sqlite3_close(s_data.db);
    return 0;
  }

  //Init attractions
  init_attractions(&s_data);
  
  pthread_t thread_id_udp;
  if( pthread_create( &thread_id_udp , NULL ,  planetring_udp_server_handler , (void*)&s_data) < 0) {
    perror("Could not create thread");
    sqlite3_close(s_data.db);
    return -1;
  }
  pthread_detach(thread_id_udp);

  pthread_t thread_id_teml;
  if( pthread_create( &thread_id_teml , NULL ,  planetring_teml_server_handler , (void*)&s_data) < 0) {
    perror("Could not create thread");
    sqlite3_close(s_data.db);
    return -1;
  }
  pthread_detach(thread_id_teml);

  pthread_t thread_id_keepalive;
  if( pthread_create( &thread_id_keepalive , NULL ,  planetring_keepalive_server_handler , (void*)&s_data) < 0) {
    perror("Could not create thread");
    sqlite3_close(s_data.db);
    return -1;
  }
  pthread_detach(thread_id_keepalive);
  
#ifdef DCNET
  pthread_t thread_id_status_updater;
  if (pthread_create(&thread_id_status_updater, NULL, planetring_status_updater, (void*)&s_data) < 0)
    perror("Could not create status updater thread");
  pthread_detach(thread_id_status_updater);
#endif

  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1) {
    sqlite3_close(s_data.db);
    planetring_error("Could not create socket");
    return 0;
  }
    
  optval = 1;
  setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
  
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( s_data.pr_port );
  
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
    sqlite3_close(s_data.db);
    planetring_error("Bind failed. Error");
    return 0;
  }
  planetring_info("Started TCP listener on port: %d", s_data.pr_port);
  
  listen(socket_desc , 3);
  
  c = sizeof(struct sockaddr_in);
  pthread_t thread_id;
  
  while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
    planetring_info("Connection accepted from %s on socket %d", inet_ntoa(client.sin_addr), client_sock);
    //Store player data
    player_t *pl = (player_t *)malloc(sizeof(player_t));
    memset(pl->username, 0, MAX_UNAME_LEN);
    pl->addr = client;
    pl->sock = client_sock;
    pl->data = &s_data;
    
    if( pthread_create( &thread_id , NULL ,  planetring_client_handler , (void*)pl) < 0) {
      sqlite3_close(s_data.db);
      perror("Could not create thread");
      return 0;
    }
    pthread_detach(thread_id);
  }
  
  if (client_sock < 0) {
    sqlite3_close(s_data.db);
    planetring_error("Accept failed");
    return 0;
  }

  sqlite3_close(s_data.db);
  return 0;
}
