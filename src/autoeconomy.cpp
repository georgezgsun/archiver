#include "autoeconomy.h"
#include <boost/filesystem.hpp>

void AutoEconomy::workerLoop() {
    _isFinished = false;
    try {
        boost::filesystem::remove_all(economyPath);
        boost::filesystem::create_directory(economyPath);
    } catch (const boost::filesystem::filesystem_error& ex) {
        log->error() << "Error during remove/create directory: " << ex.what();
    }
    readEvidences();
    if (trobFileName.empty() || evidenceList.empty()) {
        log->warning() << "Nothing to do";
        _isFinished = true;
        return;
    }

    if (count > 0) {
        fillImportantList();
    }

    for (std::list<std::string>::iterator it = evidenceList.begin(); it != evidenceList.end(); it++) {
        if (isImportant(*it) == true) {
            if (makeRawAsJpeg) {
                makeJpeg(*it, true);
            } else {
                makeHardlink(*it);
            }
        } else {
            makeJpeg(*it);
        }

        progress += 1.0 / (float) evidenceList.size();
        progressSignal(progress);

        boost::thread::yield();
        if (_abort == true) {
            try {
                boost::filesystem::remove_all(economyPath);
            } catch (...) {}
            _isFinished = true;
            return;
        }
    }


    try {
        std::ofstream file;
        file.open(std::string(economyPath + "/" + trobFileName).c_str(), std::ios_base::out);
        file << trob;
        file.close();
    } catch (const std::exception) {
        log->error() << "Fail to save trob file: " << std::string(economyPath + "/" + trobFileName);
    }

    _isFinished = true;
}

bool AutoEconomy::isImportant(std::string file) {
    std::set<std::string>::iterator candidate = importantList.find(file);

    return !(candidate == importantList.end());
}

void AutoEconomy::init(std::string evidencePath, unsigned int count, bool rawAsJpeg, int quality, int width, int height) {
    this->_isFinished = false;
    this->evidencePath = evidencePath;
    this->economyPath = evidencePath;
    this->width = width;
    this->height = height;
    this->progress = 0;
    this->normQualityLvl = quality;
    this->highQualityLvl = 90;
    this->_abort = false;
    this->count = count;
    this->makeRawAsJpeg = rawAsJpeg;

    economyPath.append("/economy");

    imgTool = GetImgTool();
    log = GetLogger("AutoEconomy");
}

AutoEconomy::AutoEconomy(std::string evidencePath, Params params):
    IAutoEconomy(evidencePath, params),
    imgTool(NULL)
{
    int quality[3] = {90, 50, 10};
    int res[3][2] = {{1, 1}, {2, 2}, {4, 4}};

    init(evidencePath, params.count, params.hqAsJpeg, quality[params.quality], res[params.resolution][0], res[params.resolution][1]);
}

AutoEconomy::~AutoEconomy() {
    if (imgTool != NULL) {
        delete imgTool;
    }
    delete log;

    if (workerThread != NULL) {
        if (workerThread->joinable()) {
            workerThread->join();
        }

        delete workerThread;
        workerThread = NULL;
    }
}

void AutoEconomy::start() {
    _isFinished = false;
    workerThread = new boost::thread(boost::bind(&AutoEconomy::workerLoop, this));
}

void AutoEconomy::abort() {
    _abort = true;
}

void AutoEconomy::clear() {
    boost::filesystem::remove_all(economyPath);
}

bool AutoEconomy::isFinished() {
    return _isFinished;
}

void AutoEconomy::waitForFinish() {
    while (_isFinished != true) {
        sleep(1);
    }
}

float AutoEconomy::getProgress() {
    return progress;
}

std::string AutoEconomy::getEconomyPath() {
    return economyPath;
}


void AutoEconomy::makeHardlink(std::string file) {
    if (boost::filesystem::exists(economyPath + "/" + file)) {
        return;
    }
    try {
        boost::filesystem::create_hard_link(evidencePath + "/" + file, economyPath + "/" + file);
    } catch (...) {
        return;
    }

    trob.evidPaths.insert(file);
}


void AutoEconomy::makeJpeg(std::string file, bool isHighQuality) {
    SmartEye::EvidenceData data;
    if (!FsTools::LoadEvidFile(evidencePath + "/" + file, data)) {
        log->error() << "Can't load evid: " << file;
        return;
    }

    SmartEye::Image imgRes;
    std::set<SmartEye::EvidSnapshot>::iterator snapIter;
    int qualityLvl = isHighQuality? highQualityLvl : normQualityLvl;

    imgRes.width = data.cameraData.frame.data.width;
    imgRes.height = data.cameraData.frame.data.height;

    if (!isHighQuality) {
        imgRes.width = imgRes.width / width;
        imgRes.height = imgRes.height / height;
    }

    imgRes.size = imgRes.width * imgRes.height * 3;
    imgRes.format = SmartEye::RGB;

    log->normal() << "Size: " << " " << imgRes.width << " " << imgRes.height << " " << imgRes.size;

    imgRes.buffer = new unsigned char[imgRes.size];


    /* Do your stuff here */

    try {
        imgTool->resize(&(data.cameraData.frame.data), &imgRes, false, NULL, SmartEye::IMGTool::InterLanczos);

        if (qualityLvl > 0) {
            delete [] data.cameraData.frame.data.buffer;
            data.cameraData.frame.data.buffer = 0;
            imgTool->encodeToJpegMemBuffer(&imgRes, &data.cameraData.frame.data, qualityLvl);
        } else {
            data.cameraData.frame.data = imgRes;
        }

        data.cameraData.frame.data.width = imgRes.width;
        data.cameraData.frame.data.height = imgRes.height;

        try {
            snapIter = trob.snapshots.find(SmartEye::EvidSnapshot(data.countersData.shownFrames));

            if (snapIter != trob.snapshots.end()) {
                snapIter->frameSpecs.quality         = qualityLvl;
                snapIter->frameSpecs.resolution.w    = imgRes.width;
                snapIter->frameSpecs.resolution.h    = imgRes.height;
            } else {
                log->warning() << "Cannot find snapshot for frame: " << data.countersData.shownFrames;
            }
        } catch (std::exception& ex) {
            log->warning() << "Error occured during update of snapshot details: " << ex.what();
        }

        std::ofstream f;
        f.open(std::string(economyPath + "/" + file).c_str(), std::ios_base::out);
        f << data;
        f.close();

        trob.evidPaths.insert(file);

    } catch (std::exception& ex) {
        log->error() << "Error occured during making jpeg file: " << ex.what();
    } catch (...) {
        log->error() << "Unknown error occured during making jpeg file";
    }

    if (data.cameraData.frame.data.buffer == imgRes.buffer && imgRes.buffer != nullptr) {
        delete[] data.cameraData.frame.data.buffer;
        data.cameraData.frame.data.buffer = nullptr;
    } else {
        if (imgRes.buffer != nullptr) {
            delete[] imgRes.buffer;
        }

        if (data.cameraData.frame.data.buffer != nullptr) {
            delete[] data.cameraData.frame.data.buffer;
            data.cameraData.frame.data.buffer = nullptr;
        }
    }
}

void AutoEconomy::readEvidences() {
    DIR* dir = nullptr;
    struct dirent* node = nullptr;

    if ((dir = opendir(evidencePath.c_str())) == nullptr) {
        return;
    }
    assert(dir != nullptr);

    while ((node = readdir(dir)) != nullptr) {
        const std::string filename(evidencePath + "/" + node->d_name);

        if (! isTrackedObject(filename)) {
            continue;
        }
        trobFileName = std::string(node->d_name);

        log->normal() << std::string(evidencePath + "/" + trobFileName);

        if (! FsTools::LoadTrobFile(std::string(evidencePath + "/" + trobFileName), trob, true)) {
            log->error() << "Error occured during read trob: " << trobFileName;
            continue;
        }

        for (std::set<std::string>::iterator start = trob.evidPaths.begin(); start != trob.evidPaths.end(); start++) {
            evidenceList.push_front(*start);
        }
        trob.evidPaths.clear();
        closedir(dir);
        return;
    }

}

void AutoEconomy::fillImportantList() {
    std::map<float, InputData> distanceMap;

    SmartEye::EvidenceData data;
    for (std::list<std::string>::iterator it = evidenceList.begin(); it != evidenceList.end(); it++) {

        try {
            std::ifstream stream;
            stream.open(std::string(evidencePath + "/" + *it).c_str(), std::ios::in | std::ios::binary);
            if (!stream.good()) {
                throw std::runtime_error("Can't open file");
            }

            try {
                boost::archive::text_iarchive iarch(stream);
                iarch >> BOOST_SERIALIZATION_NVP(data);
            } catch(...) {
                stream.clear();
                stream.seekg(0, std::ios::beg);
                boost::archive::xml_iarchive iarch(stream);
                iarch >> BOOST_SERIALIZATION_NVP(data);
            }
            stream.close();

        } catch (...) {
            log->error() << "Error occured during read evid: " << *it;
            continue;
        }
        assert(data.cameraData.frame.data.buffer == nullptr); // don't allocate

        SmartEye::Radar::RadarObject * rObj = nullptr;

        for (int i = 0; i < SmartEye::RadarObjectsCount; i++) {
            if (data.radarData.frame.data.radarObjects[i].id == trob.radarData.id) {
                rObj = &(data.radarData.frame.data.radarObjects[i]);
                break;
            }
        }

        if (rObj != nullptr) {
            InputData id;
            id.file = *it;
            id.distanceToPhotoLine = fabs(-rObj->zRange - (data.violationData.smartPhotoLines[rObj->lane]));
            id.isFlashed   = data.additionalData.withFlash;
            id.inPhotoLine =
                    (-rObj->zRange > (data.violationData.smartPhotoLines[rObj->lane] - data.violationData.photoLineOffset)) &&
                    (-rObj->zRange < (data.violationData.smartPhotoLines[rObj->lane] + data.violationData.photoLineOffset));

            distanceMap.insert(std::pair<float, InputData>(-rObj->zRange, id));
        }
    }

    {
        std::map<float, InputData>::reverse_iterator s = distanceMap.rbegin();
        std::map<float, InputData>::reverse_iterator t;
        while (s != distanceMap.rend()) {
            if (distanceMap.size() > count) {
                if (s->second.isFlashed) {
                    std::string str = s->second.file;
                    importantList.insert(str);
                    t = s++;
                    distanceMap.erase(t->first);
                    continue;
                }

                if (s->second.inPhotoLine == false) {
                    t = s++;
                    distanceMap.erase(t->first);
                } else {
                    s++;
                }
            } else {
                break;
            }
        }
    }

    {
        std::map<float, InputData>::iterator s = distanceMap.begin();
        std::map<float, InputData>::iterator t;
        while ( s != distanceMap.end()) {
            if (distanceMap.size() > count) {
                if (s->second.isFlashed) {
                    std::string str = s->second.file;
                    importantList.insert(str);
                    t = s++;
                    distanceMap.erase(t->first);
                    continue;
                }

                if (s->second.inPhotoLine == false) {
                    t = s++;
                    distanceMap.erase(t->first);
                } else {
                    s++;
                }
            } else {
                break;
            }
        }
    }

    std::map<float, std::string> closestToPhotoLine;
    for (std::map<float, InputData>::reverse_iterator s = distanceMap.rbegin(); s != distanceMap.rend(); s++) {
        closestToPhotoLine.insert(std::pair<float, std::string>(s->second.distanceToPhotoLine, s->second.file));
    }

    for (std::map<float, std::string>::iterator s = closestToPhotoLine.begin(); (importantList.size() < count) && (s != closestToPhotoLine.end()); s++) {
        importantList.insert(s->second);
    }
}

bool AutoEconomy::isTrackedObject(std::string file) {
    return (file.substr(file.size() - 5, 5) == ".trob");
}

boost::signals2::connection AutoEconomy::connectProgressSlot(const IAutoEconomy::ProgressSlotType& slot) {
    return progressSignal.connect(slot);
}
