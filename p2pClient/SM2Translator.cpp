//
// Created by tungsten on 3/16/2021.
//

#include "SM2Translator.h"
#include "../config.h"
using namespace std;

EC_KEY* SM2Translator::CreateEC(unsigned char* key, int is_public)
{
    EC_KEY *ec_key = NULL;
    BIO *keybio = NULL;
    keybio = BIO_new_mem_buf(key, -1);

    if (keybio==NULL) {
        cout << "Failed to Get Key" << endl;
        exit(1);
    }

    if(is_public) {
        ec_key = PEM_read_bio_EC_PUBKEY(keybio, NULL, NULL, NULL);
    }
    else {
        ec_key = PEM_read_bio_ECPrivateKey(keybio, NULL, NULL, NULL);
    }

    if(ec_key == NULL) {
        cout << "Failed to Get Key" << endl;
        exit(1);
    }

    return ec_key;
}

pair<string, string> SM2Translator::GenKey(void)
{
    EC_KEY *keypair = NULL;
    EC_GROUP *group1 = NULL;

    keypair = EC_KEY_new();
    if(!keypair) {
        cout << "Failed to Gen Key" << endl;
        exit(1);
    }

    group1 = EC_GROUP_new_by_curve_name(NID_sm2p256v1);
    if(group1 == NULL){
        cout << "Failed to Gen Key" << endl;
        exit(1);
    }

    int ret1 = EC_KEY_set_group(keypair, group1);
    if(ret1 != 1){
        cout << "Failed to Gen Key" << endl;
        exit(1);
    }

    int ret2 = EC_KEY_generate_key(keypair);
    if(ret2 != 1){
        cout << "Failed to Gen Key" << endl;
        exit(1);
    }

    size_t pri_len;
    size_t pub_len;
    char *pri_key = NULL;
    char *pub_key = NULL;

    BIO *pri = BIO_new(BIO_s_mem());
    BIO *pub = BIO_new(BIO_s_mem());

    PEM_write_bio_ECPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
    PEM_write_bio_EC_PUBKEY(pub, keypair);

    pri_len = BIO_pending(pri);
    pub_len = BIO_pending(pub);

    pri_key = new char[pri_len + 1];
    pub_key = new char[pub_len + 1];

    BIO_read(pri, pri_key, pri_len);
    BIO_read(pub, pub_key, pub_len);

    pri_key[pri_len] = '\0';
    pub_key[pub_len] = '\0';

    string public_key = pub_key;
    string private_key = pri_key;

    EC_KEY_free(keypair);
    BIO_free_all(pub);
    BIO_free_all(pri);
    delete [] pri_key;
    delete [] pub_key;

    return std::pair<string, string>(public_key, private_key);
}
string SM2Translator::Encrypt(const string& public_key, const string& plain_text)
{
    unsigned char encrypted[MAX_SIZE] = {};

    EC_KEY *rsa = CreateEC((unsigned char*)public_key.c_str(), 1);
    size_t encrypted_length = MAX_SIZE;
    int ret = SM2_encrypt_with_recommended((unsigned char*)plain_text.c_str(), plain_text.length(),
                                           (unsigned char*)encrypted,&encrypted_length, rsa);

    if (ret == 0) {
        cout << "Failed to Encrypt" << endl;
        exit(1);
    }

    string enc_text((char*)encrypted, encrypted_length);
    return enc_text;
}
string SM2Translator::Decrypt(const string& private_key, const string& enc_text)
{
    unsigned char decrypted[MAX_SIZE] = {};

    EC_KEY * key1 = CreateEC((unsigned char*)private_key.c_str(), 0);

    size_t decrypted_length = 0;
    int ret = SM2_decrypt_with_recommended((unsigned char*)enc_text.c_str(), enc_text.length(), decrypted, &decrypted_length, key1);

    if (ret == 0) {
        cout << "Failed to Decrypt" << endl;
        exit(1);
    }

    string plain_text((char*)decrypted, decrypted_length);
    return plain_text;
}
