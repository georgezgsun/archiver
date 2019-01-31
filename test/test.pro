TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

include(../../unix.pri)
include(../../ipp.pri)
include(../../boost.pri)

TARGET = archiver-test

SOURCES += main.cpp

LIBS += -llogger -larchiver -limgtool -lcrypto
