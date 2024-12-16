#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <netinet/in.h>
#include "DataStructureLib.h"
#include "serverprocess.h"


void addFdEventInEpoll(int epoll_fd, int fd, int is_one_shot);
static LinkHash *user_hash;
void initHash();

void threadTask(void* args);
int getListenSocket(const char *ip, int port);

#endif