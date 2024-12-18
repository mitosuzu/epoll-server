#include "dboperator.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "globalinfo.h"

int searchUserCallback(void *para, int f_num, char **f_value, char **f_name)
{
    UserInfo *user = (UserInfo *)calloc(1, sizeof(UserInfo));
    user->account = atoi(f_value[0]);
    strcpy(user->user_name, f_value[1]);
    strcpy(user->password, f_value[2]);
    user->remain_money = atof(f_value[3]);
    user->is_manager = atoi(f_value[4]);
    UserInfo **temp = (UserInfo**)para;
    *temp = user;
    return 0;
}

int openDatabase(char *filename, sqlite3 **pp_db)
{
    int ret = sqlite3_open(filename, pp_db);
    if (SQLITE_OK != ret)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(*pp_db));
        return -1;
    }
    return 0;
}

int getAllDataFromTable(sqlite3 *p_db, char *s_sql, char *errmsg, char **result)
{
    int rows, columns;
    int rc = sqlite3_get_table(p_db, "SELECT * from UserInfo WHERE Name = 'LiuLin';", &result, &rows, &columns, &errmsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
    else
    {
        /* 打印结果 */
        for (int i = 0; i <= rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                printf("%s ", result[i * columns + j]);
            }
            printf("\n");
        }
        /* 释放结果数组 */
        sqlite3_free_table(result);
    }
}

int execSql(sqlite3 *p_db, char *s_sql, char **ps_errmsg, int is_search, void **search_rst)
{
    int ret;
    if (!is_search)
    {
        ret = sqlite3_exec(p_db, s_sql, NULL, NULL, ps_errmsg);
        if (ret != SQLITE_OK)
            return -1;
    }
    else
    {
        ret = sqlite3_exec(p_db, s_sql, &searchUserCallback, (void*)search_rst, ps_errmsg);
        if (ret < 0)
            return -1;
    }
    return 0;
}
