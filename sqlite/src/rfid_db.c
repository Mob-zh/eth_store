#include "rfid_db.h"
//增
int Add_Elem(sqlite3 *db, struct element *elem)
{
    char *zErrMsg = 0;
    int rc;
    char sql[256];

    sprintf(sql, "INSERT INTO element (elem_id,type,value,unit,temp_min,temp_max,wet_min,wet_max,shelf_id,slot_id)"
                 "VALUES(%d,'%s',%d,'%s',%d,%d,%d,%d,%d,%d);",
            elem->id, elem->type, elem->value, elem->unit, elem->temp_min, elem->temp_max, elem->wet_min, elem->wet_max, elem->shelf_id, elem->slot_id);

    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    else
    {
        fprintf(stdout, "elem insert successfully\n");
    }

    sprintf(sql, "UPDATE slot  SET elem_type='%s' , unit='%s',status=1 "
                 "WHERE shelf_id=%d AND slot_id=%d;",
            elem->type, elem->unit, elem->shelf_id, elem->slot_id);

    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    else
    {
        fprintf(stdout, "slot insert successfully\n");
    }

    return 0;
}
int Add_Shelf(sqlite3 *db, int shelf_id, int length, int width)
{
    char *zErrMsg = 0;
    int rc;
    char sql[256];
    int capacity = length * width;

    sprintf(sql, "INSERT INTO shelf (shelf_id,capacity)"
                 "VALUES(%d,%d);",
            shelf_id, length * width);

    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    else
    {
        fprintf(stdout, "shelf insert successfully\n");
    }

    for (int i = 1; i <= capacity; i++)
    {
        sprintf(sql, "INSERT INTO slot (slot_id,shelf_id)"
                     "VALUES(%d,%d);",
                i, shelf_id);

        rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return -1;
        }
    }

    fprintf(stdout, "slot create successfully\n");

    return 0;
}

//
