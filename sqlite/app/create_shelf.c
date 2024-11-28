#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <getopt.h>
#include "rfid_db.h"

void print_help(void)
{
    printf("Usage ./create_shelf: <shelf_id> <height> <width> <store_type>\n");
}

int main(int argc, char **argv)
{

    if (argc < 5)
    {
        print_help();
        exit(0);
    }
    int shelf_id = atoi(argv[1]);
    int height = atoi(argv[2]);
    int width = atoi(argv[3]);
    char store_type[MAX_ELEM_TYPE_SIZE];
    strcpy(store_type, argv[4]);
    //链接数据库
    sqlite3 *db;

    sqlite3_open(STORE_DB, &db);
    //在数据库中创建新的货架以及货架对应的货仓
    if (Add_Shelf(db, shelf_id, height, width, store_type) < 0)
    {
        printf("创建货架失败\n");
    }

    exit(0);
}
