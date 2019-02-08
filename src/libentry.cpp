#include <iarchiver.h>
#include <iautoeconomy.h>
#include <ievidencewrapper.h>

#include "archiver.h"
#include "autoeconomy.h"
#include "evidencewrapper.h"

ARCHAPI IArchiver* GetArchiver(std::string archiveFileName) {
    return (IArchiver*) new Archiver(archiveFileName);
}

ARCHAPI IAutoEconomy* GetAutoEconomy(std::string evidencePath, IAutoEconomy::Params params) {
    return (IAutoEconomy*) new AutoEconomy(evidencePath, params);
}

ARCHAPI IEvidenceWrapper* GetEvidenceWrapper(std::string archiveName) {
    return (IEvidenceWrapper*) new EvidenceWrapper(archiveName);
}

