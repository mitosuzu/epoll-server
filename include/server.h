#ifndef _SERVER_H
#define _SERVER_H



typedef enum
{
    FAIL_DATABASE = -7,
    FAIL_REGISTER_EXIST,
    FAIL_LOGIN_NO_ACCOUNT,
    FAIL_LOGIN_NO_PSW,
    FAIL_SEND,
    FAIL_RECV,
    FAIL_FILE_OPEN,
    SUCCESS
} Error;

void initReactorThread(int listen_sock_fd, int thread_count);
int getListenSocket(const char *ip, int port);

#endif