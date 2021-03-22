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
#include <boost/asio.hpp>

#define MAX_SIZE (1*(1<<10))
#define MAIL_ID "tungsten"
#define MAIL_HOST "smtp.qq.com"
#define MAIL_PORT 25
#define MAIL_BUF 512
#define MAIL_USER "1019205908@qq.com"
#define MAIL_PASS "zkejkzybijotbaih"
#define DEBUG true
#define SERVER_PORT 666
#define SERVER_ADDR "127.0.0.1"
#define TIME_INTERVAL 6
#define TIME_LIMIT 12
#define RECV_BUF_SIZE 512
#define SEND_BUF_SIZE 512
#define VERIFY_TIME 100
#define LIST_SIZE 0x1000
#define NAME_BUF 32
#define FILE_BUF 1024
inline static void printError(const boost::system::error_code& e){
	printf("\nwrong:%s\n",e.message().c_str());
}


#endif //P2PNET_CONFIG_H
