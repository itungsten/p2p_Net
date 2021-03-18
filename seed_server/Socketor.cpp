//
// Created by tungsten on 3/17/2021.
//

#include "Socketor.h"
#include "utils.h"

bool Socketor::getConnect(const char* const host, int port)
{
    int len;
    struct sockaddr_in address;
    struct hostent* h;
    sockfd = socket(AF_INET,SOCK_STREAM,0);

    int length = strlen(host);
    h = gethostbyname(host);
    if(NULL==h){
        cout<<"Fail to resolve host!"<<endl;
        return false;
    }
    address.sin_addr.s_addr = *((unsigned long*)h->h_addr_list[0]);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    len = sizeof(address);
    connect(sockfd,(struct sockaddr*)&address,len);
    if(DEBUG)cout << "connect socket" << endl;
    return true;
}
bool Socketor::disConnect()
{
    close(sockfd);
    if(DEBUG)cout << "close socket" << endl;
    return true;
}

int Socketor::recvMsg(char* const buf,const size_t len)
{
    bzero(buf,len);
    int resLen=recv(sockfd,buf,len,0);
    if(DEBUG)cout << "\n" << "recv: " << buf << "\n";
    return resLen;
}

void Socketor::sendMsg(const char * const msg,int len)
{
    if(-1==len)len=strlen(msg);
    send(sockfd,msg,len,0);
    if(DEBUG)cout << "\n" << "senc:" << msg << "\n";
}