#ifndef ARCHIVERFILE_H
#define ARCHIVERFILE_H

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "archiverutils.h"
#include "iarchiverfile.h"

//#include <ilogger.h>

#include <boost/uuid/sha1.hpp>

class ArchiverFile: public IArchiverFile {

private:
//    ILogger* logger;
    uint32_t readCount;
    FILE* headerStart;

    size_t fileSizeLeft;
    size_t fileSizeLeft2;
    uint32_t headersSize;
    uint32_t seek;
    std::string archiveFileName;

    bool doCompression;
    bool doEncryption;

    IArchiverFile::Mode mode;
    uint32_t fileNameSize;
    std::string fileName;
    uint32_t compressedSize;
    uint32_t dataSize;
    IArchiverFile::Checksum checksum;

    boost::uuids::detail::sha1 sha;

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

public:
    ArchiverFile(std::string archiveFileName, uint32_t pos);
    virtual ~ArchiverFile();

    virtual std::string getArchiverFileName();

    virtual void readData(unsigned char* data);

    virtual IArchiverFile::Mode getMode();
    virtual uint32_t getFileNameSize();
    virtual std::string getFileName();

    virtual uint32_t getCompressedSize();
    virtual uint32_t getDataSize();
    virtual uint32_t getHeaderSize();

    virtual IArchiverFile::Checksum getChecksum();
};

#endif // ARCHIVERFILE_H
