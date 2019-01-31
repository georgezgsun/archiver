TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11

include(../boost.pri)
include(../unix.pri)

DEFINES += "_HAS_ITERATOR_DEBUGGING=0" ARCH_LIBRARY_EXPORT

TARGET = archiver

HEADERS += \
    include/iarchiverfile.h \
    include/iarchiver.h \
    include/iautoeconomy.h \
    include/ievidencewrapper.h \
    include/lz4.h \
    include/filesreader.h \
    include/archiverutils.h \
    include/archiverfile.h \
    include/archiver.h \
    include/evidencewrapper.h \
    include/autoeconomy.h

SOURCES += \
    src/lz4.c \
    src/filesreader.cpp \
    src/archiverutils.cpp \
    src/archiverfile.cpp \
    src/archiver.cpp \
    src/libentry.cpp \
    src/autoeconomy.cpp \
    src/evidencewrapper.cpp
