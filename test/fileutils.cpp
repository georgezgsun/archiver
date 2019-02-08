#include "../include/fileutils.h"
#include <unistd.h>

std::string getFilePath(std::string filename)
{
    size_t i = filename.rfind('/');
    if (i != std::string::npos)
        return filename.substr(0,i);
    return "";
};

std::string getFilename(std::string filename)
{
    size_t i = filename.rfind('/');
    if (i != std::string::npos)
        return filename.substr(i+1);
    return filename;
};

std::string getFileBaseName(std::string filename)
{
    std::string name = filename;
    size_t i = filename.rfind('/');
    if (i != std::string::npos)
        name = filename.substr(i+1);
    size_t j = name.rfind('.');

    return name.substr(0, j);
};

std::string getFileExtension(std::string filename)
{
    size_t i = filename.rfind('/');
    size_t j = filename.rfind('.');
    if ((j != std::string::npos) && (j > i))
        return filename.substr(j+1);
    return "";
};

bool isRegularFile(std::string filename)
{
    struct stat st;
    if (stat(filename.c_str(), &st))
        return false;
    if (S_ISREG(st.st_mode))
        return true;
    return false;
};

bool isDirectory(std::string path)
{
    struct stat st;
    if (stat(path.c_str(), &st))
        return false;
    if (S_ISDIR(st.st_mode))
        return true;
    return false;
};

std::string getCurrentWorkingPath()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof (cwd)))
        return cwd;
    return "";
}
