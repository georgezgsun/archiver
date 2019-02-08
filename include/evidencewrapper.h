#ifndef EVIDENCEWRAPPER_H
#define EVIDENCEWRAPPER_H

#include <list>
#include <map>

#include "archiver.h"

#include <datatypes/trackedobject.h>
#include <datatypes/evidence.h>

#include <ievidencewrapper.h>
#include <ilogger.h>

class EvidenceWrapper: public IEvidenceWrapper {

    ILogger* logger;

    SmartEye::TrackedObject trob;

    std::map<std::string, IArchiverFile*> evidences;

    bool isTrackedObject(std::string file);
    bool isMatchedFile(IArchiverFile* archiverFile, std::string fileNameFromTrob);

public:
    EvidenceWrapper(std::string archiveName);
    virtual ~EvidenceWrapper();

    virtual SmartEye::TrackedObject getTrob();
    virtual SmartEye::EvidenceData getEvidenceData(std::string fileNameFromTrob);
};

#endif // EVIDENCEWRAPPER_H
