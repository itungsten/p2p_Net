//
// Created by tungsten on 3/17/2021.
//

#ifndef SECUREFILETRANS_SOCKETOR_H
#define SECUREFILETRANS_SOCKETOR_H

#include <iostream>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <cstring>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>
#include "../config.h"
#include "utils.h"
using namespace std;

class Socketor {
private:
    int sockfd;
public:
    bool getConnect(const char* const host, int port);
    bool disConnect();
    int recvMsg(char * const buf,const size_t len);
    void sendMsg(const char * const msg,int len);
    int getSocket(){
        return sockfd;
    }
};


#endif //SECUREFILETRANS_SOCKETOR_H
