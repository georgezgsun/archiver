#include <iostream>
#include <string>
#include <list>
#include <string.h>

#include <openssl/aes.h>
#include <boost/uuid/sha1.hpp>

//#include <boost/thread.hpp>
//#include <boost/signals2.hpp>
//#include <boost/signals2/connection.hpp>
//#include <boost/signals2/signal.hpp>
//#include <boost/signals2/slot.hpp>

//#include "iarchiver.h"
//#include "archiverfile.h"
//#include <ilogger.h>
//#include <string>
//#include <string.h>
//#include <stdint.h>

class Archiver
{
private:
    char* bufferIn; //buffers for char*
    char* bufferOut;
    unsigned char* in; //buffers for unsigned char*
    unsigned char* out;

    std::string archiveFileName;

    size_t statCompressedPayload;
    size_t statPayloadSize;
    size_t statHeaderSize;

    uint32_t readCount;
    FILE* headerStart;

    size_t fileSizeLeft;
    size_t fileSizeLeft2;
    uint32_t headersSize;
    uint32_t seek;
    float progress;

    bool doCompression;
    bool doEncryption;

    boost::uuids::detail::sha1 sha;

    /*
    void parseMode();
    void parseFileNameSize();
    void parseFileName();
    void parseCompressedSize();
    void parseDataSize();
    void parseChecksum();
    bool checkChecksum();
    long readCompressedChunkSize(FILE* archive);
    long readDataChunkSize(FILE* archive);
    void readDataChunk(uint32_t size, uint32_t bytes, FILE* archive, unsigned char* data);
    */

public:
    Archiver(std::string nArchiveFilename);
    virtual ~Archiver();

    virtual int addFile(std::string filename, int mode);
    virtual int addPath(std::string path, int mode);
    virtual int loadFromArchive(std::string archive, std::string destPath = "");

    virtual std::string getArchiverFileName();

    virtual void readData(unsigned char* data);

    virtual int getMode();
    virtual uint32_t getFileNameSize();
    virtual std::string getFileName();

    virtual uint32_t getCompressedSize();
    virtual uint32_t getDataSize();
    virtual uint32_t getHeaderSize();

    virtual std::string getChecksum();
};
