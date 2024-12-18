#include "globalinfo.h"

char send_msg[1024] = {0};//操作需要线程同步
sqlite3 *p_db = NULL;
pthread_mutex_t database_lock;
pthread_mutex_t global_lock;
pthread_cond_t global_cond;
