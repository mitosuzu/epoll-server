#ifndef _SERVERPROCESS_H
#define _SERVERPROCESS_H

typedef enum
{
    FAIL_REGISTER_EXIST = -6,
    FAIL_LOGIN_NO_ACCOUNT,
    FAIL_LOGIN_NO_PSW,
    FAIL_SEND,
    FAIL_RECV,
    FAIL_FILE_OPEN,
    SUCCESS
} Error;

void processRead(int client_sock_fd,char *recv_buffer);

#endif