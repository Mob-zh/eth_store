#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "rfid_db.h"

int main(int argc, char **argv)
{

    sqlite3 *db;
    int result;
    // char **dbResult = NULL; //是 char ** 类型，两个*号
    // int nRow, nColumn;
    // int i, j;
    // int index;
    int rc;

    if (argc < 5)
    {
        printf("Usage %s: <tablename> <colume_name> <condition> <new_colume_val>\n", argv[0]);
        exit(0);
    }
    char *tablename = argv[1];
    char *columename = argv[2];
    char *condition = argv[3];
    char *new_value = argv[4];

    result = sqlite3_open(STORE_DB, &db);
    if (result != SQLITE_OK)
    {
        //数据库打开失败
        fprintf(stderr, "open db failed.");
        return -1;
    }

    //数据库操作代码

    rc = update_table(db, tablename, columename, condition, new_value);
    if (rc < 0)
    {
        fprintf(stderr, "sql err:%s\n", sqlite3_errmsg(db));
    }
    //到这里，不论数据库查询是否成功，都释放 char** 查询结果，使用 sqlite 提供的功能来释放

    //关闭数据库

    sqlite3_close(db);

    return 0;
}
