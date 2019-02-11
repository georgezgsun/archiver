#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>

#include "fileutils.h"

using namespace std;

Fileutils::Fileutils(string filename)
{
    Path = "";
    Filename = filename;
    BaseName = Filename;
    Extension = "";
    isDirectory = false;
    isSymbolLink = false;
    isRegularFile = false;

    size_t i = Filename.rfind('/');
    if (i != string::npos)
    {
        Path = Filename.substr(0,i);
        Filename = Filename.substr(i+1);
    }

    i = Filename.rfind(('.'));
    if (i != string::npos)
    {
        BaseName = Filename.substr(0,i);
        Extension = Filename.substr(i+1);
    }

    struct stat st;
    if (stat(filename.c_str(), &st) == 0)
    {
        isDirectory = S_ISDIR(st.st_mode);
        isRegularFile = S_ISREG(st.st_mode);
        isSymbolLink = S_ISLNK(st.st_mode);
    }
}

string getCurrentWorkingPath()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof (cwd)))
        return cwd;
    return "";
}

string getFilePath(string filename)
{
    size_t i = filename.rfind('/');
    if (i != string::npos)
        return filename.substr(0,i);
    return "";
};

string getFilename(string filename)
{
    size_t i = filename.rfind('/');
    if (i != string::npos)
        return filename.substr(i+1);
    return filename;
};

string getFileBaseName(string filename)
{
    string name = filename;
    size_t i = filename.rfind('/');
    if (i != string::npos)
        name = filename.substr(i+1);
    size_t j = name.rfind('.');

    return name.substr(0, j);
};

string getFileExtension(string filename)
{
    size_t i = filename.rfind('/');
    size_t j = filename.rfind('.');
    if ((j != string::npos) && (j > i))
        return filename.substr(j+1);
    return "";
};

bool isRegularFile(string filename)
{
    struct stat st;
    if (stat(filename.c_str(), &st))
        return false;
    if (S_ISREG(st.st_mode))
        return true;
    return false;
};

bool isDirectory(string path)
{
    struct stat st;
    if (stat(path.c_str(), &st))
        return false;
    if (S_ISDIR(st.st_mode))
        return true;
    return false;
};

