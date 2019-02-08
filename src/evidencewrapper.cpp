#include "evidencewrapper.h"

#include <exceptions.h>
#include <sstream>

EvidenceWrapper::EvidenceWrapper(std::string archiveName):
    IEvidenceWrapper(archiveName)
{
    logger = GetLogger("EvidenceWrapper");
    logger->normal() << "Loading evidence " << archiveName;

    std::list<IArchiverFile*> fileList;

    int ret = IArchiver::loadFromArchive(archiveName, &fileList);
    if (ret != 0) {
        delete logger;
        throw EVIDENCE_WRAPPER_WRONG_FILE("Cannot read file list");
    }

    std::list<IArchiverFile*>::iterator trobIt;
    for (trobIt = fileList.begin(); trobIt != fileList.end(); trobIt++) {
        if (isTrackedObject((*trobIt)->getFileName())) {
            unsigned char * buffer = new unsigned char[(*trobIt)->getDataSize()];

            try {
                (*trobIt)->readData(buffer);
            } catch (...) {
                delete logger;
                throw EVIDENCE_WRAPPER_WRONG_FILE("Cannot read trob");
            }

            std::stringstream streamBuffer;

            streamBuffer.write((char *) buffer, (*trobIt)->getDataSize());

            delete [] buffer;
            streamBuffer >> trob;

            break;
        }
    }

    std::set<std::string>::iterator evidItFromTrob;
    for (evidItFromTrob = trob.evidPaths.begin(); evidItFromTrob != trob.evidPaths.end(); evidItFromTrob++) {
        std::list<IArchiverFile*>::iterator evidIt;
        for (evidIt = fileList.begin(); evidIt != fileList.end(); evidIt++) {
            if (isMatchedFile(*evidIt, *evidItFromTrob)) {
                evidences.insert(std::pair<std::string, IArchiverFile*>(*evidItFromTrob, *evidIt));
                fileList.erase(evidIt);
                break;
            }
        }
    }
    std::list<IArchiverFile*>::iterator evidIt;
    for (evidIt = fileList.begin(); evidIt != fileList.end(); evidIt++) {
        delete (*evidIt);
    }
}

EvidenceWrapper::~EvidenceWrapper() {

    std::map<std::string, IArchiverFile*>::iterator evidIt;
    for (evidIt = evidences.begin(); evidIt != evidences.end(); evidIt++) {
        delete evidIt->second;
    }

    delete logger;
}

SmartEye::TrackedObject EvidenceWrapper::getTrob() {
    return trob;
}

SmartEye::EvidenceData EvidenceWrapper::getEvidenceData(std::string fileNameFromTrob) {

    SmartEye::EvidenceData evidence;

    std::map<std::string, IArchiverFile*>::iterator it = evidences.find(fileNameFromTrob);
    if (it == evidences.end()) {
        logger->error() << "Cannot find chosen evid";
    }

    unsigned char * buffer = new unsigned char[it->second->getDataSize()];

    it->second->readData(buffer);

    std::stringstream streamBuffer;
    streamBuffer.write((char *) buffer, it->second->getDataSize());

    delete [] buffer;

    streamBuffer >> evidence;

    return evidence;
}

bool EvidenceWrapper::isTrackedObject(std::string file) {
    return (file.substr(file.size() - 5, 5) == ".trob");
}

bool EvidenceWrapper::isMatchedFile(IArchiverFile* archiverFile, std::string fileNameFromTrob) {
    std::string fromArch = archiverFile->getFileName();
    std::string fromTrob = fileNameFromTrob;

    if (fromArch.size() < (fromTrob.size())) return false;

    return (fromArch.substr(fromArch.size() - fromTrob.size(), fromTrob.size()) == fromTrob);
}
