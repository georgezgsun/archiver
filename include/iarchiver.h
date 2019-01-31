#ifndef IARCHIVER_H
#define IARCHIVER_H

#include <string>
#include <iarchiverfile.h>

#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/slot.hpp>

#ifdef _WIN32
    #if defined(ARCH_LIBRARY_EXPORT)
        #define ARCHAPI   __declspec(dllexport)
    #else
        #define ARCHAPI   __declspec(dllimport)
    #endif
#else
    #define ARCHAPI
#endif

class IArchiver {

public:

    typedef boost::signals2::signal<void(float)> ProgressSlot;
    typedef ProgressSlot::slot_type ProgressSlotType;

    IArchiver(std::string archiveFileName) { archiveFileName = archiveFileName; }
    virtual ~IArchiver() {}

    virtual int addFile(std::string fileName, IArchiverFile::Mode mode = IArchiverFile::FULL) = 0;
    virtual int addPath(std::string path, IArchiverFile::Mode mode = IArchiverFile::FULL) = 0;

    virtual float getProgress() = 0;
    virtual boost::signals2::connection connectProgressSlot(const IArchiver::ProgressSlotType& slot) = 0;

    static int loadFromArchive(std::string archiveFileName, std::list<IArchiverFile*>* list);
};

ARCHAPI IArchiver* GetArchiver(std::string archiveFileName);

#endif // IARCHIVER_H
