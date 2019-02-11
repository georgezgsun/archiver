#include <assert.h>
#include <dirent.h>
#include <string>
#include <iostream>

#include "archiver.h"
#include "archiverutils.h"
#include "lz4.h"
#include "fileutils.h"

#define ARCHIVER_BUFFER_SIZE (1024 * 1024 * 16)
#define CHECKSUM_SIZE 20

/*
 * |                                        header                                       |                payload               |
 * | mode | long int file name size | file name | compressed size | data size | checksum | compressed chunk | data chunk | data |
 *
 */
using namespace std;

Archiver::Archiver(string narchiveFileName)
{
//    logger = GetLogger("Archiver");

//    cerr << "Openning " << archiveFileName << " archive file";
    archiveFileName = narchiveFileName;
    statCompressedPayload = 0;
    statPayloadSize = 0;
    statHeaderSize = 0;

    FILE* touch = fopen(archiveFileName.c_str(), "wb");
    assert(touch != NULL);
    fclose(touch);

    int len = LZ4_compressBound(ARCHIVER_BUFFER_SIZE);
    bufferIn = new char [len];
    bufferOut = new char [len];
    in = new unsigned char[len];
    out = new unsigned char[len];

    progress = 0;
}

Archiver::~Archiver() {
    delete [] bufferIn;
    delete [] bufferOut;
    delete [] in;
    delete [] out;
}

int Archiver::addFile(string fileName, int mode)
{
    cerr << "Adding file: " << fileName << " to archive: " << archiveFileName << " with mode " << mode;

    string fileKey = fileName;
    string archiveKey = archiveFileName;

    if (!isRegularFile(fileName))
    {
        cerr << fileName << " is not a regular file." << endl;
        return -1;
    }

    FILE* archive = fopen(archiveFileName.c_str(), "rwb+");
    if (!archive)
    {
        cerr<< "Cannot open archive file: " << archiveFileName << endl;
        return -2;
    }

    FILE* file = fopen(fileName.c_str(), "rb");
    if (!file)
    {
        cerr<< "Cannot open " << fileName << " to add to archive: " << endl;
        fclose(archive);
        return -3;
    }

    int ret = fseek(archive, 0, SEEK_END);
    if (ret != 0)
    {
        cerr<< "Cannot go to end of file: " << fileName << " with error code " << ret << endl;
        ret=-4;
        fclose(archive);
        fclose(file);
        return -4;
    }

    int64_t start = ftell(archive);
    cerr<< "Adding to archive at: " << start << endl;
    if (start < 0)
    {
        cerr<< "Cannot get end of file position: " << fileName << endl;
        fclose(archive);
        fclose(file);
        return -5;
    }

    uint32_t fileNameSize = static_cast<uint32_t>(fileName.size());
    uint32_t headerSize = sizeof(mode) + sizeof(fileNameSize) + fileNameSize + sizeof(uint32_t) + sizeof(uint32_t) + CHECKSUM_SIZE;
    cerr << "Header size " << headerSize << endl;

    // Jump to payload section
    ret = fseek(archive, headerSize, SEEK_CUR);
    if (ret != 0)
    {
        cerr<< "Cannot go to payload position: " << fileName << endl;
        fclose(archive);
        fclose(file);
        return -6;
    }

    bool doCompression = (mode & 1) == 0;
    bool doEncryption = (mode & 2) == 0;
    if (!doCompression && !doEncryption)
    {
        cerr<< "Mode specification " << mode << " is incorrect. It can only be 0(Compression and Encryption), 1(Comprission only), or 2(Encryption only)." << endl;
        fclose(archive);
        fclose(file);
        return -7;
    }

    int dataSize = 0;
    int compressedSize = 0;
    size_t written;

    unsigned char in[sizeof(bufferIn)];
    unsigned char out[sizeof(bufferOut)];

    while (feof(file) == 0)
    {
        size_t bytes = fread(bufferIn, 1, ARCHIVER_BUFFER_SIZE, file);
        sha.process_bytes(bufferIn, bytes);
        dataSize += bytes;
        statPayloadSize += bytes;

        int size;

        if (doCompression)
        {
            size = LZ4_compress(bufferIn, bufferOut, static_cast<int>(bytes));
            statCompressedPayload += static_cast<uint32_t>(size);
        }
        else
        {
            memcpy(bufferOut, bufferIn, bytes);
            statCompressedPayload += bytes;
            size = static_cast<int>(bytes);
        } 

        // process the payload/compressed chunk
        if (doEncryption)
        {
            memcpy(in, &size, sizeof(size));
            ArchiverUtils::encrypt(getFilename(fileName), in, out, sizeof(size));
        }
        else
        {
            memcpy(out, &size, sizeof(size));
        }
        written = fwrite(out, 1, sizeof(size), archive);
        if (written != sizeof(size))
        {
            cerr<< "Cannot write to file: " << archiveFileName << endl;
            fclose(archive);
            fclose(file);
            return -8;
        }
        compressedSize += sizeof(size);
        statHeaderSize += sizeof(size);

        // process the payload/data chunk
        if (doEncryption)
        {
            memcpy(in, &bytes, sizeof(bytes));
            ArchiverUtils::encrypt(getFilename(fileName), in, out, sizeof(bytes));
        } else
        {
            memcpy(out, &bytes, sizeof(bytes));
        }
        written = fwrite(out, 1, sizeof(bytes), archive);
        if (written != sizeof(bytes))
        {
            cerr<< "Cannot write to file: " << archiveFileName << endl;
            fclose(archive);
            fclose(file);
            return -8;
        }
        compressedSize += sizeof(bytes);
        statHeaderSize += sizeof(bytes);

        // process the payload/data
        if (doEncryption)
        {
            bytes = static_cast<size_t>(size);
            memcpy(in, bufferOut, bytes);
            ArchiverUtils::encrypt(getFilename(fileName), in, out, size);
        }
        else
        {
            memcpy(out, bufferOut, bytes);
        }
        written = fwrite(out, 1, bytes, archive);
        if (written != bytes)
        {
            cerr<< "Cannot write to file: " << archiveFileName << endl;
            fclose(archive);
            fclose(file);
            return -8;
        }
        compressedSize += bytes;
    }

    cerr << "Compressed size: " << compressedSize << endl;
    cerr << "Data size: " << dataSize << endl;

    // process the header
    ret = fseek(archive, start, SEEK_SET);
    if (ret != 0)
    {
        cerr << "Cannot go to start position: " << fileName << endl;
        fclose(archive);
        fclose(file);
        return -9;
    }

    unsigned int checksum[5];
    sha.get_digest(checksum);

    // process the header:mode
    if (doEncryption)
    {
        memcpy(in, &mode, sizeof (mode));
        ArchiverUtils::encrypt(getFilename(archiveFileName), in, out, sizeof(mode));
    }
    else
    {
        memcpy(out, &mode, sizeof (mode));
    }
    written = fwrite(out, 1, sizeof(mode), archive);
    statHeaderSize += written;
    if (written != sizeof(mode))
    {
        cerr<< "Cannot write header:mode to file: " << archiveFileName << endl;
        fclose(archive);
        fclose(file);
        return -10;
    }

    // process the header:size of filename
    if (doEncryption)
    {
        memcpy(in, &mode, sizeof (fileNameSize));
        ArchiverUtils::encrypt(getFilename(archiveFileName), in, out, sizeof(fileNameSize));
    }
    else
    {
        memcpy(out, &fileNameSize, sizeof(fileNameSize));
    }
    written = fwrite(out, 1, sizeof(fileNameSize), archive);
    statHeaderSize += written;
    if (written != sizeof(fileNameSize))
    {
        cerr<< "Cannot write header:size of filename to file: " << archiveFileName << endl;
        fclose(archive);
        fclose(file);
        return -11;
    }

    //process the header:filename
    if (doEncryption)
    {
        ArchiverUtils::encrypt(getFilename(archiveFileName), (unsigned char*) fileName.c_str(), out, static_cast<int>(fileNameSize));
    }
    else
    {
        memcpy(out, fileName.c_str(), fileNameSize);
    }
    written = fwrite(out, 1, fileNameSize, archive);
    statHeaderSize += written;
    if (written != fileNameSize)
    {
        cerr << "Cannot write header:filename to file: " << archiveFileName << endl;
        fclose(archive);
        fclose(file);
        return -12;
    }

    // process the header:compressed size
    if (doEncryption)
    {
        memcpy(in, &compressedSize, sizeof (fileNameSize));
        ArchiverUtils::encrypt(getFilename(archiveFileName), in, out, sizeof(compressedSize));
    }
    else
    {
        memcpy(out, &compressedSize, sizeof(compressedSize));
    }
    written = fwrite(out, 1, sizeof(compressedSize), archive);
    statHeaderSize += written;
    if (written != sizeof(compressedSize))
    {
        cerr << "Cannot write to file: " << archiveFileName << endl;
        fclose(archive);
        fclose(file);
        return -13;
    }

    // process header:data size
    if (doEncryption)
    {
        memcpy(in, &mode, sizeof (dataSize));
        ArchiverUtils::encrypt(getFilename(archiveFileName), in, out, sizeof(dataSize));
    }
    else
    {
        memcpy(out, &dataSize, sizeof(dataSize));
    }
    written = fwrite(out, 1, sizeof(dataSize), archive);
    statHeaderSize += written;
    if (written != sizeof(dataSize))
    {
        cerr << "Cannot write header:data size to file: " << archiveFileName << endl;
        fclose(archive);
        fclose(file);
        return -14;
    }

    // process header:checksum
    if (doEncryption)
    {
        memcpy(in, &checksum, CHECKSUM_SIZE);
        ArchiverUtils::encrypt(getFilename(archiveFileName), in, out, CHECKSUM_SIZE);
    }
    else
    {
        memcpy(out, &checksum, CHECKSUM_SIZE);
    }
    written = fwrite(out, 1, CHECKSUM_SIZE, archive);
    statHeaderSize += written;
    if (written != CHECKSUM_SIZE)
    {
        cerr << "Cannot write header:checksum to file: " << archiveFileName << endl;
        fclose(archive);
        fclose(file);
        return -15;
    }

    cerr << "Stats: payload size: " << statPayloadSize << ", compressed payload size: " << statCompressedPayload << ", headers size: "<< statHeaderSize;

    fclose(file);
    fclose(archive);

    return 0;

}

int  Archiver::addPath(string path, int mode)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        std::cerr << "Error: " << path << " is not a directory. " << endl;
        return -1;
    }

    int num = 0;
    struct dirent * ent;
    while ((ent = readdir(dir)) && isRegularFile(ent->d_name))
    {
        num ++;
        if (addFile(ent->d_name, mode) < 0)
        {
            cerr << "Cannot add " << ent->d_name << " to archive: " << archiveFileName << endl;
            return -2;
        }
    }

    return 0;
}

int loadFromArchive(string archiveFile, string destPath = "")
{
    if (!isRegularFile(archiveFile))
    {
        cerr << "Cannot find archive file " << archiveFile << endl;
        return -1;
    }

    FILE* archive = fopen(archiveFile.c_str(), "rb");
    if (!archive)
    {
        cerr<< "Cannot open archive file " << archiveFile << endl;
        return -2;
    }

    int ret = fseek(archive, 0 , SEEK_END);
    if (ret != 0) {
        cerr<< "Cannot go to end position of " << archiveFile << " with error code " << ret << endl;
        fclose(archive);
        return -3;
    }

    int64_t end = ftell(archive);
    if (end < 0) {
        cerr<< "Cannot get end of file position of " << archiveFile << " with error code " << end << endl;
        fclose(archive);
        return -4;
    }

    rewind(archive);
    fclose(archive);

    if (!isDirectory(destPath))
    {
        cerr<< destPath << " is not a valid directory." << endl;
        return -5;
    }

    uint32_t cur = 0;
    while(cur < end)
    {
        //FILE* file;

        cur += end;
    }

    return 0;
}

