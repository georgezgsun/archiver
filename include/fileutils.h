#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <iostream>
#include <string>

#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>

std::string getFilePath(std::string filename);
std::string getFilename(std::string filename);
std::string getFileBaseName(std::string filename);
std::string getFileExtension(std::string filename);
std::string getCurrentWorkingPath();

bool isRegularFile(std::string filename);
bool isDirectory(std::string path);
#endif // FILEUTILS_H
