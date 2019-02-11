#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>
using namespace std;

class Fileutils
{

public:
    Fileutils(string filename);
    string Filename;
    string Path;
    string BaseName;
    string Extension;

    bool isRegularFile;
    bool isDirectory;
    bool isSymbolLink;
};

string getCurrentWorkingPath();
string getFilePath(string filename);
string getFilename(string filename);
string getFileBaseName(string filename);
string getFileExtension(string filename);

bool isRegularFile(string filename);
bool isDirectory(string path);

#endif // FILEUTILS_H
