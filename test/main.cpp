#include <iostream>
#include <../include/archiver.h>

#include <time.h>



void testMode(int mode, std::string path) {
        std::string archiveName = "archive.arc";
        std::string archivePath = path;

        time_t start = time(nullptr);
        Archiver* archiver = new Archiver(archiveName);
        archiver->addFile("archiver.o", mode);
        delete archiver;
        archiver = NULL;
        time_t stop = time(nullptr);
        std::cout << mode << " mode time: " << (stop - start) << std::endl;

        std::list<IArchiverFile*> fileList;
        IArchiver::loadFromArchive(archiveName, &fileList);
        std::list<IArchiverFile*>::iterator it;
        for (it = fileList.begin(); it != fileList.end(); it++) {

            std::cout << "File:" << std::endl;
            std::cout << "  Mode: " << (*it)->getMode() << std::endl;
            std::cout << "  Name size: " << (*it)->getFileNameSize() << std::endl;
            std::cout << "  Name: " << (*it)->getFileName() << std::endl;
            std::cout << "  Compressed size: " << (*it)->getCompressedSize() << std::endl;
            std::cout << "  Data size: " << (*it)->getDataSize() << std::endl;
            std::cout << "  Header size: " << (*it)->getHeaderSize() << std::endl;
            std::cout << "  Checksum: " << (*it)->getChecksum().digest << std::endl;

            unsigned char* data = new unsigned char[(*it)->getDataSize()];
            (*it)->readData(data);
            delete [] data;

            delete (*it);
        }
}

void testAutoEco(std::string path) {
    IAutoEconomy::Params params;
    params.count = 3;
    params.enabled = true;
    params.quality = IAutoEconomy::Params::MEDIUM;
    params.hqAsJpeg = false;
    params.resolution = IAutoEconomy::Params::QUARTER;

    IAutoEconomy* ae = GetAutoEconomy(path, params);

    ae->start();
    ae->waitForFinish();

    ae->clear();

    delete ae;
}

void testEvidenceWrapper(std::string evidPath) {

   IEvidenceWrapper* evidenceWrapper = GetEvidenceWrapper(evidPath);

   SmartEye::TrackedObject trackedObject = evidenceWrapper->getTrob();

   std::set<std::string>::reverse_iterator evidFilename;
   std::string fullEvidPath;

   FsTools::SaveTrobFile("//home/matte0/workspace/evidTest//")

   for (evidFilename = trackedObject.evidPaths.rbegin() ; evidFilename != trackedObject.evidPaths.rend(); ++evidFilename) {
       SmartEye::EvidenceData evid = evidenceWrapper->getEvidenceData(*evidFilename);



       std::string fileName = "//home/matte0/workspace/evidTest//" + *evidFilename;
       std::ofstream ofs(fileName.c_str());
       ofs << evid;

       std::cout << *evidFilename << "\n";
   }

   delete evidenceWrapper;
}

int main(int argc, char *argv[])
{

    SmartEye::EvidenceData evid;
    memset(&evid, 0, sizeof(evid));

    evid.cameraData.frame.params.dataCaptured = true;
    evid.cameraData.frame.data.size = 2500 * 2500 * 3;
    evid.cameraData.frame.data.buffer = new unsigned char[evid.cameraData.frame.data.size];
    std::ofstream ofs("test1.evid");
    ofs << evid;
    delete [] evid.cameraData.frame.data.buffer;

    memset(&evid, 0, sizeof(evid));
    std::ifstream ifs("test1.evid");
    ifs >> evid;
    std::cout << evid.cameraData.frame.data.size << std::endl;
    delete [] evid.cameraData.frame.data.buffer;

    if (argc == 2) {
        try {
            testEvidenceWrapper(argv[1]);
        } catch (...) {

        }
    }

    if (argc == 3) {
        testMode(IArchiverFile::FULL, argv[2]);
        testMode(IArchiverFile::COMP_ONLY, argv[2]);
        testMode(IArchiverFile::ENC_ONLY, argv[2]);
        testMode(IArchiverFile::NONE, argv[2]);
    }

    if (argc == 4) {
        testAutoEco(argv[3]);
    }


    return 0;
}
