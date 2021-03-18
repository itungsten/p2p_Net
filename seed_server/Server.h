//
// Created by mooninwater on 2018/10/2.
//

#ifndef P2PNET_SERVER_H
#define P2PNET_SERVER_H

#include <string>
#include <list>
#include "../config.h"
#include "utils.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
using namespace boost::asio;

class Server {
private:
    struct Client{
        time_t lastTime;
        uint32_t hostAddr;
        uint16_t hostPort;
        uint16_t verifyTime;
        int MAC;
        std::string email;

        std::shared_ptr<ip::tcp::socket> socketPtr;
        Client(uint32_t addr,uint16_t port,const std::shared_ptr<ip::tcp::socket>& ptr)
        : hostAddr(addr),hostPort(port),lastTime(time(0)),socketPtr(ptr),verifyTime(VERIFY_TIME),MAC(rand())
        {
        }
        bool operator==(const Client& o){
            return socketPtr==o.socketPtr;
        }
        bool isActive()const{
            bool active=(time(0)-lastTime)<=TIME_LIMIT;
            std::string addr=socketPtr->remote_endpoint().address().to_string();
            if(active)std::cout<<addr<<" is active\n";
            else std::cout<<addr<<" isn't active";
            return active;
        }
        void update(){
            lastTime=time(0);
        }
        bool sendMail(const std::string& _email){
            email=_email;
            MAC=rand();
            UTILS::sendMail(email,"SecureFileTrans MAC","Your MAC code is "+std::to_string(MAC)+" .USE it QUICKLY!");
        }
    };

    std::list<Client> clients;
    boost::asio::steady_timer timer;
    boost::asio::io_service& io;             //io context
    ip::tcp::acceptor acceptor;

    void startListen();
    void addClient(const std::shared_ptr<ip::tcp::socket>& socketPtr);
    void sendList(const std::shared_ptr<ip::tcp::socket>& socketPtr);
    void keepRead(const std::shared_ptr<ip::tcp::socket>& socketPtr);
    void timeHandler(const boost::system::error_code& e);
    void readHandler(const boost::system::error_code& e,size_t bytes_transferred,std::shared_ptr<ip::tcp::socket> socketPtr,std::shared_ptr<std::array<char,RECV_BUF_SIZE>> recvBuf);

    void printError(const boost::system::error_code& e){
        if(DEBUG)printf("\nwrong:%s\n",e.message().c_str());
    }
    Client& getClientBySocketPtr(const std::shared_ptr<ip::tcp::socket>& socketPtr){
        for(auto& iter:clients){
            if(iter.socketPtr==socketPtr)return iter;
        }
        if(DEBUG)("\nClient Not Found!\n");
    }
public:
    Server(boost::asio::io_service& io);
};

#endif //P2PNET_SERVER_H
