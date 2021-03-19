//
// Created by mooninwater on 2018/10/2.
//

#include "Server.h"
Server::Server(boost::asio::io_service& io)
        : io(io), timer(io,boost::asio::chrono::seconds(TIME_INTERVAL)),acceptor(io,ip::tcp::endpoint(ip::address_v4::from_string(SERVER_ADDR),SERVER_PORT),true)
{
    srand(time(0));
    startListen();
    timer.async_wait(boost::bind(&Server::timeHandler,this,boost::asio::placeholders::error));
}
void Server::startListen() {
    //创建一个通信套接字
    std::shared_ptr<ip::tcp::socket> socketPtr=std::make_shared<ip::tcp::socket>(io);
    acceptor.async_accept(*socketPtr, [this,socketPtr](boost::system::error_code e){
        if(e){
            printError(e);
            return;
        }
        addClient(socketPtr);
        //sendList(socketPtr);
        keepRead(socketPtr);
        //递归启动一个循环，等待下一个连接
        this->startListen();
    });
}
void Server::timeHandler(const boost::system::error_code& e){
    if(e){
        printError(e);
        return;
    }
    timer.expires_at(timer.expiry()+boost::asio::chrono::seconds(TIME_INTERVAL));
    std::list<Client>::iterator i=clients.begin();
    while(i!=clients.end()){
        if((!(i->isActive()))){
            std::cout<<"\nRemote: "<<(i->socketPtr)->remote_endpoint().address().to_string()<<":"<<i->hostPort<<" Disconnect!\n";
            i->socketPtr->close();
            clients.erase(i++);
        }
        else if((i->verifyTime==0)){
            std::cout<<"\nRemote: "<<(i->socketPtr)->remote_endpoint().address().to_string()<<":"<<i->hostPort<<" too long to VERIFY!\n";
            i->socketPtr->close();
            clients.erase(i++);
        }
        else{
            if(i->verifyTime>0)i->verifyTime--;
            ++i;
        }
    }
    timer.async_wait(boost::bind(&Server::timeHandler,this,boost::asio::placeholders::error));
}
void Server::readHandler(const boost::system::error_code& e,size_t bytes_transferred,std::shared_ptr<ip::tcp::socket> socketPtr,std::shared_ptr<std::array<char,RECV_BUF_SIZE>> recvBuf){
    if(e){
        printError(e);return;
    }
//    std::cout.write((*recvBuf).data(),bytes_transferred);
    char command=(*recvBuf)[0];
    switch (command) {
        case 'k':
        {
            //keep link
            getClientBySocketPtr(socketPtr).update();
            break;
        }
        case 'q':
        {
            //quit
            clients.remove(Client(0,0,socketPtr));
            socketPtr->close();
            break;
        }
        case 'l':
        {
            //login    usage:l<space><your email>
            std::string email((*recvBuf).data()+2);
            email=email.substr(0,email.length());
            getClientBySocketPtr(socketPtr).sendMail(email);
			char* bufPtr=new char[16];
            sprintf(bufPtr,"\nEmail Sended!\n");
            boost::asio::async_write(*socketPtr, boost::asio::buffer((void*)bufPtr, 16),\
                    [this,bufPtr](const boost::system::error_code& e, std::size_t bytes_transferred){
                    if(e) {
                        if(DEBUG)printError(e);
                    }
                    delete[] bufPtr;
            });
            break;
        }
        case 'p':
        {
            //password
            std::string MAC((*recvBuf).data()+2);
            MAC=MAC.substr(0,MAC.length());
            Client& client=getClientBySocketPtr(socketPtr);
            if(std::stoi(MAC)==client.MAC){
                client.verifyTime=-1;
                printf("\nUser %s has verified!\n",client.email.c_str());
                char* bufPtr=new char[15];
                sprintf(bufPtr,"\nCorrect MAC!\n");
                boost::asio::async_write(*socketPtr, boost::asio::buffer((void*)bufPtr, 15),\
                    [this,bufPtr](const boost::system::error_code& e, std::size_t bytes_transferred){
                    if(e) {
                        if(DEBUG)printError(e);
                    }
                    delete[] bufPtr;
                });
            }
            else{
                char* bufPtr=new char[13];
                sprintf(bufPtr,"\nWrong MAC!\n");
                boost::asio::async_write(*socketPtr, boost::asio::buffer((void*)bufPtr, 13),\
                    [this,bufPtr](const boost::system::error_code& e, std::size_t bytes_transferred){
                    if(e) {
                        if(DEBUG)printError(e);
                    }
                    delete[] bufPtr;
                });
            }
            break;
        }
        case 'r':
        {
            //refresh list:
            sendList(socketPtr);
            break;
        }
    }
    socketPtr->async_receive(boost::asio::buffer(*recvBuf),boost::bind(&Server::readHandler,this,boost::asio::placeholders::error,\
                                                                                boost::asio::placeholders::bytes_transferred,socketPtr,recvBuf));

}
void Server::addClient(const std::shared_ptr<ip::tcp::socket>& socketPtr){
    printf("\nfind new connect\n");
    auto addr_str=socketPtr->remote_endpoint().address().to_string();
    uint32_t addrBuf;
    auto ret=inet_pton(AF_INET, addr_str.c_str(), (void*)&addrBuf);
    if(1!=ret){
        if(DEBUG)printf("wrong:inet_pton faild\n");
        return;
    }
    auto portNum=socketPtr->remote_endpoint().port();
    printf("remote ip:%s\n",addr_str.c_str());
    printf("remote port:%d\n", portNum);
    auto endPtr=clients.insert(clients.end(), Client(addrBuf,portNum,socketPtr));
}
void Server::sendList(const std::shared_ptr<ip::tcp::socket>& socketPtr){
    //构造peer_list报文，将peer list 发送给客户端
    // message_size | addr[4字节] | port[2字节] | email[30字节]........
    std::size_t bufLen= sizeof(std::size_t) + clients.size() * 36;
    std::size_t realLen = sizeof(std::size_t);
    char* bufPtr=new char[bufLen];
    char* bufPtrBK=bufPtr;
    bufPtr+=sizeof(std::size_t);
    for(const auto &iter:clients){
        if(iter.verifyTime<0){
            *((uint32_t *)bufPtr)=iter.hostAddr;
            bufPtr+=4;
            *((uint16_t *)bufPtr)=iter.hostPort;
            bufPtr+=2;
            std::sprintf(bufPtr,"%s",iter.email.c_str());
            bufPtr+=30;
            realLen+=36;
        }
    }
    *((std::size_t *)bufPtr)=realLen;

    boost::asio::async_write(*socketPtr, boost::asio::buffer((void*)bufPtrBK, realLen),\
                    [bufPtrBK,socketPtr](const boost::system::error_code& error, std::size_t bytes_transferred){
        if(error) {
            if(DEBUG)printf("wrong: write data err: %s\n", error.message().c_str());
        }
        delete[] bufPtrBK;
    });
}
void Server::keepRead(const std::shared_ptr<ip::tcp::socket>& socketPtr){
    std::shared_ptr<std::array<char,RECV_BUF_SIZE>> recvBuf(new std::array<char,RECV_BUF_SIZE>);
    socketPtr->async_receive(boost::asio::buffer(*recvBuf),boost::bind(&Server::readHandler,this,boost::asio::placeholders::error,\
                                                                            boost::asio::placeholders::bytes_transferred,socketPtr,recvBuf));
}
