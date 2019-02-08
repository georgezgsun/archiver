TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = test.out

HEADERS += \
    ../include/archiver.h \
    ../include/fileutils.h

SOURCES += fileutilstest.cpp \
        fileutils.cpp


