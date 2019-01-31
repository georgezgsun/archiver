#ifndef AUTOECONOMY_H
#define AUTOECONOMY_H

#include <iostream>
#include <list>
#include <set>
#include <cstdlib>

#include <dirent.h>

#include <datatypes.h>
#include <interfaces/iimgtool.h>

#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include <iautoeconomy.h>
#include <ilogger.h>
#include <fstools.h>

class AutoEconomy: public IAutoEconomy {

private:
    typedef struct {
        std::string file;
        bool inPhotoLine;
        bool isFlashed;
        float distanceToPhotoLine;
    } InputData;

    bool _isFinished;

    IImgTool * imgTool;

    boost::thread * workerThread;
    std::string evidencePath;
    std::string economyPath;

    std::list<std::string> evidenceList;
    std::set<std::string> importantList;

    float progress;

    SmartEye::TrackedObject trob;
    std::string trobFileName;

    int width;
    int height;
    int normQualityLvl;
    int highQualityLvl;
    unsigned int count;
    bool makeRawAsJpeg;
    bool _abort;

    void workerLoop();
    bool isImportant(std::string file);
    void makeHardlink(std::string file);
    void makeJpeg(std::string file, bool isHighQuality = false);
    void readEvidences();
    void fillImportantList();
    bool isTrackedObject(std::string file);

    void init(std::string evidencePath, unsigned int count = 1, bool RAWasJPEG = false, int normQualityLvl = 90, int width = -1, int height = -1);

    boost::signals2::signal<void (float)> progressSignal;

    ILogger* log;

public:
    AutoEconomy(std::string evidencePath, IAutoEconomy::Params params);
    virtual ~AutoEconomy();

    virtual void start();
    virtual void abort();
    virtual void clear();
    virtual bool isFinished();
    virtual void waitForFinish();
    virtual float getProgress();
    virtual std::string getEconomyPath();
    virtual boost::signals2::connection connectProgressSlot(const IAutoEconomy::ProgressSlotType& slot);
};

#endif // AUTOECONOMY_H
