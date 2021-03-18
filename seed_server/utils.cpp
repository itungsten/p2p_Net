//
// Created by tungsten on 3/16/2021.
//
#include "utils.h"
#include "Socketor.h"

void UTILS::base64_encode(const  char*in,  int inlen,char* out,  int &outlen)
{
#define BASE64_PAD '='
    static const char base64en[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3',
            '4', '5', '6', '7', '8', '9', '+', '/',
    };
    int s;
    unsigned int i;
    unsigned int j;
    unsigned char c;
    unsigned char l;

    s = 0;
    l = 0;
    for (i = j = 0; i < inlen; i++) {
        c = in[i];

        switch (s) {
            case 0:
                s = 1;
                out[j++] = base64en[(c >> 2) & 0x3F];
                break;
            case 1:
                s = 2;
                out[j++] = base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)];
                break;
            case 2:
                s = 0;
                out[j++] = base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)];
                out[j++] = base64en[c & 0x3F];
                break;
        }
        l = c;
    }

    switch (s) {
        case 1:
            out[j++] = base64en[(l & 0x3) << 4];
            out[j++] = BASE64_PAD;
            out[j++] = BASE64_PAD;
            break;
        case 2:
            out[j++] = base64en[(l & 0xF) << 2];
            out[j++] = BASE64_PAD;
            break;
    }

    out[j] = 0;
    outlen = j;
    //https://github.com/zhicheng/base64
}

bool UTILS::sendMail(const string& target,const string& subject,const string& body){
    Socketor sock;sock.getConnect(MAIL_HOST,MAIL_PORT);

    char outBuf[MAIL_BUF];int outLen= 0;bzero(outBuf,MAIL_BUF);

    outLen=sock.recvMsg(outBuf,MAIL_BUF);

    sock.sendMsg("helo " MAIL_ID "\r\n",-1);  //连接确认
    outLen=sock.recvMsg(outBuf,MAIL_BUF);

    sock.sendMsg("auth login\r\n",-1);   //账号登陆
    outLen=sock.recvMsg(outBuf,MAIL_BUF);

    base64_encode(MAIL_USER, strlen(MAIL_USER), outBuf, outLen);strcat(outBuf, "\r\n\0");
    sock.sendMsg(outBuf,-1);
    outLen=sock.recvMsg(outBuf,MAIL_BUF);

    //输入密码
    base64_encode(MAIL_PASS, strlen(MAIL_PASS), outBuf, outLen);strcat(outBuf, "\r\n\0");
    sock.sendMsg(outBuf,-1);
    outLen=sock.recvMsg(outBuf,MAIL_BUF);

    string FromMail = "MAIL FROM: <" MAIL_USER ">\r\n";
    sock.sendMsg(FromMail.c_str(),FromMail.length());
    outLen=sock.recvMsg(outBuf,MAIL_BUF);

    //邮件接收人
    string ToMail = "RCPT TO: <" + target + ">\r\n";
    sock.sendMsg(ToMail.c_str(),ToMail.length());
    outLen=sock.recvMsg(outBuf,MAIL_BUF);

    //邮件正文
    sock.sendMsg("DATA\r\n",-1);
    FromMail = "From:" MAIL_USER "\r\n";
    ToMail = "To:<"+target+">\r\n";
    sock.sendMsg(FromMail.c_str(),FromMail.length());
    sock.sendMsg(ToMail.c_str(),ToMail.length());
    string tmp="Subject:"+subject+"\r\n\n";
    sock.sendMsg(tmp.c_str(),tmp.length());
    tmp=body+"\r\n";
    sock.sendMsg(tmp.c_str(),tmp.length());
    sock.sendMsg("\r\n.\r\n",-1);

    outLen=sock.recvMsg(outBuf,MAIL_BUF);
    sock.disConnect();
    return true;
}