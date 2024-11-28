#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include "rfid_db.h"

int main(int argc, char *argv[])
{
    sqlite3 *db;
    int rc;

    /* Open database */
    rc = sqlite3_open("/eth_store_db/store.db", &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }
    char table[30] = {0};
    strcpy(table, "element");
    if (argc > 2)
    {
        strcpy(table, argv[1]);
    }

    print_table(db, argv[1]);
    sqlite3_close(db);
    return 0;
}
