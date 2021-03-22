//
// Created by tungsten on 2021/3/18.
//
#include <vector>
#include <iostream>
#include <fstream>
#include "peerClient.h"

static string readAll(ifstream& inFile){
    string res;
    char ch;
    while(inFile.get(ch)){
        res.push_back(ch);
    }
    return res;
}
std::size_t writeAll(ofstream& outFile,const string& buf){
    size_t len=buf.size();
    for(int i=0;i<len;++i)outFile.put(buf[i]);
    return len;
}
peerClient::peerClient(boost::asio::io_service& io_service)
:io(io_service),timer(io_service,boost::asio::chrono::seconds(TIME_INTERVAL))
{
    key=SM2Translator::GenKey();

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
//    if(DEBUG)std::cout<<"The remote addr is "<<addr<<" and the port is "<<port<<std::endl;

    try {
        ip::tcp::endpoint peerEnd(ip::address_v4::from_string(addr),port);
        ip::tcp::socket peerSocket(io);
        boost::system::error_code e;
        peerSocket.connect(peerEnd);;if(e){if(DEBUG)printError(e);return;}
        //connect to remote peer

        char pubKeyBuf[179];
        size_t bytes_transferred=boost::asio::read(peerSocket,boost::asio::buffer(pubKeyBuf,179));
        string peerPubKey(pubKeyBuf);
//        cout<<peerPubKey<<endl;
        //receive public key

        std::cout<<"\nPlease input filePath.\n>>>";
        std::string filePath;std::cin >> filePath;
        ifstream oriFile(filePath,ios::in|ios::binary);
        string oriBody=readAll(oriFile);
        oriFile.close();
//        cout<<"Origin is "<<oriBody<<endl;
//        cout<<oriBody.length()<<endl;
//        cout<<peerPubKey.length()<<endl;
        string encBody=SM2Translator::Encrypt(peerPubKey,oriBody);
        int len=encBody.length();
//        cout<<len;
        char infoBuf[30 + sizeof(int)];
        *((int*)infoBuf)=len;
        sprintf(infoBuf+sizeof(int),"%s",filePath.c_str());
        boost::asio::write(peerSocket,boost::asio::buffer(infoBuf,sizeof(infoBuf)));//len
        boost::asio::write(peerSocket,boost::asio::buffer(encBody.c_str(), len)); //body
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
//    if(DEBUG)std::cout<<"\n Remote peer's pubic info is "<<addr_str.c_str()<<":"<<portNum<<":"<<email_str<<"\n"<<std::flush;
    boost::thread td(boost::bind(&peerClient::download,this));
}
void peerClient::download() {
    try{
        char send_buf[SEND_BUF_SIZE];
        acceptorPtr=std::make_shared<ip::tcp::acceptor>(io, ip::tcp::endpoint(ip::tcp::v4(), 0));
        uint16_t port=acceptorPtr->local_endpoint().port();
        std::string addr =acceptorPtr->local_endpoint().address().to_string();
        *((uint16_t*)send_buf)=port;
//        if(DEBUG)std::cout<<"The new acceptor's addr is "<<addr<<" the port is "<<port<<std::endl;
        sprintf(send_buf+2,"%s",addr.c_str());
        serverPtr->send(boost::asio::buffer(send_buf,2+addr.length()+1));
        //send to server to transmit

        ip::tcp::socket peerSocket(io);
        acceptorPtr->accept(peerSocket);
        //connect to peer

        size_t bytes_transferred=peerSocket.send(boost::asio::buffer(key.first.c_str(),179));

        int len;
        char infoBuf[30 + sizeof(len)];
        bytes_transferred=boost::asio::read(peerSocket,boost::asio::buffer(infoBuf, sizeof(infoBuf)));
        len=*((int* )infoBuf);
//        std::cout<<len<<endl;
//        std::cout<<bytes_transferred<<endl;
        char* fileBuf=new char[len];
        bytes_transferred=boost::asio::read(peerSocket,boost::asio::buffer(fileBuf,len));
//        std::cout<<bytes_transferred<<endl;
        string encBody(fileBuf,len);
//        cout<<encBody.length();
        string decBody=SM2Translator::Decrypt(key.second,encBody);
        string oriName=(infoBuf+sizeof(int));
        string recvName="recv_"+oriName;
        ofstream decFile(recvName.c_str(), ios::out | ios ::binary);
//        cout<<decBody<<endl;
        writeAll(decFile,decBody);
        decFile.close();
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
