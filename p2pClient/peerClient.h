//
// Created by mooninwater on 2018/10/2.
//

#ifndef P2PNET_PEER_CLIENT_H
#define P2PNET_PEER_CLIENT_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include "../config.h"


using namespace boost::asio;

class peerClient {
private:
    struct peer{
        std::string addr;
        uint16_t port;
        std::string email;
        peer(std::string addr,uint16_t port,std::string mail): addr(addr), port(port), email(mail){}
    };
    std::vector<peer> peerList;
//    std::string hostAddr;
//    uint16_t hostPort;

//    ip::tcp::acceptor acceptor;
    steady_timer timer;
    std::shared_ptr<ip::tcp::socket> serverPtr;
    void refresh();
    void makeList(char* listBuf,size_t bytes_transferred);
    void printList();
//    void connectPeer();
//    void listenLoop();
    void timeHandler(const boost::system::error_code& e);

    char keepChar='k';
    char quitChar='q';
    char refreshChar='r';
    char loginChar='l';
    char passChar='p';

public:
    boost::asio::io_service& io;

    peerClient(boost::asio::io_service& io_service)
    :io(io_service),timer(io_service,boost::asio::chrono::seconds(TIME_INTERVAL))
    {
        ip::tcp::endpoint serverEnd(ip::address_v4::from_string(SERVER_ADDR), SERVER_PORT);
        serverPtr=std::make_shared<ip::tcp::socket>(io);
        boost::system::error_code e;serverPtr->connect(serverEnd,e);if(e){if(DEBUG)printError(e);return;}
        timer.async_wait(boost::bind(&peerClient::timeHandler,this,boost::asio::placeholders::error));
    }
    void start();

    static void printError(const boost::system::error_code& e){
        printf("\nwrong:%s\n",e.message().c_str());
    }
    void quit();
    void sendCommand(const std::string& command);
    void pass(const std::string& command);
};
#endif //P2PNET_PEER_CLIENT_H