#ifndef IAUTOECONOMY_H
#define IAUTOECONOMY_H

#include <string>
#include <boost/signals2.hpp>

#ifdef _WIN32
    #if defined(ARCH_LIBRARY_EXPORT)
        #define ARCHAPI __declspec(dllexport)
    #else
        #define ARCHAPI __declspec(dllimport)
    #endif
#else
    #define ARCHAPI
#endif

class IAutoEconomy {

public:
    typedef struct {
        enum Resolution {
            FULL,
            HALF,
            QUARTER
        };

        enum Quality {
            HIGH,
            MEDIUM,
            LOW
        };

        Resolution resolution;
        Quality quality;
        unsigned int count;
        bool enabled;
        bool hqAsJpeg;

    } Params;

    typedef boost::signals2::signal<void(float)> ProgressSlot;
    typedef ProgressSlot::slot_type ProgressSlotType;

    IAutoEconomy(std::string evidencePath, IAutoEconomy::Params params)
    {
        evidencePath = evidencePath;
        params = params;
    }

    virtual ~IAutoEconomy()
    {
    }

    virtual void start() = 0;
    virtual void abort() = 0;
    virtual void clear() = 0;

    virtual bool isFinished() = 0;
    virtual void waitForFinish() = 0;
    virtual float getProgress() = 0;

    virtual std::string getEconomyPath() = 0;

    virtual boost::signals2::connection connectProgressSlot(const IAutoEconomy::ProgressSlotType& slot) = 0;
};

ARCHAPI IAutoEconomy* GetAutoEconomy(std::string evidencePath, IAutoEconomy::Params params);

#endif // IAUTOECONOMY_H
