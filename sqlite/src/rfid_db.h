#ifndef _RFID_DB_H_
#define _RFID_DB_H_

#define STORE_DB "/eth_store_db/store.db"

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <termios.h>
#include "rfid_M4255.h"

int Add_Elem(sqlite3 *db, struct element *elem);

#endif // !_RFID_DB_H_
