#include "archiver.h"
#include "archiverutils.h"

#include <assert.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/uuid/sha1.hpp>

#include "lz4.h"
#include "watchdog.h"

#define ARCHIVER_BUFFER_SIZE (1024 * 1024 * 16)
#define CHECKSUM_SIZE 20

/*
 * |                                        header                                       |                payload               |
 * | mode | long int file name size | file name | compressed size | data size | checksum | compressed chunk | data chunk | data |
 *
 */

Archiver::Archiver(std::string archiveFileName):
    IArchiver(archiveFileName),
    archiveFileName(archiveFileName),
    statCompressedPayload(0),
    statPayloadSize(0),
    statHeaderSize(0)
{
    logger = GetLogger("Archiver");

    logger->normal() << "Openning " << archiveFileName << " archive file";

    FILE* touch = fopen(archiveFileName.c_str(), "wb");
    assert(touch != NULL);

    fclose(touch);

    bufferIn = new unsigned char [LZ4_compressBound(ARCHIVER_BUFFER_SIZE)];
    bufferOut = new unsigned char [LZ4_compressBound(ARCHIVER_BUFFER_SIZE)];

    progress = 0;

}

Archiver::~Archiver() {
    delete [] bufferIn;
    delete [] bufferOut;
    delete logger;
}

int Archiver::addFile(std::string fileName, IArchiverFile::Mode mode) {
    logger->normal() << "Adding file: " << fileName << " to archive: " << archiveFileName << " with mode " << (int) mode;

    boost::filesystem::path fileKey(fileName);
    boost::filesystem::path archiveKey(archiveFileName);

    if (!boost::filesystem::is_regular_file(boost::filesystem::status(fileKey)) ||
        boost::filesystem::is_symlink(boost::filesystem::symlink_status(fileKey))) {
        logger->error() << "Cannot add not regular file: " << fileName;
        return -1;
    }

    FILE* archive = fopen(archiveFileName.c_str(), "rwb+");
    if (archive == NULL) {
        logger->error() << "Cannot open archive file: " << archiveFileName;
        return -1;
    }

    FILE* file = fopen(fileName.c_str(), "rb");
    if (file == NULL) {
        logger->error() << "Cannot open file to add to archive: " << fileName;
        fclose(archive);
        return -1;
    }

    int ret = fseek(archive, 0, SEEK_END);
    if (ret != 0) {
        logger->error() << "Cannot go to end of file: " << fileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    int64_t start = ftell(archive);
    logger->normal() << "Adding to archive at: " << start;
    if (start < 0) {
        logger->error() << "Cannot get end of file position: " << fileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    uint32_t fileNameSize = fileName.size();
    uint32_t headerSize = sizeof(IArchiverFile::Mode) + sizeof(fileNameSize) + fileNameSize + sizeof(uint32_t) + sizeof(uint32_t) + CHECKSUM_SIZE;
    logger->normal() << "Header size " << headerSize;

    ret = fseek(archive, headerSize, SEEK_CUR);
    if (ret != 0) {
        logger->error() << "Cannot go to payload position: " << fileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    uint32_t dataSize = 0;
    uint32_t compressedSize = 0;

    boost::uuids::detail::sha1 sha;

    bool doCompression = (mode == IArchiverFile::FULL || mode == IArchiverFile::COMP_ONLY);
    bool doEncryption = (mode == IArchiverFile::FULL || mode == IArchiverFile::ENC_ONLY);

    while (feof(file) == 0) {
        uint32_t bytes = fread(bufferIn, 1, ARCHIVER_BUFFER_SIZE, file);
        sha.process_bytes(bufferIn, bytes);
        dataSize += bytes;
        statPayloadSize += bytes;

        uint32_t size;
        uint32_t written;

        if (doCompression) {
            size = LZ4_compress((char*) bufferIn, (char*) bufferOut, bytes);
        } else {
            memcpy(bufferOut, bufferIn, bytes);
            size = bytes;
        }
        statCompressedPayload += size;

        if (doEncryption) {
            ArchiverUtils::encrypt(fileKey.leaf().string(), (unsigned char*) &size, bufferIn, sizeof(uint32_t));
        } else {
            memcpy(bufferIn, (unsigned char*) &size, sizeof(uint32_t));
        }
        written = fwrite(bufferIn, 1, sizeof(uint32_t), archive);
        if (written != sizeof(uint32_t)) {
            logger->error() << "Cannot write to file: " << archiveFileName;
            fclose(archive);
            fclose(file);
            return -1;
        }
        compressedSize += sizeof(uint32_t);
        statHeaderSize += sizeof(uint32_t);


        if (doEncryption) {
            ArchiverUtils::encrypt(fileKey.leaf().string(), (unsigned char*) &bytes, bufferIn, sizeof(uint32_t));
        } else {
            memcpy(bufferIn, (unsigned char*) &bytes, sizeof(uint32_t));
        }
        written = fwrite(bufferIn, 1, sizeof(uint32_t), archive);
        if (written != sizeof(uint32_t)) {
            logger->error() << "Cannot write to file: " << archiveFileName;
            fclose(archive);
            fclose(file);
            return -1;
        }
        compressedSize += sizeof(uint32_t);
        statHeaderSize += sizeof(uint32_t);

        if (doEncryption) {
            ArchiverUtils::encrypt(fileKey.leaf().string(), bufferOut, bufferIn, size);
        } else {
            memcpy(bufferIn, bufferOut, size);
        }
        written = fwrite(bufferIn, 1, size, archive);
        if (written != size) {
            logger->error() << "Cannot write to file: " << archiveFileName;
            fclose(archive);
            fclose(file);
            return -1;
        }
        compressedSize += size;
    }

    logger->normal() << "Compressed size: " << compressedSize;
    logger->normal() << "Data size: " << dataSize;

    ret = fseek(archive, start, SEEK_SET);
    if (ret != 0) {
        logger->error() << "Cannot go to start position: " << fileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    unsigned int checksum[5];
    sha.get_digest(checksum);

    uint32_t written;

    ArchiverUtils::encrypt(archiveKey.leaf().string(), (unsigned char *) &mode, bufferOut, sizeof(mode));
    written = fwrite(bufferOut, 1, sizeof(mode), archive);
    statHeaderSize += written;
    if (written != sizeof(mode)) {
        logger->error() << "Cannot write to file: " << archiveFileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    if (doEncryption) {
        ArchiverUtils::encrypt(archiveKey.leaf().string(), (unsigned char *) &fileNameSize, bufferOut, sizeof(fileNameSize));
    } else {
        memcpy(bufferOut, (unsigned char *) &fileNameSize, sizeof(fileNameSize));
    }
    written = fwrite(bufferOut, 1, sizeof(fileNameSize), archive);
    statHeaderSize += written;
    if (written != sizeof(fileNameSize)) {
        logger->error() << "Cannot write to file: " << archiveFileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    if (doEncryption) {
        ArchiverUtils::encrypt(archiveKey.leaf().string(), (unsigned char*) fileName.c_str(), bufferOut, fileNameSize);
    } else {
        memcpy(bufferOut, (unsigned char*) fileName.c_str(), fileNameSize);
    }
    written = fwrite(bufferOut, 1, fileNameSize, archive);
    statHeaderSize += written;
    if (written != fileNameSize) {
        logger->error() << "Cannot write to file: " << archiveFileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    if (doEncryption) {
        ArchiverUtils::encrypt(archiveKey.leaf().string(), (unsigned char*) &compressedSize, bufferOut, sizeof(compressedSize));
    } else {
        memcpy(bufferOut, (unsigned char*) &compressedSize, sizeof(compressedSize));
    }
    written = fwrite(bufferOut, 1, sizeof(compressedSize), archive);
    statHeaderSize += written;
    if (written != sizeof(compressedSize)) {
        logger->error() << "Cannot write to file: " << archiveFileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    if (doEncryption) {
        ArchiverUtils::encrypt(archiveKey.leaf().string(), (unsigned char*) &dataSize, bufferOut, sizeof(dataSize));
    } else {
        memcpy(bufferOut, (unsigned char*) &dataSize, sizeof(dataSize));
    }
    written = fwrite(bufferOut, 1, sizeof(dataSize), archive);
    statHeaderSize += written;
    if (written != sizeof(dataSize)) {
        logger->error() << "Cannot write to file: " << archiveFileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    if (doEncryption) {
        ArchiverUtils::encrypt(archiveKey.leaf().string(), (unsigned char*) &checksum, bufferOut, CHECKSUM_SIZE);
    } else {
        memcpy(bufferOut, (unsigned char*) &checksum, CHECKSUM_SIZE);
    }
    written = fwrite(bufferOut, 1, CHECKSUM_SIZE, archive);
    statHeaderSize += written;
    if (written != CHECKSUM_SIZE) {
        logger->error() << "Cannot write to file: " << archiveFileName;
        fclose(archive);
        fclose(file);
        return -1;
    }

    logger->normal() << "Stats: payload size: " << statPayloadSize << ", compressed payload size: " << statCompressedPayload << ", headers size: "<< statHeaderSize;

    fclose(file);
    fclose(archive);

    return 0;
}

int  Archiver::addPath(std::string path, IArchiverFile::Mode mode) {

    logger->normal() << "Adding path: " << path;

    boost::filesystem::path dir = boost::filesystem::absolute(path);
    boost::filesystem::recursive_directory_iterator end;

    logger->normal() << "Absolute path: " << dir.string();

    int num = 0;
    if (boost::filesystem::exists(dir)) {
        for (boost::filesystem::recursive_directory_iterator start(dir); start != end; start++) {
            if (boost::filesystem::is_regular_file(start->status())) {
                num++;
            }
        }
    }

    logger->normal() << "Number of files to add: " << num;

    for (boost::filesystem::recursive_directory_iterator start(dir); start != end; start++) {
        if (SmartEye::Watchdog::retrieveCloseSignal()) {
            logger->warning() << "Got close signal. Skipping archiving";
            return -1;
        }
        logger->normal() << "Looking at: " << start->path().string();

        if (boost::filesystem::is_regular_file(start->status()) && !boost::filesystem::is_symlink(start->symlink_status())) {
            int ret = addFile(std::string(start->path().string()), mode);
            if (ret != 0) {
                logger->error() << "Cannot add file to archive: " << start->path().string();
                return -1;
            }

            mutex.lock();
            progress = progress + (1.0 / (float) num);
            progressSignal(progress);
            mutex.unlock();
        }
    }

    return 0;
}

int IArchiver::loadFromArchive(std::string archiveFileName, std::list<IArchiverFile*>* list) {
    ILogger* logger = GetLogger("IArchiver");

    FILE* archive = fopen(archiveFileName.c_str(), "rb");
    if (archive == NULL) {
        logger->error() << "Cannot open archive file: " << archiveFileName;
        delete logger;
        return -1;
    }

    int ret = fseek(archive, 0 , SEEK_END);
    if (ret != 0) {
        logger->error() << "Cannot go to end position: " << archiveFileName;
        fclose(archive);
        delete logger;
        return -1;
    }

    int64_t end = ftell(archive);
    if (end < 0) {
        logger->error() << "Cannot get end of file position: " << archiveFileName;
        fclose(archive);
        delete logger;
        return -1;
    }

    rewind(archive);
    fclose(archive);

    uint32_t cur = 0;
    while(cur < end) {
        ArchiverFile* file;

        try {
            file = new ArchiverFile(archiveFileName, cur);
        } catch (...) {

            std::list<IArchiverFile*>::iterator it;
            for (it = list->begin(); it != list->end(); it++) {
                IArchiverFile* file = (*it);
                delete file;
            }

            delete logger;
            return -1;
        }

        list->push_front(file);
        cur += file->getCompressedSize() + file->getHeaderSize();
    }

    delete logger;
    return 0;
}

boost::signals2::connection Archiver::connectProgressSlot(const IArchiver::ProgressSlotType& slot) {
    return progressSignal.connect(slot);
}

float Archiver::getProgress() {
    float p;

    mutex.lock();
    p = progress;
    mutex.unlock();

    return p;
}
