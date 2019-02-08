//#include "archiverfile.h"
#include "archiver.h"
#include "lz4.h"

#include <boost/filesystem.hpp>

//#include <exceptions/exceptiontypes.h>

void ArchiverFile::parseMode() {
    unsigned char numBuffer[sizeof(uint32_t)];

    if (fileSizeLeft < sizeof(uint32_t)) {
        throw printf("File too small");
    }
    readCount = fread(numBuffer, 1, sizeof(uint32_t), headerStart);
    fileSizeLeft -= sizeof(uint32_t);

    boost::filesystem::path archiveKey(archiveFileName);
    uint32_t m;
    ArchiverUtils::decrypt(archiveKey.leaf().string(), (unsigned char *) numBuffer, (unsigned char *) &m, sizeof(uint32_t));
    mode = static_cast<IArchiverFile::Mode>(m);

    doCompression = (mode == IArchiverFile::FULL || mode == IArchiverFile::COMP_ONLY);
    doEncryption = (mode == IArchiverFile::FULL || mode == IArchiverFile::ENC_ONLY);

    headersSize += sizeof(uint32_t);
}

void ArchiverFile::parseFileNameSize() {
    unsigned char numBuffer[sizeof(uint32_t)];

    if (fileSizeLeft < sizeof(uint32_t)) {
        throw printf("File too small.");
    }
    readCount = fread(numBuffer, 1, sizeof(uint32_t), headerStart);
    fileSizeLeft -= sizeof(uint32_t);

    if (doEncryption) {
        boost::filesystem::path archiveKey(archiveFileName);
        ArchiverUtils::decrypt(archiveKey.leaf().string(), (unsigned char *) numBuffer, (unsigned char *) &fileNameSize, sizeof(uint32_t));
    } else {
        memcpy((unsigned char *) &fileNameSize, (unsigned char *) numBuffer, sizeof(uint32_t));
    }

    headersSize += sizeof(uint32_t);
}

void ArchiverFile::parseFileName() {
    if (fileSizeLeft < fileNameSize) {
        throw printf("File too small.");
    }

    char * fileNameTmp = new char[fileNameSize + 1];
    char * fileNameTmp2 = new char[fileNameSize + 1];

    readCount = fread(fileNameTmp2, 1, fileNameSize, headerStart);
    fileSizeLeft -= fileNameSize;

    if (doEncryption) {
        boost::filesystem::path archiveKey(archiveFileName);
        ArchiverUtils::decrypt(archiveKey.leaf().string(), (unsigned char *) fileNameTmp2, (unsigned char *) fileNameTmp, fileNameSize);
    } else {
        memcpy((unsigned char *) fileNameTmp, (unsigned char *) fileNameTmp2, fileNameSize);
    }

    fileNameTmp[fileNameSize] = '\0';

    fileName = std::string(fileNameTmp);

    delete [] fileNameTmp;
    delete [] fileNameTmp2;

    headersSize += fileNameSize;
}

void ArchiverFile::parseCompressedSize() {
    unsigned char numBuffer[sizeof(uint32_t)];

    if (fileSizeLeft < sizeof(uint32_t)) {
        throw printf("File too small.");
    }
    readCount = fread(numBuffer, 1, sizeof(uint32_t), headerStart);
    fileSizeLeft -= sizeof(uint32_t);

    if (doEncryption) {
        boost::filesystem::path archiveKey(archiveFileName);
        ArchiverUtils::decrypt(archiveKey.leaf().string(), (unsigned char *) numBuffer, (unsigned char *) &compressedSize, sizeof(uint32_t));
    } else {
        memcpy((unsigned char *) &compressedSize, (unsigned char *) numBuffer, sizeof(uint32_t));
    }

    headersSize += sizeof(uint32_t);
}

void ArchiverFile::parseDataSize() {
    unsigned char numBuffer[sizeof(uint32_t)];

    if (fileSizeLeft < sizeof(uint32_t)) {
        throw printf("File too small.");
    }
    readCount = fread(numBuffer, 1, sizeof(uint32_t), headerStart);
    fileSizeLeft -= sizeof(uint32_t);

    if (doEncryption) {
        boost::filesystem::path archiveKey(archiveFileName);
        ArchiverUtils::decrypt(archiveKey.leaf().string(), (unsigned char *) numBuffer, (unsigned char *) &dataSize, sizeof(uint32_t));
    } else {
        memcpy((unsigned char *) &dataSize, (unsigned char *) numBuffer, sizeof(uint32_t));
    }

    headersSize += sizeof(uint32_t);
}

void ArchiverFile::parseChecksum() {
    unsigned char numBuffer[20];

    if (fileSizeLeft < 20) {
        throw printf("File too small.");
    }
    readCount = fread(numBuffer, 1, 20, headerStart);
    fileSizeLeft -= 20;

    if (doEncryption) {
        boost::filesystem::path archiveKey(archiveFileName);
        ArchiverUtils::decrypt(archiveKey.leaf().string(), (unsigned char *) numBuffer, (unsigned char *) checksum.digest, 20);
    } else {
        memcpy((unsigned char *) checksum.digest, (unsigned char *) numBuffer, 20);
    }

    headersSize += 20;
}

ArchiverFile::ArchiverFile(std::string archiveFileName, uint32_t seek) :
    readCount(0),
    headersSize(0),
    seek(seek),
    archiveFileName(archiveFileName),
    fileNameSize(0),
    compressedSize(0),
    dataSize(0)
{
    logger = GetLogger("ArchiverFile");

    logger->normal() << archiveFileName << " " << seek;

    headerStart = fopen(archiveFileName.c_str(), "rb");

    fseek(headerStart, 0, SEEK_END);
    fileSizeLeft = ftell(headerStart);

    if (fileSizeLeft < seek) {
        fclose(headerStart);
        delete logger;
        throw printf("File too small.");
    }

    fseek(headerStart, seek, SEEK_SET);
    fileSizeLeft -= seek;

    try {
        parseMode();
        logger->normal() << "Mode: " << (int) mode;

        parseFileNameSize();
        logger->normal() << "File Name Size: " << fileNameSize;

        parseFileName();
        logger->normal() << "File Name: " << fileName;

        parseCompressedSize();
        logger->normal() << "Compressed Size: " << compressedSize;

        parseDataSize();
        logger->normal() << "Data Size: " << dataSize;

        parseChecksum();
    } catch (...) {
        fclose(headerStart);
        delete logger;
        throw printf("File too small.");
    }

    fclose(headerStart);

    if (fileSizeLeft < (size_t) compressedSize) {
        delete logger;
        throw printf("File too small.");
    }
}

ArchiverFile::~ArchiverFile() {
    delete logger;
}

bool ArchiverFile::checkChecksum() {
    IArchiverFile::Checksum checksumTmp;
    sha.get_digest(checksumTmp.digest);
    if (checksum == checksumTmp) {
        return true;
    }

    throw ARCHIVER_CHECKSUM_FAIL("Checksum fail");

    return false;
}

long ArchiverFile::readCompressedChunkSize(FILE * archive) {
    uint32_t size;
    unsigned char buffer[sizeof(uint32_t)];

    if (fileSizeLeft2 < sizeof(uint32_t)) {
        throw printf("File too small.");
    }
    readCount = fread(buffer, 1, sizeof(uint32_t), archive);
    fileSizeLeft2 -= sizeof(uint32_t);

    if (doEncryption) {
        boost::filesystem::path fileKey(fileName);
        ArchiverUtils::decrypt(fileKey.leaf().string(), (unsigned char *) buffer, (unsigned char *) &size, sizeof(uint32_t));
    } else {
        memcpy((unsigned char *) &size, (unsigned char *) buffer, sizeof(uint32_t));
    }

    return size;
}

long ArchiverFile::readDataChunkSize(FILE* archive) {
    uint32_t bytes;
    unsigned char buffer[sizeof(uint32_t)];

    if (fileSizeLeft2 < sizeof(uint32_t)) {
        throw printf("File too small.");
    }
    readCount = fread(buffer, 1, sizeof(uint32_t), archive);
    fileSizeLeft2 -= sizeof(uint32_t);

    if (doEncryption) {
        boost::filesystem::path fileKey(fileName);
        ArchiverUtils::decrypt(fileKey.leaf().string(), (unsigned char *) buffer, (unsigned char *) &bytes, sizeof(uint32_t));
    } else {
        memcpy((unsigned char *) &bytes, (unsigned char *) buffer, sizeof(uint32_t));
    }

    return bytes;
}

void ArchiverFile::readDataChunk(uint32_t size, uint32_t bytes, FILE * archive, unsigned char * data) {
    logger->normal() << "Read data chunk loop start";

    uint32_t size2 = (size > bytes) ? size : bytes;

    unsigned char * buffer = new unsigned char[size2];
    unsigned char * buffer2 = new unsigned char[size2];

    if (fileSizeLeft2 < size) {
        throw printf("File too small.");
    }
    readCount = fread(buffer, 1, size, archive);
    fileSizeLeft2 -= size;

    if (doEncryption) {
        boost::filesystem::path fileKey(fileName);
        ArchiverUtils::decrypt(fileKey.leaf().string(), buffer, buffer2, size);
    } else {
        memcpy(buffer2, buffer, size);
    }

    delete [] buffer;

    if (doCompression) {
        ArchiverUtils::uncompress(buffer2, data, bytes);
    } else {
        memcpy(data, buffer2, bytes);
    }

    delete [] buffer2;

    sha.process_bytes(data, bytes);

    logger->normal() << "Read data chunk loop stop";
}

void ArchiverFile::readData(unsigned char * data) {
    FILE * archive = fopen(archiveFileName.c_str(), "rb");

    fseek(archive, seek + headersSize, SEEK_SET);

    sha.reset();

    uint32_t bytesLeft = dataSize;

    fileSizeLeft2 = fileSizeLeft;

    while (bytesLeft > 0) {
        uint32_t size = readCompressedChunkSize(archive);

        uint32_t bytes = readDataChunkSize(archive);

        bytesLeft -= bytes;

        readDataChunk(size, bytes, archive, data);
        data += bytes;
    }

    fclose(archive);

    checkChecksum();
}

IArchiverFile::Mode ArchiverFile::getMode() {
    return mode;
}

uint32_t ArchiverFile::getFileNameSize() {
    return fileNameSize;
}

std::string ArchiverFile::getFileName() {
    return fileName;
}

uint32_t ArchiverFile::getCompressedSize() {
    return compressedSize;
}

uint32_t ArchiverFile::getDataSize() {
    return dataSize;
}

uint32_t ArchiverFile::getHeaderSize() {
    return headersSize;
}

std::string ArchiverFile::getArchiverFileName() {
    return archiveFileName;
}

IArchiverFile::Checksum ArchiverFile::getChecksum() {
    return checksum;
}
