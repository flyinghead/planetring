#include <sqlite3.h> 

sqlite3* open_planetring_db(const char* db_path);
int write_player_to_planetring_db(sqlite3* db, const char* username, const char* passwd, const char* city, uint8_t gender, uint32_t c_a_c, uint32_t language, uint32_t birthday, uint8_t intelligence, uint8_t kindness, uint8_t looks, unsigned int c_d);
int update_player_in_planetring_db(sqlite3* db, const char* username, const char* passwd, const char* city, uint8_t gender, uint32_t c_a_c, uint32_t language, uint32_t birthday, uint8_t intelligence, uint8_t kindness, uint8_t looks);
int is_player_in_planetring_db(sqlite3* db, const char* username);
int is_username_taken(sqlite3* db, const char* u_name);
int validate_player_login(sqlite3* db, const char* u_name, const char* passwd);
int load_players_to_array(server_data_t *s);
int update_rank(server_data_t* s, player_t *pl, uint8_t table_id, uint8_t rankmode, uint16_t nfo_1, uint16_t nfo_2, uint16_t nfo_3, uint16_t nfo_4, uint16_t nfo_5, uint16_t nfo_6);
int get_rank_for_player(sqlite3* db,player_t *pl, const char* username, uint8_t rankmode);
