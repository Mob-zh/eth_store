#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <getopt.h>
#include "rfid_db.h"

void print_help(void)
{
    printf("Usage ./create_shelf: <shelf_id> \n");
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        print_help();
        exit(0);
    }
    int shelf_id = atoi(argv[1]);

    //链接数据库
    sqlite3 *db;

    sqlite3_open(STORE_DB, &db);
    //在数据库中创建新的货架以及货架对应的货仓
    if (Del_Shelf(db, shelf_id) < 0)
    {
        printf("删除货架失败\n");
    }

    exit(0);
}
