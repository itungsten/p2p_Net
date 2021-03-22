//
// Created by tungsten on 3/16/2021.
//

#ifndef SECUREFILETRANS_UTILS_H
#define SECUREFILETRANS_UTILS_H

#include"../config.h"

namespace UTILS{
    void base64_encode(const  char*in,  int inlen,char* out,  int &outlen);
    bool sendMail(const std::string& target,const std::string& subject,const std::string& body);
}

#endif //SECUREFILETRANS_UTILS_H
