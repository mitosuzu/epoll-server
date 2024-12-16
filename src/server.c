#include "server.h"
#include "ThreadPool.h"

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>

void addFdEventInEpoll(int epoll_fd, int fd, int is_one_shot)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLET | EPOLLIN | EPOLLRDHUP;
    if (is_one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);

    //将socket设置为非阻塞
    int old_option = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, old_option | O_NONBLOCK);
}

void modifyFdEventInEpoll(int epoll_fd, int fd, int event_id)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = event_id | EPOLLET |EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void initHash()
{
    initializeHash(&user_hash);
}

void threadTask(void* args)
{
    int epoll_fd = ((TaskArgs*)args)->epoll_fd;
    int client_sock_fd = ((TaskArgs*)args)->socket_fd;
    char recv_buffer[1024];
    sprintf(recv_buffer, "%s", ((TaskArgs*)args)->massage);
    
    processRead(client_sock_fd, recv_buffer);

    modifyFdEventInEpoll(epoll_fd, client_sock_fd, EPOLLIN);
    
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

