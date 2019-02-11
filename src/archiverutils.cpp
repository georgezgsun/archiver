#include "archiverutils.h"
#include <openssl/aes.h>

#include <string.h>
#include <boost/uuid/sha1.hpp>
#include "lz4.h"
#include <dirent.h>

void ArchiverUtils::generateKey(unsigned char key[16], std::string fileName) {
    unsigned char hash[20];
    generateHash(hash, fileName);
    memcpy(key, hash, 16);
}


void ArchiverUtils::generateIV(unsigned char iv[16], std::string fileName) {
    unsigned char hash[20];
    generateHash(hash, fileName);
    memcpy(iv, hash + 4, 16);
}

void ArchiverUtils::encrypt(std::string pass, unsigned char * in, unsigned char * out, int len) {
    unsigned char ckey[16];
    unsigned char ivec[16];

    generateKey(ckey, pass);
    generateIV(ivec, pass);

    AES_KEY key;
    AES_set_encrypt_key(ckey, 128, &key);

    int num = 0;
    AES_cfb128_encrypt(in, out, static_cast<size_t>(len), &key, ivec, &num, AES_ENCRYPT);
}

void ArchiverUtils::decrypt(std::string pass, unsigned char * in, unsigned char *out, int len) {
    unsigned char ckey[16];
    unsigned char ivec[16];

    generateKey(ckey, pass);
    generateIV(ivec, pass);

    AES_KEY key;
    AES_set_encrypt_key(ckey, 128, &key);

    int num = 0;

    memset(out, 0, len);

    AES_cfb128_encrypt(in, out, static_cast<size_t>(len), &key, ivec, &num, AES_DECRYPT);

}

void ArchiverUtils::compress(unsigned char * in, unsigned char * out, int size) {
    LZ4_compress((const char *) in, (char *) out, size);
}

void ArchiverUtils::uncompress(unsigned char * in, unsigned char * out, int size) {
    LZ4_uncompress((const char *) in, (char *) out, size);
}


void ArchiverUtils::generateHash(unsigned char hash[20], std::string fileName) {
    unsigned int h[5];
    boost::uuids::detail::sha1 sha;
    sha.process_bytes((void *) fileName.c_str(), fileName.size());
    sha.get_digest(h);
    for (int i = 0; i < 5; i++) {
        unsigned char * b = (unsigned char *) &(h[i]);

        hash[i * 4] = b[0];
        hash[i * 4 + 1] = b[1];
        hash[i * 4 + 2] = b[2];
        hash[i * 4 + 3] = b[3];
    }
}
