/*

  Planet Ring MSG functions header for Dreamcast
  Author Shuouma

*/

//MASS-MESSAGES
void snd_table_ts(server_data_t *s, player_t *pl, uint8_t table_id);
void snd_player_ts(server_data_t *s, player_t *pl, uint8_t table_id, int position, int status);
void snd_tr_to_attraction(server_data_t* s, attraction_t *a, uint8_t location);
void snd_trs_to_player(player_t *pl);
int join_table_in_attraction(player_t *pl, uint8_t table_id);
void leave_table_in_attraction(player_t *pl, uint8_t table_id);

//MESSAGES

//UDP
uint16_t pr_table_query_udp(server_data_t *s, int nr_parsed, char **tok_array);
uint16_t pr_user_query_alive_udp(server_data_t *s, struct sockaddr_in *client, int socket_desc, int nr_parsed, char **tok_array);
uint16_t pr_message_udp(server_data_t *s, struct sockaddr_in *client, int socket_desc, int nr_parsed, char **tok_array);
uint16_t pr_profile_provider_udp(server_data_t *s, struct sockaddr_in *client, int socket_desc, int nr_parsed, char **tok_array);
uint16_t pr_ranking_pool_udp(server_data_t *s, struct sockaddr_in *client, int socket_desc, int nr_parsed, char **tok_array);

//TCP
uint16_t pr_submit_user(server_data_t *s, char* msg, int nr_parsed, char **tok_array);
uint16_t pr_new_user(server_data_t *s, player_t *cli, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_check_user(server_data_t *s, player_t *cli, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_move_to(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_entry_request(server_data_t *s, player_t * cli, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_attraction_query(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_table_join(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_table_leave(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_game_query(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_ring_out(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_game_exit(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_ranking_end(server_data_t *s, player_t* cli, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_profile_query(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_profile_update(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
uint16_t pr_ranking_start(server_data_t *s, char *msg, int nr_parsed, char **tok_array);
