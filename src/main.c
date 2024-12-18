#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#include <fcntl.h>

#include "server.h"
#include "ThreadPool.h"
#include "globalinfo.h"
//#include "dboperator.h"
int main(int argc, char const *argv[])
{
    // char filename[20];
    // printf("需要打开的数据是:");
    // strcpy(filename, "database.db");
    // sqlite3 *db = NULL;
    // if (openDatabase(filename, &db) < 0)
    // {
    //     return -1;
    // }
    // printf("打开数据库 成功! db = %p\n", db);

    // // 定义sql语句
    // char *errmsg = NULL;
    // char sql[256] = {0};
    // char table_name[50] = {0};
    // char logonName[20] = {"10000"};
    // char logonPwd[20] = {"L111909"};

    // // 登陆逻辑：按照用户名检索数据库中对应的密码，将其返回
    // bzero(sql, 256);
    // strcpy(table_name, "UserInfo");
    // char RetPwd[20] = {0}; // strlen = 0
    // // sprintf(sql, "select Password from %s where Account = '%s';", table_name, logonName);
    // sprintf(sql, "select * from UserInfo where Name = 'LiuL3in';");

    // UserInfo *user = NULL;
    // if (execSql(db, sql, &errmsg, 1, (void**)&user) < 0)
    // {
    //     printf("执行出错:%s\n", errmsg);
    //     sqlite3_close(db);
    //     return -1;
    // }
    // printf("%s\n", user->password);
    // printf("%s\n", user->user_name);
    // printf("%d\n", user->account);
    


    // if (argc != 3)
    //     return -1;
    // const char *ip = argv[1];
    // int port = atoi(argv[2]);

    int listen_sock_fd = getListenSocket("127.0.0.1", 8080);
    // int listen_sock_fd = getListenSocket(ip, port);
    if (listen_sock_fd < 0)
        return -1;
    printf("Listening...\n");
    initReactorThread(listen_sock_fd, 4);
    close(listen_sock_fd);

    return 0;
}