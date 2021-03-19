//
// Created by mooninwater on 2018/10/2.
//
#include <deque>
#include <vector>
#include <iostream>
#include "peerClient.h"


void peerClient::start(){
    refresh();
    boost::thread td(boost::bind(&boost::asio::io_service::run,&io));
    std::string command;char kind;
    while(std::cout<<"\n>>>"&&std::getline(std::cin,command)){
        kind=command[0];
        switch(kind){
            case 'q':
            {
                quit();
                break;
            }
            case 'r':
            {
                refresh();
                break;
            }
            case 'l':
            {
                sendCommand(command);
                break;
            }
            case 'p':
            {
                sendCommand(command);
                break;
            }
        }
    }
}
void peerClient::makeList(char* listBuf,size_t bytes_transferred) {
    char addr_str[16];
    peerList.clear();
    for (std::size_t i = sizeof(std::size_t); i < bytes_transferred;) {
        uint32_t addr_binary = *((uint32_t *) (listBuf + i));
        if (inet_ntop(AF_INET, (void *) &addr_binary, addr_str, 16) != nullptr) {
            std::string addr(addr_str);
            i = i + 4;
            uint16_t port = *(uint16_t *) (listBuf + i);
            i = i + 2;
            std::string email((char *)(listBuf+i));
            i = i + 30;
            peerList.push_back({addr, port,email});
        } else {
            return;
        }
    }
}

void peerClient::timeHandler(const boost::system::error_code& e) {
    timer.expires_at(timer.expiry()+boost::asio::chrono::seconds(TIME_INTERVAL));
    serverPtr->async_send(boost::asio::buffer(&keepChar, 1), [](const boost::system::error_code& error, \
                                                          std::size_t bytes_transferred ){
    });
    timer.async_wait(boost::bind(&peerClient::timeHandler,this,boost::asio::placeholders::error));
}
void peerClient::printList() {
    std::cout<<"\nPeer List\n";
    for(const auto&iter:peerList){
        std::cout<<iter.addr<<":"<<iter.port<<":"<<iter.email<<std::endl;
    }
}
void peerClient::quit() {
    serverPtr->send(boost::asio::buffer(&quitChar, 1));
    serverPtr->close();
    exit(0);
}
void peerClient::refresh() {
    char listBuf[LIST_SIZE];
    boost::system::error_code e;
    serverPtr->send(boost::asio::buffer(&refreshChar, 1));
    size_t bytes_transferred=serverPtr->read_some(boost::asio::buffer((void *) listBuf, LIST_SIZE),e);
    if (e && e != boost::asio::error::eof) {
        if(DEBUG)printError(e);
    }
    else{
        makeList(listBuf,bytes_transferred);
        printList();
    }
}
void peerClient::sendCommand(const std::string& command) {
    char recvBuf[RECV_BUF_SIZE];
    serverPtr->send(boost::asio::buffer(command.c_str(), command.length()+1));
    boost::system::error_code e;
    size_t bytes_transferred=serverPtr->read_some(boost::asio::buffer((void *)recvBuf,RECV_BUF_SIZE),e);
    if (e && e != boost::asio::error::eof) {
        if(DEBUG)printError(e);
    }
    else{
        std::cout.write(recvBuf,bytes_transferred);
    }
}

//void peerClient::listenLoop() {
//    std::shared_ptr<ip::tcp::socket> p_data_socket=std::make_shared<ip::tcp::socket>(io);
//    acceptor.async_accept(*p_data_socket, [p_data_socket,this](boost::system::error_code ec){
//        if(e) {
//            printError(e);
//            return;
//        }
//        this->listenLoop();
//        std::shared_ptr<connection> c=std::make_shared<connection>(\
//                        p_data_socket->remote_endpoint().address().to_string(),\
//                        p_data_socket->remote_endpoint().port(),*this);
//        c->read_msg_from_socket(p_data_socket);
//    });
//}

//void peerClient::connectPeer() {
//    for (const auto &peer:this->peerList) {
//        if (hostAddr == peer.addr &&
//            hostPort == peer.port) {
//            printf("self  addr:%s  port:%u\n", peer.addr.c_str(), peer.port);
//        } else {
//            printf("other addr:%s  port:%u\n", peer.addr.c_str(), peer.port);
//            std::shared_ptr<connection> c=std::make_shared<connection>(peer.addr, peer.port, *this);
//            //连接其他节点 并发送消息
//            c->connect_peer();
//        }
//    }
//
//}