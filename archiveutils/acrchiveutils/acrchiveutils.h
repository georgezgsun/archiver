#ifndef ACRCHIVEUTILS_H
#define ACRCHIVEUTILS_H

#include <string>
using namespace std;

class ArchiverUtils
{

public:

    static void generateHash(unsigned char hash[20], string fileName);
    static void generateKey(unsigned char key[16], string fileName);
    static void generateIV(unsigned char iv[16], string fileName);
    static void encrypt(string pass, unsigned char * in, unsigned char * out, int len);
    static void decrypt(string pass, unsigned char * in, unsigned char * out, int len);
    static void compress(unsigned char * in, unsigned char * out, int size);
    static void uncompress(unsigned char * in, unsigned char * out, int size);

};

#endif // ACRCHIVEUTILS_H
