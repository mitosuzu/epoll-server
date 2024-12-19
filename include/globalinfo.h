#ifndef _GLOBALINFO_H
#define _GLOBALINFO_H

#include <pthread.h> 
#include <sqlite3.h>

typedef struct user
{
    int account;
    char user_name[32];
    char password[32];
    float remain_money;
    int is_manager;
} UserInfo, ManagerInfo;

typedef struct movie
{
    int id;
    char cn_name[64];
    char en_name[64];
    char country[32];
    char duration[32];
    char director_name[64];
    float price;
} MovieInfo;

typedef struct cinema
{
    char name[20];
} CinemaRoomInfo;

extern char send_msg[1024];
extern int list_client[1024];//应该用链表存储
extern int client_count;
extern pthread_mutex_t database_lock;
extern pthread_mutex_t global_lock;
extern pthread_cond_t global_cond;
extern sqlite3 *p_db;

#endif