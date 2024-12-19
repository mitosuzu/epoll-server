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

int main(int argc, char const *argv[])
{
    // if (argc != 3)
    //     return -1;
    // const char *ip = argv[1];
    // int port = atoi(argv[2]);
    
    int listen_sock_fd = getListenSocket("127.0.0.1", 8084);
    // int listen_sock_fd = getListenSocket(ip, port);
    if (listen_sock_fd < 0)
        return -1;
    printf("Listening...\n");
    initReactorThread(listen_sock_fd, 4);
    close(listen_sock_fd);

    return 0;
}