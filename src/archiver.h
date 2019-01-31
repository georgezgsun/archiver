#ifndef ARCHIVER_H
#define ARCHIVER_H

#ifndef OF
#define OF(x) x
#endif

#ifndef _Z_OF
#define _Z_OF(x) x
#endif

#include <iostream>
#include <string>
#include <list>

#include <openssl/aes.h>

#include <boost/thread.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/slot.hpp>

#include <datatypes.h>
#include <iarchiver.h>

#include "archiverfile.h"

#include <ilogger.h>

class Archiver: public IArchiver {

private:
    ILogger* logger;

    unsigned char* bufferIn;
    unsigned char* bufferOut;

    std::list<ArchiverFile> files;
    std::string archiveFileName;

    boost::mutex mutex;

    float progress;

    boost::signals2::signal<void(float)> progressSignal;

    size_t statCompressedPayload;
    size_t statPayloadSize;
    size_t statHeaderSize;

public:
    Archiver(std::string nArchiveFileName);
    virtual ~Archiver();

    virtual int addPath(std::string path, IArchiverFile::Mode mode = IArchiverFile::FULL);
    virtual int addFile(std::string fileName, IArchiverFile::Mode mode = IArchiverFile::FULL);

    virtual float getProgress();
    virtual boost::signals2::connection connectProgressSlot(const IArchiver::ProgressSlotType& slot);

};

#endif
