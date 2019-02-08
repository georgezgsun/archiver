#include <iostream>
#include <string>
#include <stdio.h>
#include "../include/fileutils.h"
#include "../include/archiver.h"

#include <time.h>

using namespace std;

int main(int argc, char *argv[])
{
    string filename;

    if (argc < 2)
        filename = argv[0];
    else
        filename = argv[1];

    cout << "The candidate is " << filename << endl;
    cout << "- the path is " << getFilePath(filename) << endl;
    cout << "- the filename is " << getFilename(filename) << endl;
    cout << "- the file base name is " << getFileBaseName(filename) << endl;
    cout << "- the extension is " << getFileExtension(filename) << endl;

    cout << "The current working directory is " << getCurrentWorkingPath() << endl;

    bool exist = false;
    cout << "The candidate " << filename;
    if (isRegularFile(filename))
    {
        cout << " is a regular file." << endl;
        exist = true;
    }
    if (isDirectory(filename))
    {
        exist = true;
        cout << " is a directory." << endl;
    }
    if (!exist)
        cout << " does not exist." << endl;


    string files;
    if (argc < 3)
    {
        cout << "Usage: " << getFileBaseName(argv[0]) << " archive_file mode file/path [file/path]" << endl;
        return 0;
    }

    string arguments=argv[2];
    int mode = stoi(argv[2]);

    if (mode > 3 || mode < 0)
    {
        cout << "Invalid mode value " << mode << ". 0 <= mode <= 3." << endl;
        return 0;
    }

    bool result=false;
    Archiver *arch = new Archiver(argv[1]);
    for (int i = 2; i < argc + 1; i++)
    {
        if (arch->addFile(argv[i], mode))
            result = true;
    }

    if (!result)
        cout << "Failed!\n";

    arch->~Archiver();

    return 0;
}
