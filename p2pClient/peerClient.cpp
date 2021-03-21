//
// Created by tungsten on 2021/3/18.
//
#include <vector>
#include <iostream>
#include "peerClient.h"
peerClient::peerClient(boost::asio::io_service& io_service)
:io(io_service),timer(io_service,boost::asio::chrono::seconds(TIME_INTERVAL))
{
    //    connect to server
    ip::tcp::endpoint serverEnd(ip::address_v4::from_string(SERVER_ADDR), SERVER_PORT);
    serverPtr=std::make_shared<ip::tcp::socket>(io);
    boost::system::error_code e;
    serverPtr->connect(serverEnd,e);
    if(e){if(DEBUG)printError(e);return;}

    //    launch timer(for heart beat)
    timer.async_wait(boost::bind(&peerClient::timeHandler,this,boost::asio::placeholders::error));
    boost::thread td(boost::bind(&boost::asio::io_service::run,&io));

    //    CLI interface
    interactive();
}

void peerClient::interactive() {
    std::string command;char kind;
    while(std::cout<<(isOpen?"\n(Open )":"\n(Close)")<<">>>"&&std::getline(std::cin,command)){
        kind=command[0];
        sendMsg(command);
        switch(kind){
            case 'q':
            {
                exit(0);
                break;
            }
            case 'r':
            {
                refreshList();
                break;
            }
            case 'l':
            {
                std::cout<<recvMsg();
                break;
            }
            case 'p':
            {
                std::cout<<recvMsg();
                break;
            }
            case 'o':{
                if(!isOpen)open();
                break;
            }
            case 'c':
            {
                isOpen=false;
                break;
            }
            case 's':{
                sendFile();
                break;
            }
        }
    }
}
void peerClient::sendFile() {
    std::string recvBuf=recvMsg();
    char* bufPtr=(char*)recvBuf.c_str();
    uint16_t port=*((uint16_t*)bufPtr);
    std::string addr(bufPtr+sizeof(uint16_t));
    if(DEBUG)std::cout<<"The remote addr is "<<addr<<" and the port is "<<port<<std::endl;

    try {
        ip::tcp::endpoint peerEnd(ip::address_v4::from_string(addr),port);
        ip::tcp::socket peerSocket(io);
        boost::system::error_code e;
        peerSocket.connect(peerEnd);;if(e){if(DEBUG)printError(e);return;}
        //connect to remote peer

        std::cout<<"\nPlease input testMsg.\n>>>";
        int len;std::cin>>len;
        std::string testMsg;std::cin >> testMsg;
        peerSocket.send(boost::asio::buffer(&len,sizeof(int)));  //len
        peerSocket.send(boost::asio::buffer(testMsg.c_str(), len)); //body
        peerSocket.close();
    }
    catch(const std::exception& e){
        if(DEBUG)std::cout<<e.what()<<std::endl;
        if(DEBUG)std::cout<<"NetWork Error! Try Again!"<<std::endl;
        return;
    }
}
void peerClient::digHandler(const boost::system::error_code& e,std::size_t bytes_transfered,char* recvBuf){
    if(e||!isOpen){
        if(e&&DEBUG)printError(e);
        delete [] recvBuf;
        return;
    }
    isOpen=false;
    std::string addr_str;addr_str.resize(16);
    uint32_t addr_binary = *((uint32_t *) recvBuf);
    inet_ntop(AF_INET, (void *) &addr_binary, (char*)addr_str.c_str(), 16);
    uint32_t portNum = *((uint16_t*)(recvBuf+4));
    std::string email_str(recvBuf+6);
    if(DEBUG)std::cout<<"\n Remote peer's pubic info is "<<addr_str.c_str()<<":"<<portNum<<":"<<email_str<<"\n"<<std::flush;
    boost::thread td(boost::bind(&peerClient::download,this));
}
void peerClient::download() {
    try{
        char send_buf[SEND_BUF_SIZE];
        acceptorPtr=std::make_shared<ip::tcp::acceptor>(io, ip::tcp::endpoint(ip::tcp::v4(), 0));
        uint16_t port=acceptorPtr->local_endpoint().port();
        std::string addr =acceptorPtr->local_endpoint().address().to_string();
        *((uint16_t*)send_buf)=port;
        if(DEBUG)std::cout<<"The new acceptor's addr is "<<addr<<" the port is "<<port<<std::endl;
        sprintf(send_buf+2,"%s",addr.c_str());
        serverPtr->send(boost::asio::buffer(send_buf,2+addr.length()+1));
        //send to server to transmit

        ip::tcp::socket peerSocket(io);
        acceptorPtr->accept(peerSocket);
        //connect to peer

        int len;
        size_t bytes_transferred=boost::asio::read(peerSocket,boost::asio::buffer(&len,sizeof(int)));
        std::cout<<len;
        std::cout<<bytes_transferred;
        char* fileBuf=new char[len];
        bytes_transferred=boost::asio::read(peerSocket,boost::asio::buffer(fileBuf,len));
        std::cout<<bytes_transferred;
        std::cout<<fileBuf<<std::endl;
        delete [] fileBuf;
        acceptorPtr->close();
        peerSocket.close();
        sendMsg("c");
    }
    catch(const std::exception& e){
        if(DEBUG)std::cout<<e.what()<<std::endl;
        if(DEBUG)std::cout<<"NetWork Error! Try Again!"<<std::endl;
        return;
    }
}
void peerClient::refreshList() {
    char listBuf[LIST_SIZE];
    boost::system::error_code e;
    size_t bytes_transferred=serverPtr->read_some(boost::asio::buffer((void *) listBuf, LIST_SIZE),e);
    if (e && e != boost::asio::error::eof) {
        if(DEBUG)printError(e);
        exit(1);
    }
    makeList(listBuf,bytes_transferred);
    printList();
}
void peerClient::makeList(char* listBuf,size_t bytes_transferred) {
    peerList.clear();
    char addr_str[16];
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
            if(DEBUG)printf("\nError in makeList!\n");
            return;
        }
    }
}
void peerClient::printList() {
    std::cout<<"\nPeer List\n";
    for(const auto&iter:peerList){
        std::cout<<iter.addr<<":"<<iter.port<<":"<<iter.email<<std::endl;
    }
}
void peerClient::sendMsg(const std::string& msg) {
    serverPtr->send(boost::asio::buffer(msg.c_str(), msg.length() + 1));
}
std::string peerClient::recvMsg() {
    char recvBuf[RECV_BUF_SIZE];
    boost::system::error_code e;
    size_t bytes_transferred=serverPtr->read_some(boost::asio::buffer((void *)recvBuf,RECV_BUF_SIZE),e);
    if (e && e != boost::asio::error::eof) {
        if(DEBUG)printError(e);
        exit(1);
    }
    else{
        return std::string(recvBuf,bytes_transferred);
    }
}
void peerClient::open() {
    isOpen=true;
    char* recvBuf=new char[RECV_BUF_SIZE];
    serverPtr->async_read_some(boost::asio::buffer(recvBuf,RECV_BUF_SIZE),boost::bind(&peerClient::digHandler,this,\
                                                                            boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,recvBuf));
}
void peerClient::timeHandler(const boost::system::error_code& e) {
    timer.expires_at(timer.expiry()+boost::asio::chrono::seconds(TIME_INTERVAL));
    serverPtr->async_send(boost::asio::buffer(&keepChar, 1), [](const boost::system::error_code& error, \
                                                          std::size_t bytes_transferred ){
    });
    timer.async_wait(boost::bind(&peerClient::timeHandler,this,boost::asio::placeholders::error));
}