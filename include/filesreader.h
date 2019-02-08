#ifndef FILESREADER_H
#define FILESREADER_H

#include <string>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

class FilesReader {
private:
    std::string baseBath;
    std::list<std::string> fileList;

    void removeHash(std::string path);

public:
    FilesReader(std::string path);

};

#endif // FILESREADER_H
