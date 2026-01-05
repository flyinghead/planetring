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
 * Planet Ring SQL functions for Dreamcast
 */

#include <stdlib.h>
#include <sqlite3.h>
#include <assert.h>
#include "planetring_common.h"

sqlite3* open_planetring_db(const char* db_path) {
   sqlite3 *db = NULL;
   int rc = 0;
   rc = sqlite3_open(db_path, &db);
   if(rc) {
     planetring_error("Can't open database: %s", sqlite3_errmsg(db));
     return NULL;
   }

   sqlite3_busy_timeout(db, 1000);
   
   return db;
}

int is_player_in_planetring_db(sqlite3 *db, const char* username) {
  int rc, count = 0;
  sqlite3_stmt *pStmt;
  
  const char *zSql = "SELECT COUNT(*) from PLAYER_DATA WHERE USERNAME = trim(?);"; 
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error("Prepare SQL error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_text(pStmt, 1, username, (int)strlen(username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  
  rc = sqlite3_step(pStmt);
  if (rc == SQLITE_ROW )
    count = sqlite3_column_int(pStmt, 0);
  
  if (count == 1)
    planetring_info("Username: %s is registered in the DB", username);
  else {
    planetring_info("Username: %s is not in the DB", username);
    count = 2;
  }

  sqlite3_finalize(pStmt);
  return count;
}

int validate_player_login(sqlite3* db, const char* u_name, const char* passwd) {
  int rc, count = 0;
  sqlite3_stmt *pStmt;
  
  const char *zSql = "SELECT COUNT(*) from PLAYER_DATA WHERE USERNAME = trim(?) AND PASSWORD = trim(?);"; 
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_text(pStmt, 1, u_name, (int)strlen(u_name), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error( "Bind text failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_text(pStmt, 2, passwd, (int)strlen(passwd), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error( "Bind text failed error: %d", rc);
    return 0;
  } 
  
  rc = sqlite3_step(pStmt);
  if (rc == SQLITE_ROW )
    count = sqlite3_column_int(pStmt, 0);
  
  if (count == 1)
    planetring_info("Login granted for %s", u_name);
  else
    planetring_info("Login failed for %s", u_name);

  sqlite3_finalize(pStmt);
  return count;
}

int update_player_in_planetring_db(sqlite3* db, const char* username, const char* passwd, const char* city,
				   uint8_t gender, uint32_t c_a_c, uint32_t language, uint32_t birthday, uint8_t intelligence, uint8_t kindness, uint8_t looks) {
  
  int rc = 0;
  sqlite3_stmt *pStmt;
  
  const char* zSql = "UPDATE PLAYER_DATA SET CITY=trim(?), GENDER=?, CAC=?, LANGUAGE=?, BIRTHDAY=?, INTELLIGENCE=?, KINDNESS=?, LOOKS=? WHERE USERNAME=trim(?) AND PASSWORD=trim(?);";
  
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_text(pStmt, 1, city, (int)strlen(city), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 2, gender);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 3, (int)c_a_c);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 4, (int)language);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 5, (int)birthday);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 6, intelligence);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 7, kindness);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 8, looks);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  
  rc = sqlite3_bind_text(pStmt, 9, username, (int)strlen(username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_text(pStmt, 10, passwd, (int)strlen(passwd), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_step(pStmt);
  if (rc != SQLITE_DONE) {
    planetring_error("Update failed error: %d", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  planetring_info("User: %s updated profile successfully", username);
    
  return 1;
}

int write_player_to_planetring_db(sqlite3* db, const char* username, const char* passwd, const char* city,
				  uint8_t gender, uint32_t c_a_c, uint32_t language, uint32_t birthday, uint8_t intelligence, uint8_t kindness, uint8_t looks, unsigned int c_d) {
  int rc = 0;
  sqlite3_stmt *pStmt;
  
  const char* zSql = "INSERT INTO PLAYER_DATA (ID,USERNAME,PASSWORD,CITY,GENDER,CAC,LANGUAGE,BIRTHDAY,INTELLIGENCE,KINDNESS,LOOKS,C_D) VALUES (NULL, trim(?), trim(?), trim(?), ?, ?, ?, ?, ?, ?, ?, ?);";

  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  
  rc = sqlite3_bind_text(pStmt, 1, username, (int)strlen(username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_text(pStmt, 2, passwd, (int)strlen(passwd), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_text(pStmt, 3, city, (int)strlen(city), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 4, gender);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 5, (int)c_a_c);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 6, (int)language);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 7, (int)birthday);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 8, intelligence);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 9, kindness);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 10, looks);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 11, (int)c_d);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_step(pStmt);
  if (rc != SQLITE_DONE) {
    planetring_error("Insert failed error: %d", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  planetring_info("Records created successfully for %s", username);

  return 1;
}

int load_players_to_array(server_data_t *s) {

  sqlite3 *db = s->db;
  int rc=0, index=0;
  sqlite3_stmt *pStmt;
    
  const char *zSql = "SELECT USERNAME,CITY,GENDER,CAC,LANGUAGE,BIRTHDAY,INTELLIGENCE,KINDNESS,LOOKS,C_D FROM PLAYER_DATA;"; 
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error("Prepare SQL error: %d", rc);
    return 0;
  }

  //Player struct is empty here, index is 0 and goes up
  while (sqlite3_step(pStmt) == SQLITE_ROW ) {
    player_t *pl = (player_t *)calloc(1, sizeof(player_t));
    strncpy(pl->username, (char*)sqlite3_column_text(pStmt,0), 16);
    strncpy(pl->city, (char*)sqlite3_column_text(pStmt,1), 255);
    pl->gender = (uint8_t)sqlite3_column_int(pStmt,2);
    pl->c_a_c = (uint32_t)sqlite3_column_int(pStmt,3);
    pl->language = (uint32_t)sqlite3_column_int(pStmt,4);
    pl->birthday = (uint32_t)sqlite3_column_int(pStmt,5);
    pl->intelligence = (uint8_t)sqlite3_column_int(pStmt,6);
    pl->kindness = (uint8_t)sqlite3_column_int(pStmt,7);
    pl->looks = (uint8_t)sqlite3_column_int(pStmt,8);
    pl->c_d = (unsigned int)sqlite3_column_int(pStmt,9);
    pl->microphone = 0;
    pl->location = 0;
    pl->online = 0;
    pl->data = s;
    assert(s->p_l[index] == NULL);
    s->p_l[index] = pl;
    s->c_cli++;
    index++;
  }

  planetring_info("Added %d players", s->c_cli);
  
  sqlite3_finalize(pStmt);
  return 1;
}

int store_raw_ranking(sqlite3* db, player_t *pl, uint8_t rankmode, uint16_t nfo_1, uint16_t nfo_2, uint16_t nfo_3, uint16_t nfo_4, uint16_t nfo_5, uint16_t nfo_6) {
  int rc = 0;
  sqlite3_stmt *pStmt;
  char *zSql = NULL;
    
  switch (pl->location) {
  case SPLASH:
    zSql = "INSERT INTO SPLASH_RAW_RANKING_DATA (ID,USERNAME,RANKMODE,INFO_1,INFO_2,INFO_3,INFO_4,INFO_5,INFO_6) VALUES(NULL,trim(?), ?, ?, ?, ?, ?, ?, ?);";
    break;;
  case BALLBUBBLE:
    zSql = "INSERT INTO BALLBUBBLE_RAW_RANKING_DATA (ID,USERNAME,RANKMODE,INFO_1,INFO_2,INFO_3,INFO_4) VALUES(NULL,trim(?), ?, ?, ?, ?, ?);";
    break;;
  case SOAR:
    zSql = "INSERT INTO SOAR_RAW_RANKING_DATA (ID,USERNAME,RANKMODE,INFO_1,INFO_2,INFO_3,INFO_4) VALUES(NULL,trim(?), ?, ?, ?, ?, ?);";
    break;;
  case DOROBO:
    zSql = "INSERT INTO DOROBO_RAW_RANKING_DATA (ID,USERNAME,RANKMODE,INFO_1,INFO_2,INFO_3,INFO_4) VALUES(NULL,trim(?), ?, ?, ?, ?, ?);";
    break;;
  default:
    return 0;
  }

  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, pl->username, (int)strlen(pl->username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 3, nfo_1);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 4, nfo_2);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 5, nfo_3);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 6, nfo_4);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  if (pl->location == SPLASH) {
    rc = sqlite3_bind_int(pStmt, 7, nfo_5);
    if (rc != SQLITE_OK) {
      sqlite3_finalize(pStmt);
      planetring_error("Bind int failed error: %d", rc);
      return 0;
    }
    rc = sqlite3_bind_int(pStmt, 8, nfo_6);
    if (rc != SQLITE_OK) {
      sqlite3_finalize(pStmt);
      planetring_error("Bind int failed error: %d", rc);
      return 0;
    }    
  }
  
  rc = sqlite3_step(pStmt);
  if (rc != SQLITE_DONE) {
    planetring_error("Insert failed error: %d", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  planetring_info("Added ranking for user: %s successfully", pl->username);
  
  return 1;
}

int create_ranking_soar(sqlite3* db, const char* username, uint8_t rankmode, int tot_matches, int high_score) {
  sqlite3_stmt *pStmt;
  const char *zSql = "INSERT OR REPLACE INTO SOAR_RANKING_DATA (USERNAME,RANKMODE,TOTAL_MATCHES,HIGH_SCORE) VALUES (trim(?),?,?,?);";
  int rc=0;
  
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, username, (int)strlen(username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 3, tot_matches);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 4, high_score);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_step(pStmt);
  if (rc != SQLITE_DONE) {
    planetring_error("Insert failed error: %d", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  return 1;
}

/* Update ranking for SOAR
   Col 1 => Id
   Col 2 => USERNAME
   Col 3 => RANKMODE
   Col 4 => TOTAL_MATCHES
   Col 5 => HIGH_SCORE (BEST_TIME or TOTAL Ballons)
*/
int ranking_soar(sqlite3* db, player_t *pl, uint8_t rankmode, uint16_t nfo_1, uint16_t nfo_2, uint16_t nfo_3, uint16_t nfo_4, rank_t* rp) {
  
  int rc = 0;
  sqlite3_stmt *pStmt;
  int tot_matches=0, high_score=0;
    
  const char *zSql = "SELECT USERNAME,TOTAL_MATCHES,HIGH_SCORE FROM SOAR_RANKING_DATA WHERE USERNAME = ? AND RANKMODE = ?;";
    
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, pl->username, (int)strlen(pl->username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_step(pStmt);
  //Getting no lines
  if (rc == SQLITE_DONE ) {
    planetring_info("[SOAR] - Username %s doesn't have a ranking entry", pl->username);
    tot_matches = 1;
    high_score = nfo_4;
  } else if (rc == SQLITE_ROW ) {
    //Set new values
    tot_matches = sqlite3_column_int(pStmt, 1);
    tot_matches++;
    
    if (rankmode == 0) {
      high_score += nfo_4;
    } else {
      if( nfo_4 < (uint16_t)sqlite3_column_int(pStmt, 2) ) {
	planetring_info("[SOAR] - User %s beaten his best time, new best time %d", pl->username, nfo_4);
	high_score = nfo_4;
      } else {
	planetring_info("[SOAR] - User %s did not beat his best time of %d", pl->username, sqlite3_column_int(pStmt, 2));
	high_score = sqlite3_column_int(pStmt, 2);
      }
    }
  } else {
    planetring_error("Got SQL return %d from sqlite3_step..skip storing", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  //Store data
  create_ranking_soar(db,
		      pl->username,
		      rankmode,
		      tot_matches,
		      high_score);

  /* Populate rp */
  strncpy(rp->username, pl->username, strlen(pl->username));
  if (rankmode == 0) {
    rp->p1col2 = (int)nfo_4; //score
    rp->p1col3 = (int)tot_matches; //#
    rp->p1col4 = (int)high_score; //total
    rp->p1col5 = (int)(high_score/tot_matches)*100; //Ratio
  } else {
    rp->p1col2 = (int)nfo_4; //time
    rp->p1col3 = (int)tot_matches; //#
    rp->p1col4 = (int)high_score; //best time 
  }
  
  return 1;
}
  
int create_ranking_ballbubble(sqlite3* db, const char* username, uint8_t rankmode, int tot_balls, int high_score, int tot_matches, int tot_wins) {
  sqlite3_stmt *pStmt;
  const char *zSql = "INSERT OR REPLACE INTO BALLBUBBLE_RANKING_DATA (USERNAME,RANKMODE,TOTAL_BALLS,HIGH_SCORE,TOTAL_MATCHES,TOTAL_WINS) VALUES (trim(?),?,?,?,?,?);";
  int rc=0;
  
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, username, (int)strlen(username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 3, tot_balls);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 4, high_score);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 5, tot_matches);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 6, tot_wins);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_step(pStmt);
  if (rc != SQLITE_DONE) {
    planetring_error("Insert failed error: %d", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  return 1;
}

/*
  Ball Bubble
  Col 1 => USERNAME
  Col 2 => RANKMODE
  Col 3 => TOTAL_BALLS
  Col 4 => HIGH_SCORE
  Col 5 => TOTAL_MATCHES
  Col 6 => TOTAL_WINS
*/
int ranking_ballbubble(sqlite3* db, player_t *pl, uint8_t rankmode, uint16_t nfo_1, uint16_t nfo_2, uint16_t nfo_3, uint16_t nfo_4, rank_t* rp) {
  
  int rc = 0;
  sqlite3_stmt *pStmt;
  int tot_balls=0,high_score=0,tot_matches=0,tot_wins=0; 

  /*
    Rankmode is set to different values due to if you played 2,3,4 players,
    This is not interesting so we set rankmode to 2, for the select to get
    the correct entry
  */
  rankmode = 2;
  
  const char *zSql = "SELECT USERNAME,TOTAL_BALLS,HIGH_SCORE,TOTAL_MATCHES,TOTAL_WINS FROM BALLBUBBLE_RANKING_DATA WHERE USERNAME = ? AND RANKMODE = ?;";
  
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, pl->username, (int)strlen(pl->username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  
  rc = sqlite3_step(pStmt);
  if (rc == SQLITE_DONE) {
    planetring_info("[BALLBUBBLE] - Username %s doesn't have a ranking entry", pl->username);
    tot_balls = nfo_4;
    tot_matches = 1;
    high_score = nfo_4;
    //0 = Win
    if (nfo_3 == 0)
      tot_wins = 1;
  } else if (rc == SQLITE_ROW ) {
    tot_balls = sqlite3_column_int(pStmt, 1);
    tot_balls += nfo_4;
    
    if( nfo_4 > (uint16_t)sqlite3_column_int(pStmt, 2) ) {
      planetring_info("[BALLBUBBLE] - User has a new high score %d", nfo_4);
      high_score = nfo_4;
    } else {
      planetring_info("[BALLBUBBLE] - User did not beat his high score of %d", sqlite3_column_int(pStmt, 2));
      high_score = sqlite3_column_int(pStmt, 2);
    }

    //Increase total matches by 1
    tot_matches = sqlite3_column_int(pStmt, 3);
    tot_matches++;

    //Get total wins here, otherwise it will be overwritten with 0
    tot_wins = sqlite3_column_int(pStmt, 4);
    
    //0 = Win
    if (nfo_3 == 0)
      tot_wins++;
    
  } else {
    planetring_error("Got SQL return %d from sqlite3_step..skip storing", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  sqlite3_finalize(pStmt);
  
  create_ranking_ballbubble(db,
			    pl->username,
			    rankmode,
			    tot_balls,
			    high_score,
			    tot_matches,
			    tot_wins); 
  /* Populate rp */
  strncpy(rp->username,pl->username, strlen(pl->username));
  rp->p1col2 = (int)nfo_4; //score
  rp->p1col3 = (int)((tot_balls/tot_matches)*100); //Average
  rp->p1col4 = (int)tot_balls; //total
  rp->p1col5 = (int)high_score; //High
  rp->p2col1 = (int)tot_matches; //P2 Nr matches
  rp->p2col2 = (int)tot_wins; //P2 Wins
  rp->p2col3 = (int)((tot_wins/tot_matches)*100); //P2 Ratio
  
  return 1;
}

int create_ranking_splash(sqlite3* db, const char* username, uint8_t rankmode, int tot_wins, int tot_loose, int tot_sunk, int tot_lost, int tot_hit, int tot_shot) {
  sqlite3_stmt *pStmt;
  const char *zSql = "INSERT OR REPLACE INTO SPLASH_RANKING_DATA (USERNAME,RANKMODE,TOTAL_WINS,TOTAL_LOOSE, TOTAL_SUNK, TOTAL_LOST, TOTAL_HIT, TOTAL_SHOT) VALUES (trim(?),?,?,?,?,?,?,?);";
  int rc=0;
  
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, username, (int)strlen(username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 3, tot_wins);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 4, tot_loose);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 5, tot_sunk);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 6, tot_lost);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 7, tot_hit);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
    rc = sqlite3_bind_int(pStmt, 8, tot_shot);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  
  rc = sqlite3_step(pStmt);
  if (rc != SQLITE_DONE) {
    planetring_error("Insert failed error: %d", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  return 1;
}

/*
  SPLASH
  Col 1 => USERNAME
  Col 2 => RANKMODE
  Col 3 => TOTAL_WINS
  Col 4 => TOTAL_LOOSE
  Col 5 => TOTAL_SUNK
  Col 6 => TOTAL_LOST
  Col 7 => TOTAL_HIT
  Col 8 => TOTAL_SHOT
*/
int ranking_splash(sqlite3* db, player_t *pl, uint8_t rankmode, uint16_t nfo_1, uint16_t nfo_2, uint16_t nfo_3, uint16_t nfo_4, uint16_t nfo_5, uint16_t nfo_6, rank_t* rp) {
  
  int rc = 0;
  sqlite3_stmt *pStmt;
  int tot_wins=0, tot_loose=0, tot_sunk=0;
  int tot_lost=0, tot_hit=0, tot_shot=0;

  const char *zSql = "SELECT USERNAME, TOTAL_WINS, TOTAL_LOOSE, TOTAL_SUNK, TOTAL_LOST, TOTAL_HIT, TOTAL_SHOT FROM SPLASH_RANKING_DATA WHERE USERNAME = ? AND RANKMODE = ?;";

  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, pl->username, (int)strlen(pl->username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_step(pStmt);
  if (rc == SQLITE_DONE) {
    planetring_info("Username %s doesn't have a ranking entry", pl->username);
    //1 = Win, 2 = Loose
    if (nfo_2 == 1) {
      tot_wins = 1;
      tot_loose = 0;
      tot_sunk = nfo_3;
      tot_lost = 0;
      tot_hit = nfo_5;
      tot_shot = nfo_6;
    } else if (nfo_2 == 2) {
      tot_loose = 1;
      tot_wins = 0;
      tot_sunk = nfo_3;
      tot_lost = 4;
      tot_hit = nfo_5;
      tot_shot = nfo_6;
    } else {
      planetring_error("Got nfo_2 %d in splash guess this is not the win/loose info", nfo_2);
      sqlite3_finalize(pStmt);
      return 0;
    }
  } else if (rc == SQLITE_ROW ) { 
    //1 = Win, 2 = Loose
    if (nfo_2 == 1) {
      tot_wins = sqlite3_column_int(pStmt, 1);
      tot_wins++;
      tot_loose = sqlite3_column_int(pStmt, 2);
      tot_sunk = sqlite3_column_int(pStmt, 3);
      tot_sunk += nfo_3;
      tot_lost = sqlite3_column_int(pStmt, 4);
      tot_hit = sqlite3_column_int(pStmt, 5);
      tot_hit += nfo_5;
      tot_shot = sqlite3_column_int(pStmt, 6);
      tot_shot += nfo_6;
    } else if (nfo_2 == 2) {
      tot_loose = sqlite3_column_int(pStmt, 2);
      tot_loose++;
      tot_wins = sqlite3_column_int(pStmt, 1);
      tot_sunk = sqlite3_column_int(pStmt, 3);
      tot_sunk += nfo_3;
      tot_lost = sqlite3_column_int(pStmt, 4);
      tot_lost += 4;
      tot_hit = sqlite3_column_int(pStmt, 5);
      tot_hit += nfo_5;
      tot_shot = sqlite3_column_int(pStmt, 6);
      tot_shot += nfo_6;
    } else {
      planetring_error("Got nfo_2 %d in splash guess this is not the win/loose info", nfo_2);
      sqlite3_finalize(pStmt);
      return 0;
    }
  } else {
    planetring_error("Got SQL return %d from sqlite3_step..skip storing", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  sqlite3_finalize(pStmt);

  create_ranking_splash(db,
			pl->username,
			rankmode,
			tot_wins,
			tot_loose,
			tot_sunk,
			tot_lost,
			tot_hit,
			tot_shot);
  /* Populate rp */
  strncpy(rp->username,pl->username, strlen(pl->username));
  rp->p1col1 = (int)(tot_wins+tot_loose); //#
  rp->p1col2 = (int)(tot_wins); //Win
  rp->p1col3 = (int)(tot_loose); //Lost
  rp->p1col4 = (int)((tot_wins/(rp->p1col1))*100); //Ratio

  rp->p2col1 = (int)(tot_sunk); //SUNK
  rp->p2col2 = (int)(tot_lost); //LOST
  rp->p2col3 = (int)(tot_hit); //HIT
  rp->p2col4 = (int)(tot_shot); //SHOT
  
  return 1;
}

int create_ranking_dorobo(sqlite3* db, const char* username, uint8_t rankmode, int tot_matches, int high_score) {
  sqlite3_stmt *pStmt;
  const char *zSql = "INSERT OR REPLACE INTO DOROBO_RANKING_DATA (USERNAME,RANKMODE,TOTAL_MATCHES,HIGH_SCORE) VALUES (trim(?),?,?,?);";
  int rc=0;
  
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, username, (int)strlen(username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 3, tot_matches);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 4, high_score);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  
  rc = sqlite3_step(pStmt);
  if (rc != SQLITE_DONE) {
    planetring_error("Insert failed error: %d", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  return 1;
}

/*
  DOROBO
  Col 1 => ID
  Col 2 => USERNAME
  Col 3 => RANKMODE
  Col 4 => TOTAL_MATCHES
  Col 5 => HIGH_SCORE
  Col 6 => COOP_NAME....
 */
int ranking_dorobo(sqlite3* db, player_t *pl, uint8_t rankmode, uint16_t nfo_1, uint16_t nfo_2, uint16_t nfo_3, uint16_t nfo_4, rank_t* rp) {
  
  int rc = 0;
  sqlite3_stmt *pStmt;
  int high_score = 0, tot_matches = 0;
  
  const char *zSql = "SELECT USERNAME, TOTAL_MATCHES, HIGH_SCORE FROM DOROBO_RANKING_DATA WHERE USERNAME = ? AND RANKMODE = ?;";
      
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    sqlite3_finalize(pStmt);
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_text(pStmt, 1, pl->username, (int)strlen(pl->username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 2, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  
  rc = sqlite3_step(pStmt);
  if (rc == SQLITE_DONE) {
    planetring_info("Username %s doesn't have a ranking entry", pl->username);
    high_score = nfo_2;
    tot_matches = 1;
  } else if (rc == SQLITE_ROW ) {
    tot_matches = sqlite3_column_int(pStmt, 1);
    tot_matches++;
    
    if( nfo_2 > (uint16_t)sqlite3_column_int(pStmt, 2) ) {
      planetring_info("User has beaten his high score, new high score %d", nfo_2);
      high_score = nfo_2;
    } else {
      planetring_info("User did not beat his high score of %d", sqlite3_column_int(pStmt, 2));
      high_score = sqlite3_column_int(pStmt, 2);
    }
  } else {
    planetring_error("Got SQL return %d from sqlite3_step..skip storing", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  sqlite3_finalize(pStmt);

  create_ranking_dorobo(db,
			pl->username,
			rankmode,
			tot_matches,
			high_score);

  /* Populate rp 
     Over all, #, Score, High Score, Opponent, Rank
     #,TOTAL,NAVIGATOR,ATTACKER,BUDDIES,RATIO
  */
  strncpy(rp->username,pl->username, strlen(pl->username));
  rp->p1col1 = (int)(tot_matches); //#
  rp->p1col2 = (int)(nfo_2); //Score
  rp->p1col3 = (int)high_score; //High Score
  rp->p2col1 = (int)300-(nfo_3+nfo_4); //Buddie?
  rp->p2col2 = (int)(((nfo_3+nfo_4)/300)*100); //Ratio
  rp->p2col3 = (int)(nfo_3); //Navigator
  rp->p2col4 = (int)(nfo_4); //Attacker
  
  return 1;
}

int update_splash_total_lost(sqlite3 *db, const char* username, uint8_t rankmode, int lost) {
  int rc = 0;
  sqlite3_stmt *pStmt;
  
  const char* zSql_rank = "UPDATE SPLASH_RANKING_DATA SET TOTAL_LOST = TOTAL_LOST + ? WHERE USERNAME = ? AND RANKMODE = ?;";
    
  rc = sqlite3_prepare_v2(db, zSql_rank, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 1, lost);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
  
  rc = sqlite3_bind_text(pStmt, 2, username, (int)strlen(username), SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind text failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_bind_int(pStmt, 3, (int)rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }

  rc = sqlite3_step(pStmt);
  if (rc != SQLITE_DONE) {
    planetring_error("Update failed error: %d", rc);
    sqlite3_finalize(pStmt);
    return 0;
  }
  
  sqlite3_finalize(pStmt);
  return 1;
}

int get_rank_for_player(sqlite3 *db,player_t *pl, const char* username, uint8_t rankmode) {
  int rc = 0, rank=0;
  sqlite3_stmt *pStmt;
  const char* zSql_rank;

  switch (pl->location) {
  case SPLASH:
    zSql_rank = "SELECT USERNAME FROM SPLASH_RANKING_DATA WHERE RANKMODE = ? ORDER BY TOTAL_WINS DESC;";
    break;;
  case BALLBUBBLE:
    zSql_rank = "SELECT USERNAME FROM BALLBUBBLE_RANKING_DATA WHERE RANKMODE = ? ORDER BY TOTAL_WINS DESC;";
    break;;
  case SOAR:
    if (rankmode == 0)
      zSql_rank = "SELECT USERNAME FROM SOAR_RANKING_DATA WHERE RANKMODE = ? ORDER BY HIGH_SCORE DESC;";
    else
      zSql_rank = "SELECT USERNAME FROM SOAR_RANKING_DATA WHERE RANKMODE = ? ORDER BY HIGH_SCORE ASC;";
    break;;
  case DOROBO:
    zSql_rank = "SELECT USERNAME FROM DOROBO_RANKING_DATA WHERE RANKMODE = ? ORDER BY HIGH_SCORE DESC;";
    break;;
  default:
    return 0;
  }
  
  rc = sqlite3_prepare_v2(db, zSql_rank, -1, &pStmt, 0);
  if( rc != SQLITE_OK ){
    planetring_error( "Prepare SQL error: %d", rc);
    return 0;
  }
  rc = sqlite3_bind_int(pStmt, 1, rankmode);
  if (rc != SQLITE_OK) {
    sqlite3_finalize(pStmt);
    planetring_error("Bind int failed error: %d", rc);
    return 0;
  }
 
  while((rc = sqlite3_step(pStmt)) != SQLITE_DONE) {
    rank++;
    if(strcmp(username, (char*)sqlite3_column_text(pStmt,0)) == 0) {
      planetring_info("Found player %s on line %d", username, rank);
      sqlite3_finalize(pStmt);
      return rank;
    }
  }
  
  sqlite3_finalize(pStmt);
  return 0;
}

int update_rank(server_data_t *s,
		player_t *pl,
		uint8_t table_id,
		uint8_t rankmode,
		uint16_t nfo_1,
		uint16_t nfo_2,
		uint16_t nfo_3,
		uint16_t nfo_4,
		uint16_t nfo_5,
		uint16_t nfo_6) {
  int ret=0,i=0,j=0;
  sqlite3 *db = s->db;
  rank_t * rp_item = NULL;
  rank_t rp_tmp = {0};
  
  attraction_t **a = get_attraction_from_location(s, pl->location);

  if (table_id > s->m_tables) {
    planetring_error("Got a strange table id, reset to what is stored in player struct");
    table_id = pl->tbl_id;
  }
  //Check again
  if (a == NULL || (table_id > s->m_tables))
    return 0;

  //Get a free ranking struct
  for(i=0;i<(a[table_id]->max_pl);i++) {
    if (a[table_id]->rank_pool[i]->in_use == 0) {
      a[table_id]->rank_pool[i]->in_use = 1;
      rp_item = a[table_id]->rank_pool[i];
      break;
    }
  }

  if (rp_item == NULL) {
    planetring_info("No rp items left? How strange...");
    rp_tmp.in_use = 1;
    rp_item = &rp_tmp;
  }

  ret = store_raw_ranking(db,
			  pl,
			  rankmode,
			  nfo_1,
			  nfo_2,
			  nfo_3,
			  nfo_4,
			  nfo_5,
			  nfo_6);
  if (ret != 1)
    planetring_error("Failed to store raw ranking user: %s ranking", pl->username);
  
  switch (pl->location) {
  case SPLASH:
    
    ret = ranking_splash(db,
			 pl,
			 rankmode,
			 nfo_1,
			 nfo_2,
			 nfo_3,
			 nfo_4,
			 nfo_5,
			 nfo_6,
			 rp_item);
    if (ret != 1)
      return 0;

    //The player that lost needs to update the winners (lost column)
    if (nfo_2 == 2) {
      for(j=0;j<(a[table_id]->max_pl);j++) {
	if ((a[table_id]->rank_pool[j]->in_use == 1) &&
	    (rp_item != a[table_id]->rank_pool[j])) {
	  
	  planetring_info("Found opponent %s ranking item, increase total lost with %d",
			  a[table_id]->rank_pool[j]->username,
			  nfo_3);

	  if (nfo_3 == 0) {
	    planetring_info("You sank 0 of the opponents ships, nothing to do here");
	    return 1;
	  }
	  a[table_id]->rank_pool[j]->p2col2 += nfo_3;

	  //Here we need mutex
          sqlite3_mutex_enter(sqlite3_db_mutex(db));
	  
	  if ( (update_splash_total_lost(db, a[table_id]->rank_pool[j]->username, rankmode, nfo_3) == 1) )
	    planetring_info("Updated total lost for opponent");
	  else
	    planetring_error("Failed to add lost value after a game of SPLASH");

	  sqlite3_mutex_leave(sqlite3_db_mutex(db));
	  return 1;
	}
      }
    }
    break;;
  case BALLBUBBLE:
    ret = ranking_ballbubble(db,
			     pl,
			     rankmode,
			     nfo_1,
			     nfo_2,
			     nfo_3,
			     nfo_4,
			     rp_item);
    if (ret != 1)
      return 0;
    
    break;;
  case SOAR:
    ret = ranking_soar(db,
		       pl,
		       rankmode,
		       nfo_1,
		       nfo_2,
		       nfo_3,
		       nfo_4,
		       rp_item);
    if (ret != 1)
      return 0;
    
    break;;
  case DOROBO:
    ret = ranking_dorobo(db,
			 pl,
			 rankmode,
			 nfo_1,
			 nfo_2,
			 nfo_3,
			 nfo_4,
			 rp_item);
    if (ret != 1)
      return 0;
    break;;
  default:
    return 0;
  }
  
  return 1;
}
