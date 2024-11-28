#include "rfid_db.h"
//////////////////////////////增/////////////////////////////////

//同步在物品表和格子表插入数据
/**
 *货物入库
 *  @param[in] db = 数据库指针
 *  @param[in]  elem = 要加入的货品结构
 */
int Add_Elem(sqlite3 *db, struct element *elem)
{
    char *zErrMsg = 0;
    int rc;
    char sql[SQL_LENTH];

    sprintf(sql, "INSERT INTO element (elem_id,type,value,unit,temp_min,temp_max,wet_min,wet_max,shelf_id,slot_id)"
                 "VALUES('%x','%s',%d,'%s',%d,%d,%d,%d,%d,%d);",
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

    sprintf(sql, "UPDATE slot  SET elem_type='%s' ,status=1,have_loaded=have_loaded+1 "
                 "WHERE shelf_id=%d AND slot_id=%d AND have_loaded < capacity;",
            elem->type, elem->shelf_id, elem->slot_id);

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

/**
 *新增货架
 *  @param[in] db = 数据库指针
 *  @param[in]  shelf_id = 新增的货架id
 *  @param[in]  height = 新增货架的层数
 *  @param[in]  width = 新增货架一列的货仓数
 *  @param[in]  store_type = 新增货架的货品类型
 * 
 */
int Add_Shelf(sqlite3 *db, int shelf_id, int height, int width, char *store_type)
{
    char *zErrMsg = 0;
    int rc;
    char sql[SQL_LENTH];
    int capacity = height * width;

    sprintf(sql, "INSERT INTO shelf (shelf_id,capacity,store_type) "
                 "VALUES(%d,%d,'%s');",
            shelf_id, height * width, store_type);

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

    //创建slot
    for (int i = 1; i <= capacity; i++)
    {
        //如果是混合货架
        if (!strcmp(store_type, STORE_TYPE_MIX))
        {
            sprintf(sql, "INSERT INTO slot (slot_id,shelf_id) "
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
        else
        {
            sprintf(sql, "INSERT INTO slot (slot_id,shelf_id,elem_type) "
                         "VALUES(%d,%d,'%s');",
                    i, shelf_id, store_type);

            rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
            if (rc != SQLITE_OK)
            {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                return -1;
            }
        }
    }

    fprintf(stdout, "slot create successfully\n");

    return 0;
}

//////////////////////////////删/////////////////////////////////

/**
 *货物出库
 *  @param[in] db = 数据库指针
 *  @param[in]  elem = 要出库的货品
 */
int Fetch_Elem(sqlite3 *db, struct element elem)
{
    char *zErrMsg = 0;
    int rc;
    char sql[SQL_LENTH];

    // 删除关联的槽位数据
    sprintf(sql, "DELETE FROM slot WHERE shelf_id = %d AND slot_id = %d", elem.shelf_id, elem.slot_id);

    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error when deleting slots: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}

/**
 *删除货架
 *  @param[in] db = 数据库指针
 *  @param[in]  shelf_id = 要删除的货架id
 */
int Del_Shelf(sqlite3 *db, int shelf_id)
{
    char *zErrMsg = 0;
    int rc;
    char sql[SQL_LENTH];
    char **dbResult = NULL;
    int nRow, nColumn;
    // 检查槽位是否有货物
    sprintf(sql, "SELECT COUNT(*) FROM slot WHERE shelf_id = %d AND have_loaded > 0;", shelf_id);

    rc = sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error when checking slots: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    int loaded_count = atoi(dbResult[1]); // dbResult[1] 是查询结果的第一条记录

    if (loaded_count > 0)
    {
        fprintf(stderr, "Cannot delete shelf %d: Slots contain loaded items.\n", shelf_id);
        return -1;
    }
    sqlite3_free_table(dbResult);

    // 删除关联的槽位数据
    sprintf(sql, "DELETE FROM slot WHERE shelf_id = %d;", shelf_id);

    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error when deleting slots: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    // 删除货架数据
    sprintf(sql, "DELETE FROM shelf WHERE shelf_id = %d;", shelf_id);

    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error when deleting shelf: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    fprintf(stdout, "Shelf %d and its slots deleted successfully.\n", shelf_id);
    return 0;
}

//////////////////////////////改/////////////////////////////////
//修改表中选定的内容
// 判断参数类型

static int detect_type(const char *value)
{
    char *endptr;
    strtol(value, &endptr, 10); // 尝试将字符串解析为整数
    if (*endptr == '\0')
    {
        return SQLITE_INTEGER; // 如果解析完全成功，则为整数
    }
    return SQLITE_TEXT; // 否则为文本
}

// 动态绑定参数值
static int bind_value(sqlite3_stmt *stmt, int index, const char *value)
{
    int type = detect_type(value);
    if (type == SQLITE_INTEGER)
    {
        int int_val = atoi(value); // 转为整数
        return sqlite3_bind_int(stmt, index, int_val);
    }
    else
    {
        return sqlite3_bind_text(stmt, index, value, -1, SQLITE_STATIC);
    }
    return 0;
}

//选择表名，列名，筛选条件，新的值
int update_table(sqlite3 *db, const char *tablename, const char *colume_name, const char *condition, const char *new_value)
{
    char sql[SQL_LENTH];
    snprintf(sql, sizeof(sql), "UPDATE %s SET %s = ? WHERE %s;", tablename, colume_name, condition);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // 动态绑定参数值
    rc = bind_value(stmt, 1, new_value);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind value: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc;
    }

    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return rc;
}

//为elem分配位置
int alloc_position(sqlite3 *db, struct element *elem)
{
    char *zErrMsg = 0;
    char sql[SQL_LENTH];
    char **dbResult = NULL;
    int nRow, nColumn;
    int rc;
    int index;

    // 检查元件类型是否合法
    if (!(strcmp(elem->type, "电阻") == 0 || strcmp(elem->type, "电容") == 0 ||
          strcmp(elem->type, "电感") == 0 || strcmp(elem->type, "芯片") == 0))
    {
        printf("商品类型不合规\n");
        return -1;
    }

    // 动态构建 SQL 查询
    snprintf(sql, sizeof(sql), "SELECT shelf.shelf_id, slot.slot_id, (slot.capacity - slot.have_loaded) AS available_load "
                               "FROM shelf "
                               "JOIN slot ON shelf.shelf_id = slot.shelf_id "
                               "WHERE (shelf.store_type = '%s' OR shelf.store_type = '%s') "
                               "AND slot.have_loaded < slot.capacity "
                               "AND slot.unit = '%s' "
                               "ORDER BY available_load DESC "
                               "LIMIT 1;",
             elem->type, STORE_TYPE_MIX, elem->unit);

    // 执行 SQL 查询
    rc = sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    // 判断是否找到合适的仓位
    if (nRow > 0)
    {
        index = nColumn;                          // 数据部分从 nColumn 开始
        elem->shelf_id = atoi(dbResult[index++]); // 转换为整数
        elem->slot_id = atoi(dbResult[index++]);
        int available_load = atoi(dbResult[index]) - 1;

        printf("%s成功分配到%d号货架，%d号仓，该仓剩余容量为%d\n",
               elem->type, elem->shelf_id, elem->slot_id, available_load);
    }
    else
    {
        printf("未分配到合适的空位\n");
    }

    // 释放查询结果
    sqlite3_free_table(dbResult);

    return 0;
}

//查
int print_table(sqlite3 *db, const char *tablename)
{
    if (db == NULL || tablename == NULL)
    {
        fprintf(stderr, "Error: Null database or table name.\n");
        return -1;
    }

    char *zErrMsg = 0;
    char sql[SQL_LENTH];
    int rc;
    char **dbResult; // 查询结果
    int nRow, nColumn;
    int i, j;
    int index;

    sprintf(sql, "SELECT * FROM %s", tablename);

    // 查询表数据
    rc = sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg); // 释放错误信息
        return -1;
    }

    // 打印记录总数
    printf("查到 %d 条记录\n", nRow);

    // 打印字段名称
    printf("字段名称: ");
    for (j = 0; j < nColumn; j++)
    {
        printf("%s\t", dbResult[j]); // 打印字段名
    }
    printf("\n");

    // 打印记录内容
    index = nColumn;
    for (i = 0; i < nRow; i++)
    {
        printf("第 %d 条记录\n", i + 1);
        for (j = 0; j < nColumn; j++)
        {
            printf("字段名: %s > 字段值: %s\n", dbResult[j], dbResult[index]);
            ++index; // 逐步遍历字段值
        }
        printf("\n");
    }

    // 释放查询结果
    sqlite3_free_table(dbResult);

    return 0;
}

int get_table(sqlite3 *db, const char *tablename, char ***dbResult, int *nRow, int *nColumn)
{
    char *zErrMsg = 0;
    char sql[SQL_LENTH];
    int rc;
    sprintf(sql, "SELECT * FROM %s", tablename);

    rc = sqlite3_get_table(db, sql, dbResult, nRow, nColumn, &zErrMsg);
    if (SQLITE_OK != rc)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}