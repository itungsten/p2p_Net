//
// Created by tungsten on 3/16/2021.
//

#ifndef SECUREFILETRANS_SM2TRANSLATOR_H
#define SECUREFILETRANS_SM2TRANSLATOR_H

#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/sm2.h>
#include <openssl/pem.h>
#include <openssl/is_gmssl.h>
#include <openssl/err.h>
#include "../config.h"
using namespace std;

class SM2Translator {
public:
    static EC_KEY* CreateEC(unsigned char* key, int is_public);
    static pair<string, string> GenKey(void);
    static string Encrypt(const string& public_key, const string& plain_text);
    static string Decrypt(const string& private_key, const string& enc_text);
};


#endif //SECUREFILETRANS_SM2TRANSLATOR_H
