#ifndef _RFID_DB_H_
#define _RFID_DB_H_

#define STORE_DB "/eth_store_db/store.db"

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <termios.h>
#include <string.h>
#include "rfid_M4255.h"
#include "rfid_db.h"

#define SQL_LENTH 256 * 2

int Add_Elem(sqlite3 *db, struct element *elem);
int Add_Shelf(sqlite3 *db, int shelf_id, int length, int width, char *store_type);
int Fetch_Elem(sqlite3 *db, struct element elem);
int Del_Shelf(sqlite3 *db, int shelf_id);

int alloc_position(sqlite3 *db, struct element *elem);
int print_table(sqlite3 *db, const char *tablename);
int get_table(sqlite3 *db, const char *tablename, char ***dbResult, int *nRow, int *nColumn);
int update_table(sqlite3 *db, const char *tablename, const char *colume_name, const char *condition, const char *value);
#endif // !_RFID_DB_H_
