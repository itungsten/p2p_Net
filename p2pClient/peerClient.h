//
// Created by tungsten on 2021/3/18.
//

#ifndef P2PNET_PEER_CLIENT_H
#define P2PNET_PEER_CLIENT_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include "../config.h"
#include "SM2Translator.h"


using namespace boost::asio;

class peerClient {
private:
    struct peer{
        std::string email;
        std::string addr;
        uint16_t port;
        peer(std::string addr,uint16_t port,std::string mail): addr(addr), port(port), email(mail){}
    };
    std::vector<peer> peerList;

    std::shared_ptr<ip::tcp::socket> serverPtr; //socket to  server
    std::shared_ptr<ip::tcp::acceptor> acceptorPtr;//listen to peer
    boost::asio::steady_timer timer;    //for heartbeat

    pair<string,string> key;

    void refreshList();
    void makeList(char* listBuf,size_t bytes_transferred);
    void printList();
    void timeHandler(const boost::system::error_code& e);
    void digHandler(const boost::system::error_code& e,std::size_t bytes_transfered,char* recvBuf);
    void download();

    char keepChar='k';
    bool isOpen= false;

public:
    boost::asio::io_service& io;
    explicit peerClient(boost::asio::io_service& io_service);
    void open();
    void sendFile();
    void sendMsg(const std::string& msg);
    std::string recvMsg();
    void interactive();
};
#endif //P2PNET_PEER_CLIENT_H