#ifndef _INFOSTRUCT_H
#define _INFOSTRUCT_H

typedef struct user
{
    int account;
    char user_name[20];
    char password[15];
    float remain_money;
    int is_manager;
} User, Manager;

typedef struct movie
{
    char name[20];
    float price;

} Movie;

typedef struct cinema
{
    char name[20];
} Cinema;

#endif