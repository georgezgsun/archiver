#ifndef ARCHIVERUTILS_H
#define ARCHIVERUTILS_H

#include <openssl/aes.h>
#include <string>
#include <string.h>


class ArchiverUtils {

public:

    static void generateHash(unsigned char hash[20], std::string fileName);
    static void generateKey(unsigned char key[16], std::string fileName);
    static void generateIV(unsigned char iv[16], std::string fileName);
    static void encrypt(std::string pass, unsigned char * in, unsigned char * out, int len);
    static void decrypt(std::string pass, unsigned char * in, unsigned char * out, int len);
    static void compress(unsigned char * in, unsigned char * out, int size);
    static void uncompress(unsigned char * in, unsigned char * out, int size);

};

#endif // ARCHIVERUTILS_H
