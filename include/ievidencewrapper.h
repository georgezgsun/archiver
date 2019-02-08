#ifndef IEVIDENCEWRAPPER_H
#define IEVIDENCEWRAPPER_H

#include <datatypes/trackedobject.h>
#include <datatypes/evidence.h>

#ifdef _WIN32
    #if defined(ARCH_LIBRARY_EXPORT)
        #define ARCHAPI   __declspec(dllexport)
    #else
        #define ARCHAPI   __declspec(dllimport)
    #endif
#else
    #define ARCHAPI
#endif

class IEvidenceWrapper {

public:
    IEvidenceWrapper(std::string archiveName)
    {
        archiveNme = archiveName;
    }

    virtual ~IEvidenceWrapper() {}

    virtual SmartEye::TrackedObject getTrob() = 0;
    virtual SmartEye::EvidenceData getEvidenceData(std::string fileNameFromTrob) = 0;
};

ARCHAPI IEvidenceWrapper * GetEvidenceWrapper(std::string archiveName);

#endif // IEVIDENCEWRAPPER_H
