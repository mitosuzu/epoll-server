#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <errno.h>
#include <fcntl.h>

#include "server.h"
#include "ThreadPool.h"

#define MAXSIZE 1024

int main(int argc, char const *argv[])
{
    // if (argc != 3)
    //     return -1;
    // const char *ip = argv[1];
    // int port = atoi(argv[2]);

    struct epoll_event listen_event;          // listen套接字的事件
    struct epoll_event ready_events[MAXSIZE]; // 就绪事件组
    int listen_sock_fd = getListenSocket("127.0.0.1", 8092);
    // int listen_sock_fd = getListenSocket(ip, port);
    if (listen_sock_fd < 0)
        return -1;
    printf("Listening...\n");

    ThreadPool *pool = threadPoolCreate(4);
    int epoll_fd = epoll_create(5); // 形参为了兼容旧版本，当前版本参数无实际意义
    addFdEventInEpoll(epoll_fd, listen_sock_fd, 0);

    // 每次处理的最大连接数目
    const int MAX_ACCEPTS_PER_CALL = 200;
    // 当前数量
    int current_accept;

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
                    bzero(&client_addr, sizeof(client_addr));
                    bzero(&event_recv, sizeof(event_recv));

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
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, 0 );
                    close(current_fd);
                    continue;
                }
                else if (ret == 0)
                {
                    printf("有客户端退出连接\n");
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, 0 );
                    close(current_fd);
                    continue;
                }
                TaskArgs *task_args = (TaskArgs *)calloc(1, sizeof(TaskArgs));
                task_args->epoll_fd = epoll_fd;
                task_args->socket_fd = ready_events[i].data.fd;
                task_args->massage = msg_buffer;
                threadTask((void *)task_args);
                //threadPoolEnqueue(pool, threadTask, (void *)task_args);
                // 不能在此粗暴的开线程，不然同一个socket来一个信息，就开一个线程，多个线程共用了socket，在水平触发下直接丢失了
                // 信息的完整性，即使是边缘触发也会导致回复信息的乱序，因为每个线程都是乱序处理的，例如client发送了几个数据后发
                // 送关闭请求，则可能还没处完成数据就关闭了。
                // 这里用了ONESHOT属性，保证了每一个socket同时只有一个线程访问
            }
            else if (ready_events->events & EPOLLOUT)
            {
            }
        }
    }
    close(listen_sock_fd);
    return 0;
}