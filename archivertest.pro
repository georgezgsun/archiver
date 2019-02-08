TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
PKGCONFIG += libssl

QMAKE_CXXFLAGS += -std=c++11

##include(../boost.pri)
##include(../unix.pri)

DEFINES += "_HAS_ITERATOR_DEBUGGING=0" ARCH_LIBRARY_EXPORT

TARGET = test

HEADERS += \
    include/lz4.h \
    include/archiverutils.h \
    include/archiver.h \
    include/fileutils.h

SOURCES += \
    src/lz4.c \
    src/archiverutils.cpp \
    src/archiver.cpp \
    src/fileutils.cpp \
    src/archivertest.cpp

