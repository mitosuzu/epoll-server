#ifndef _DBOPERATOR_H
#define _DBOPERATOR_H

#include <sqlite3.h>

int openDatabase(char *filename, sqlite3 **pDb); 
int execSql(sqlite3 *p_db, char *s_sql, int is_search, void **search_rst);
int getAllDataFromTable(sqlite3 *p_db, char *s_sql, char ***result, int *rows, int *columns);

#endif