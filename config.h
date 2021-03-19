//
// Created by tungsten on 3/18/2021.
//

#ifndef P2PNET_CONFIG_H
#define P2PNET_CONFIG_H

#include<ctime>
#include<array>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#define MAX_SIZE (1*(1<<10))
#define MAIL_ID "tungsten"
#define MAIL_HOST "smtp.qq.com"
#define MAIL_PORT 25
#define MAIL_BUF 512
#define MAIL_USER "1019205908@qq.com"
#define MAIL_PASS "aydrcawbygssbgai"
#define DEBUG true
#define SERVER_PORT 666
#define SERVER_ADDR "0.0.0.0"
#define TIME_INTERVAL 6
#define TIME_LIMIT 12
#define RECV_BUF_SIZE 512
#define SEND_BUF_SIZE 512
#define VERIFY_TIME 30
#define LIST_SIZE 0x1000

#endif //P2PNET_CONFIG_H
