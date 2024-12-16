#include "serverprocess.h"
#include "ThreadPool.h"
#include "infostruct.h"

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>

char **splitString(char *str, char flag)
{
    int max_count = 3;
    int sub_count = 0;
    char **rst = (char **)calloc(max_count, sizeof(char *));
    rst[0] = str;
    while (*str)
    {
        if (sub_count >= max_count - 1)
            rst = (char **)realloc(rst, sizeof(char *) * 2 * max_count);

        if (*str == flag)
        {
            *str = '\0';
            rst[++sub_count] = str + 1;
        }
        str++;
    }
    return rst;
}

Error sendFile(int sock_fd, FILE *fs)
{
    char file_buffer[65535];
    int ret;

    while (1)
    {
        bzero(file_buffer, sizeof(file_buffer));
        ret = fread(file_buffer, 1, sizeof(file_buffer), fs);
        if (ret > 0)
        {
            send(sock_fd, file_buffer, ret, 0);
        }
        if (ret == 0)
        {
            fclose(fs);
            return SUCCESS;
        }
    }
}

Error loginCheck(User user_account)
{
    // ListNode *temp;
    // int ret = searchHashNodeByElementKey(user_hash, &temp, atoi(user_account.account));
    // if (ret < 0)
    //     return FAIL_LOGIN_NO_ACCOUNT;
    // if (strcmp(temp->data.password, user_account.password))
    //     return FAIL_LOGIN_NO_PSW;
    return SUCCESS;
}

Error registerUser(User user_account)
{
    // ListNode *temp;
    // int ret = searchHashNodeByElementKey(user_hash, &temp, atoi(user_account.account));
    // if (ret >= 0)
    //     return FAIL_REGISTER_EXIST;

    // addHashElement(user_hash, user_account);

    return SUCCESS;
}

// void processWrite(int epoll_fd, int client_sock_fd, char *recv_buffer)
// {
//     int temp = 0;
//     int bytes_have_send = 0;
//     int bytes_to_send = m_write_idx;
//     if (bytes_to_send == 0)
//     {
//         modfd(epoll_fd, client_sock_fd, EPOLLIN);
//         init();
//         return;
//     }

//     while (1)
//     {
//         temp = writev(client_sock_fd, m_iv, m_iv_count);
//         if (temp <= -1)
//         {
//             if (errno == EAGAIN)
//             {
//                 modfd(m_epollfd, m_sockfd, EPOLLOUT);
//                 return true;
//             }
//             unmap();
//             return false;
//         }

//         bytes_to_send -= temp;
//         bytes_have_send += temp;
//         if (bytes_to_send <= bytes_have_send)
//         {
//             unmap();
//             if (m_linger)
//             {
//                 init();
//                 modfd(m_epollfd, m_sockfd, EPOLLIN);
//                 return true;
//             }
//             else
//             {
//                 modfd(m_epollfd, m_sockfd, EPOLLIN);
//                 return false;
//             }
//         }
//     }
// }

void processRead(int client_sock_fd, char *recv_buffer)
{
    char **strs = splitString(recv_buffer, '#');
    if (!strncmp(recv_buffer, "login", 5))
    {
        char msg_buffer[64];
        User user_account;
        strcpy(user_account.user_name, strs[1]);
        strcpy(user_account.password, strs[2]);
        bzero(msg_buffer, sizeof(msg_buffer));
        printf("发起登录 用户名:%s 密码:%s\n", strs[1], strs[2]);

        int ret = loginCheck(user_account);
        if (ret == FAIL_LOGIN_NO_ACCOUNT)
        {
            printf("用户名不存在\n");
            sprintf(msg_buffer, "message#用户名不存在");
            send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
        }
        if (ret == FAIL_LOGIN_NO_PSW)
        {
            printf("密码错误\n");
            sprintf(msg_buffer, "message#密码错误");
            send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
        }
        if (ret == SUCCESS)
        {
            printf("登陆成功\n");
            sprintf(msg_buffer, "message#success");
            send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
        }
    }
    else if (!strncmp(recv_buffer, "register", 8))
    {
        char msg_buffer[64];
        User user_account;
        strcpy(user_account.user_name, strs[1]);
        strcpy(user_account.password, strs[2]);

        bzero(msg_buffer, sizeof(msg_buffer));
        printf("发起注册 用户名:%s 密码:%s\n", strs[1], strs[2]);

        int ret = registerUser(user_account);
        if (ret == FAIL_REGISTER_EXIST)
        {
            printf("用户名已存在\n");
            sprintf(msg_buffer, "message#用户名已存在");
            send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
        }
        if (ret == SUCCESS)
        {
            printf("注册成功\n");
            sprintf(msg_buffer, "message#success");
            send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
        }
    }
    else if (!strncmp(recv_buffer, "download", 8))
    {
        struct stat sta;
        char path[80];
        char msg_buffer[64];
        sprintf(path, "/home/liulin/24101/NetWork/Day1/%s", strs[1]);

        FILE *fs = fopen(path, "r");
        if (fs == NULL)
        {
            sprintf(msg_buffer, "message#文件不存在");
            send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
            return;
        }

        stat(path, &sta);
        sprintf(msg_buffer, "datasize#%ld", sta.st_size);
        send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
        sleep(1);
        sendFile(client_sock_fd, fs);
    }
    else
    {
        printf("发来数据:%s\n", recv_buffer);
    }
}