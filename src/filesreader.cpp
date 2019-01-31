#include "filesreader.h"

void FilesReader::removeHash(std::string path)
{
    while (path.at(path.length() - 1) == '/') {
        path.erase(path.length() - 1, 1);
    }
}

FilesReader::FilesReader(std::string path) :
    baseBath(path)
{

    while (path.at(path.length() - 1) == '/') {
        path.erase(path.length() - 1, 1);
    }

    boost::filesystem::path dir(baseBath);
    boost::filesystem::recursive_directory_iterator end;

    for (boost::filesystem::recursive_directory_iterator begin(dir); begin != end; begin++) {
        std::string p = begin->path().string();
    }

}
