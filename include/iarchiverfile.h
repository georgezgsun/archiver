#ifndef IARCHIVERFILE_H
#define IARCHIVERFILE_H

#include <string>
#include <string.h>
#include <stdint.h>

class IArchiverFile {

public:

    typedef struct Checksum_ {
        unsigned int digest[5];
        bool operator == (const Checksum_& val ) {
            if (memcmp(val.digest, digest, 20) == 0) {
                return true;
            }
            return false;
        }
    } Checksum;

    typedef enum {
        FULL,
        COMP_ONLY,
        ENC_ONLY,
        NONE
    } Mode;

    virtual ~IArchiverFile() {}

    virtual std::string getArchiverFileName() = 0;

    virtual void readData(unsigned char* data) = 0;

    virtual IArchiverFile::Mode getMode() = 0;
    virtual uint32_t getFileNameSize() = 0;
    virtual std::string getFileName() = 0;

    virtual uint32_t getCompressedSize() = 0;
    virtual uint32_t getDataSize() = 0;
    virtual uint32_t getHeaderSize() = 0;

    virtual IArchiverFile::Checksum getChecksum() = 0;
};

#endif // IARCHIVERFILE_H
