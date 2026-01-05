#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sqlite3.h>

#define CLIENT_UDP_PORT 7648
#define MAX_PKT_SIZE 4096
#define MAX_TEML_PKT_SIZE 12288
#define MAX_UNAME_LEN 17

typedef struct {
  int sock;
  struct sockaddr_in addr;
  char username[MAX_UNAME_LEN];
  char city[256];
  uint8_t status;
  uint8_t gender;
  uint8_t online, location, table;
  uint8_t intelligence, kindness, looks;
  uint8_t microphone;
  uint32_t birthday;
  uint32_t language;
  uint32_t c_a_c;
  unsigned int c_d;
  uint8_t tbl_id;
  uint32_t keepalive;

  void *data;
} player_t;

typedef struct {
  int in_use;
  char username[MAX_UNAME_LEN];
  //Page 1
  int p1col1; 
  int p1col2;
  int p1col3;
  int p1col4;
  int p1col5;
  int p1col6;
  //Page 2
  int p2col1;
  int p2col2;
  int p2col3;
  int p2col4;
} rank_t;
  
typedef struct {
  uint8_t id_tbl;
  uint8_t min_pl;
  uint8_t max_pl;
  uint8_t status;
  uint8_t pl_in;
  rank_t **rank_pool;
  player_t **p_l;
} attraction_t;

typedef struct {
  int m_cli;
  int c_cli;
  int m_tables;
  uint16_t pr_port;
  char pr_ip[INET_ADDRSTRLEN];
  char pr_db_path[256];
  char pr_teml_path[256];
  
  int info_sock;
  int planet_sock;
  int splash_sock;
  int bb_sock;
  int soar_sock;
  int doro_sock;

  //DB
  sqlite3 *db;
  
  //Data
  player_t **p_l;
  attraction_t **splash;
  attraction_t **ball_bubble;
  attraction_t **soar;
  attraction_t **dorobo;
} server_data_t;

typedef enum {
  DOOR = 0,
  INFO = 1,
  PLANET = 2,
  SPLASH = 10,
  BALLBUBBLE = 11,
  SOAR = 12,
  DOROBO = 13,
} LOCATION;

typedef enum {
  EMPTY = 0,
  WAIT = 1,
  CLOSE = 2,
  IN_GAME = 3,
} TBL_STATUS;

typedef enum {
  RUNNING_AROUND = 0,
  SITTING_IN_TABLE = 1,
  PLAYING_GAME = 2,
} PL_STATUS;

//Client handler
void *planetring_client_handler(void *data);

//Print functions
void planetring_error(const char* format, ... );
void planetring_info(const char* format, ... );

//Parse config
int get_planetring_config(server_data_t *s, char *fn);

//Help
uint32_t strlcpy(char *dst, const char *src, size_t size);
int str2int(char const* str);
uint32_t char_to_uint32(char* data);
uint16_t char_to_uint16(char* data);
int uint32_to_char(uint32_t data, char* msg);
int uint16_to_char(uint16_t data, char* msg);

unsigned int planetring_profile_dates(void);
void print_planetring_data(void* ds,unsigned long data_size);
int get_udp_sock_from_location(server_data_t *s, uint8_t location);
player_t* get_user_from_addr(server_data_t *s, struct sockaddr_in *addr);
player_t* get_user_from_nick(server_data_t *s, char* u_name);
attraction_t** get_attraction_from_socket(server_data_t *s, int socket);
attraction_t** get_attraction_from_location(server_data_t *s, uint8_t location);
int get_socket_from_location(server_data_t *s, uint8_t location);
void sanity_check(player_t *pl);
void keepalive_check(server_data_t *s);
uint8_t get_zodiac(uint32_t birthday);
int get_age(uint32_t birthday);
