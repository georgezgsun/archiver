#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <iostream>
#include <string>

#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>

std::string getPath(std::string filename);
std::string getName(std::string filename);
std::string getExtension(std::string filename);
std::string getCurrentWorkingPath();

bool isRegularFile(std::string filename);
bool isDirectory(std::string path);
#endif // FILEUTILS_H
