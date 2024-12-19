#include "server.h"
#include "ThreadPool.h"
#include "dboperator.h"
#include "globalinfo.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define MAXSIZE 1024

char **splitString(char *str, char flag);
Error loginCheck(UserInfo *user_info);
void addFdEventInEpoll(int epoll_fd, int fd, int is_one_shot);
void modifyFdEventInEpoll(int epoll_fd, int fd, int event_id);
void threadTask(void *args);
void processRead(int epoll_fd, int client_sock_fd, char *recv_buffer);
Error registerUser(UserInfo user_account);
void initReactorThread(int listen_sock_fd, int thread_count)
{
    ThreadPool *pool = threadPoolCreate(thread_count);
    int epoll_fd = epoll_create(5); // 形参为了兼容旧版本，当前版本参数无实际意义
    addFdEventInEpoll(epoll_fd, listen_sock_fd, 0);

    struct epoll_event listen_event;          // listen套接字的事件
    struct epoll_event ready_events[MAXSIZE]; // 就绪事件组
    const int MAX_ACCEPTS_PER_CALL = 200;     // 每次处理的最大连接数目
    int current_accept;                       // 本次已处理数量

    openDatabase("database.db", &p_db);
    pthread_mutex_init(&global_lock, NULL);
    pthread_mutex_init(&database_lock, NULL);
    pthread_cond_init(&global_cond, NULL);

    while (1)
    {
        int ready_count = epoll_wait(epoll_fd, ready_events, MAXSIZE, -1); // 内核将就绪的事件们拷贝到事件数组中

        for (int i = 0; i < ready_count; i++)
        {
            int current_fd = ready_events[i].data.fd;
            if (current_fd == listen_sock_fd)
            {
                current_accept = 0;
                while (current_accept <= MAX_ACCEPTS_PER_CALL)
                {
                    struct sockaddr_in client_addr;
                    struct epoll_event event_recv;
                    memset(&client_addr, 0, sizeof(client_addr));
                    memset(&event_recv, 0, sizeof(event_recv));

                    int client_addr_len = sizeof(client_addr);
                    int new_sock_fd = accept(listen_sock_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
                    if (new_sock_fd == -1)
                    {
                        if (errno == EWOULDBLOCK)
                            break;
                    }
                    else
                    {
                        ++current_accept;
                        addFdEventInEpoll(epoll_fd, new_sock_fd, 1);
                        printf("accept new client:%s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    }
                }
            }
            else if (ready_events->events & EPOLLIN)
            {
                // 检查events[i].events中是否设置了EPOLLIN标志位。如果events[i].events中除了EPOLLIN之外还有其他事件标志也被设置（例如EPOLLOUT、EPOLLERR等），这个运算仍然会返回非零值（即真），只要EPOLLIN标志位被设置。
                char *msg_buffer = (char *)calloc(1024, sizeof(char));
                memset(msg_buffer, 0, sizeof(msg_buffer));
                int ret = recv(current_fd, msg_buffer, 1024, 0);
                if (ret < 0)
                {
                    perror("recv client msg error");
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, 0);
                    close(current_fd);
                    continue;
                }
                else if (ret == 0)
                {
                    printf("有客户端退出连接\n");
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, 0);
                    close(current_fd);
                    continue;
                }
                printf("%s\n", msg_buffer);
                TaskArgs *task_args = (TaskArgs *)calloc(1, sizeof(TaskArgs));
                task_args->epoll_fd = epoll_fd;
                task_args->socket_fd = ready_events[i].data.fd;
                task_args->massage = msg_buffer;
                // threadTask((void *)task_args);
                threadPoolEnqueue(pool, threadTask, (void *)task_args);
                //   不能在此粗暴的开线程，不然同一个socket来一个信息，就开一个线程，多个线程共用了socket，在水平触发下直接丢失了
                //   信息的完整性，即使是边缘触发也会导致回复信息的乱序，因为每个线程都是乱序处理的，例如client发送了几个数据后发
                //   送关闭请求，则可能还没处完成数据就关闭了。
                //   这里用了ONESHOT属性，保证了每一个socket同时只有一个线程访问
            }
            else if (ready_events->events & EPOLLOUT)
            {
                modifyFdEventInEpoll(epoll_fd, current_fd, EPOLLIN);

                pthread_mutex_lock(&global_lock);
                send(current_fd, send_msg, sizeof(send_msg), 0);
                memset(send_msg, 0, sizeof(send_msg));
                pthread_cond_signal(&global_cond);
                pthread_mutex_unlock(&global_lock);
            }
        }
    }
}

void processRead(int epoll_fd, int client_sock_fd, char *recv_buffer)
{
    char **strs = splitString(recv_buffer, '#');
    char msg_buffer[1024];
    memset(msg_buffer, 0, sizeof(msg_buffer));
    if (!strncmp(recv_buffer, "login", 5))
    {
        UserInfo user_info;
        user_info.account = atoi(strs[1]);
        strcpy(user_info.password, strs[2]);
        printf("发起登录 用户名:%s 密码:%s\n", strs[1], strs[2]);

        int ret = loginCheck(&user_info);
        switch (ret)
        {
        case FAIL_LOGIN_NO_ACCOUNT:
            printf("用户名不存在\n");
            sprintf(msg_buffer, "message#failed#用户名不存在");
            break;
        case FAIL_LOGIN_NO_PSW:
            printf("密码错误\n");
            sprintf(msg_buffer, "message#failed#密码错误");
            break;
        case SUCCESS:
            printf("登陆成功\n");
            sprintf(msg_buffer, "message#success#%d#%d#%s#%.2f#%s",
                    user_info.account, user_info.is_manager, user_info.password,
                    user_info.remain_money, user_info.user_name);
            break;
        default:
            break;
        }
        pthread_mutex_lock(&global_lock);
        memset(send_msg, 0, sizeof(send_msg));
        modifyFdEventInEpoll(epoll_fd, client_sock_fd, EPOLLOUT); // 保证了同时只有一个线程发送写请求
        strcpy(send_msg, msg_buffer);

        pthread_cond_wait(&global_cond, &global_lock); // 等待发送成功
        pthread_mutex_unlock(&global_lock);
    }
    else if (!strncmp(recv_buffer, "register", 8))
    {

        UserInfo user_account;
        strcpy(user_account.user_name, strs[1]);
        strcpy(user_account.password, strs[2]);

        printf("发起注册 用户名:%s 密码:%s\n", strs[1], strs[2]);

        int ret = registerUser(user_account);
        if (ret == FAIL_REGISTER_EXIST)
        {
            printf("用户名已存在\n");
            sprintf(msg_buffer, "message#failed#用户名已存在");
            send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
        }
        if (ret == SUCCESS)
        {
            printf("注册成功\n");
            sprintf(msg_buffer, "message#success");
            send(client_sock_fd, msg_buffer, sizeof(msg_buffer), 0);
        }
    }
    else if (!strncmp(recv_buffer, "requestmovie", 12))
    {
        int row, col;
        char sql[256];
        char **rst_info = NULL;

        sprintf(sql, "SELECT * FROM MovieInfo;");

        pthread_mutex_lock(&database_lock); // 操作数据库需要加锁，若已被锁则阻塞等待解锁
        getAllDataFromTable(p_db, sql, &rst_info, &row, &col);
        pthread_mutex_unlock(&database_lock);

        sprintf(msg_buffer, "sendcount#%d#", row);

        pthread_mutex_lock(&global_lock);
        memset(send_msg, 0, sizeof(send_msg));
        modifyFdEventInEpoll(epoll_fd, client_sock_fd, EPOLLOUT);
        strcpy(send_msg, msg_buffer);
        pthread_cond_wait(&global_cond, &global_lock);
        pthread_mutex_unlock(&global_lock);

        for (int i = 1; i <= row; i++)
        {
            sprintf(msg_buffer, "message#requestmovie#success#%s#%s#%s#%s#%s#%s#%s#",
                    rst_info[i * col + 0], rst_info[i * col + 1], rst_info[i * col + 2],
                    rst_info[i * col + 3], rst_info[i * col + 4], rst_info[i * col + 5], rst_info[i * col + 6]);
            pthread_mutex_lock(&global_lock);
            memset(send_msg, 0, sizeof(send_msg));
            modifyFdEventInEpoll(epoll_fd, client_sock_fd, EPOLLOUT); // 保证了同时只有一个线程发送写请求
            strcpy(send_msg, msg_buffer);
            pthread_cond_wait(&global_cond, &global_lock);
            pthread_mutex_unlock(&global_lock);
        }
        sqlite3_free_table(rst_info);
    }
    else if (!strcmp(strs[1], "requestaddmovie"))
    {
        int row, col;
        char sql[256];
        char **rst_info = NULL;
        
        sprintf(sql, "INSERT INTO MovieInfo (CN_Name, EN_Name, Country, Duration, Director_Name, Price) VALUES ('%s', '%s', '%s', '%s', '%s', %s);",
                strs[2], strs[3], strs[4], strs[5], strs[6], strs[7]);
        printf("%s\n", sql);
        pthread_mutex_lock(&database_lock); // 操作数据库需要加锁，若已被锁则阻塞等待解锁
        execSql(p_db, sql, 0, NULL);
        pthread_mutex_unlock(&database_lock);
    }
    else if (!strcmp(strs[1], "requestremovemovie"))
    {
        int row, col;
        char sql[256];
        char **rst_info = NULL;

        sprintf(sql, "UPDATE MovieInfo SET EN_Name, Country, Duration, Director_Name, Price) VALUES ('%s', '%s', '%s', '%s', '%s', %s);",
                strs[3], strs[4], strs[5], strs[6], strs[7], strs[8]);

        pthread_mutex_lock(&database_lock); // 操作数据库需要加锁，若已被锁则阻塞等待解锁
        execSql(p_db, sql, 0, NULL);
        pthread_mutex_unlock(&database_lock);
    }
    else
    {
        printf("发来数据:%s\n", recv_buffer);
    }
}

void addFdEventInEpoll(int epoll_fd, int fd, int is_one_shot)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLET | EPOLLIN | EPOLLRDHUP;
    if (is_one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);

    // 将socket设置为非阻塞
    int old_option = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, old_option | O_NONBLOCK);
}

void modifyFdEventInEpoll(int epoll_fd, int fd, int event_id)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = event_id | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void threadTask(void *args)
{
    int epoll_fd = ((TaskArgs *)args)->epoll_fd;
    int client_sock_fd = ((TaskArgs *)args)->socket_fd;
    char recv_buffer[1024];

    sprintf(recv_buffer, "%s", ((TaskArgs *)args)->massage);
    processRead(epoll_fd, client_sock_fd, recv_buffer);
    modifyFdEventInEpoll(epoll_fd, client_sock_fd, EPOLLIN);

    free(((TaskArgs *)args)->massage);
    free((TaskArgs *)args);
}

int getListenSocket(const char *ip, int port)
{
    int ret = 0;
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1)
        return -1;

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    ret = bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        perror("bind error");
        close(listen_socket);
        return ret;
    }
    ret = listen(listen_socket, 3);
    if (ret < 0)
    {
        perror("listen error");
        close(listen_socket);
        return ret;
    }
    return listen_socket;
}

char **splitString(char *str, char flag)
{
    int max_count = 10;
    int sub_count = 0;
    char **rst = (char **)calloc(max_count, sizeof(char *));
    rst[0] = str;
    while (*str)
    {
        if (sub_count > max_count - 1)
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

Error loginCheck(UserInfo *user_info)
{
    int ret = 0;
    char sql[256];
    UserInfo *rst_info = NULL;

    sprintf(sql, "SELECT * FROM UserInfo WHERE Account = %d;", user_info->account);

    pthread_mutex_lock(&database_lock); // 操作数据库需要加锁，若已被锁则阻塞等待解锁
    ret = execSql(p_db, sql, 1, (void **)&rst_info);
    pthread_mutex_unlock(&database_lock);

    if (ret < 0)
        return FAIL_DATABASE;
    if (rst_info == NULL)
        return FAIL_LOGIN_NO_ACCOUNT;
    if (strcmp(rst_info->password, user_info->password))
        return FAIL_LOGIN_NO_PSW;

    user_info->is_manager = rst_info->is_manager;
    user_info->remain_money = rst_info->remain_money;
    strcpy(user_info->user_name, rst_info->user_name);

    return SUCCESS;
}

Error registerUser(UserInfo user_account)
{
    // ListNode *temp;
    // int ret = searchHashNodeByElementKey(user_hash, &temp, atoi(user_account.account));
    // if (ret >= 0)
    //     return FAIL_REGISTER_EXIST;

    // addHashElement(user_hash, user_account);

    return SUCCESS;
}