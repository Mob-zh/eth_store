#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "rfid_db.h"

#define STORE_DB "/eth_store_db/store.db"

static int callback(void *data, int argc, char **argv, char **azColName)
{
    int i;

    fprintf(stderr, "%s: ", (const char *)data);
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main(int argc, char *argv[])
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char *data = "Callback function called";
    /* Open database */
    rc = sqlite3_open(STORE_DB, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    }
    else
    {
        fprintf(stdout, "Opened database successfully\n");
    }

    /* Create SQL statement */
    sql = "SELECT * from COMPANY WHERE NAME=\"Teddy\"";
    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void *)data, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Operation successfully\n");
    }
    sqlite3_close(db);
    return 0;
}
