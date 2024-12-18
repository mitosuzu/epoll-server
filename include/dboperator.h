#ifndef _DBOPERATOR_H
#define _DBOPERATOR_H

#include <sqlite3.h>

int openDatabase(char *filename, sqlite3 **pDb); 
int execSql(sqlite3 *p_db, char *s_sql, char **ps_errmsg, int is_search, void **search_rst);

#endif