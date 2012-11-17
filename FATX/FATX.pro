#-------------------------------------------------
#
# Project created by QtCreator 2011-12-30T11:35:47
#
#-------------------------------------------------
include(IO/include.pri)
include(FATX/include.pri)

QT       -= gui

TARGET = FATX
TEMPLATE = lib
CONFIG += staticlib app_bundle
DEFINES += UNICODE _UNICODE #QT_NO_DEBUG_OUTPUT # _LARGEFILE64_SOURCE
QMAKE_CXXFLAGS += -std=c++0x
macx {
    QMAKE_CC = clang
    QMAKE_CXX = clang++
}

#QMAKE_CXXFLAGS += -D_FILE_OFFSET_BITS=64

#win32{
#QMAKE_CFLAGS_RELEASE += -Zi
#QMAKE_CXXFLAGS_RELEASE += -Zi -g
#QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF}


CONFIG(release, debug|release) {
 #DEFINES += QT_NO_DEBUG_OUTPUT
}

# FATX Headers
HEADERS += \
    FATX/StaticInformation.h \
    FATX/Helpers.h \
    FATX/Drive.h \
    FATX/stfspackage.h \
    nowide/config.h \
    nowide/convert.h \
    nowide/cstdio.h \
    nowide/fstream.h \
    nowide/streambuf.h \
    FATX/pcutils.h
SOURCES += \
    FATX/Helpers.cpp \
    FATX/Drive.cpp \
    FATX/stfspackage.cpp \
    src/convert.cpp \
    src/streambuf.cpp \
    FATX/pcutils.cpp

# IO Headers
HEADERS += \
    IO/xMultiFileStream.h \
    IO/xFileStream.h \
    IO/xDeviceStream.h \
    IO/xDeviceFileStream.h \
    IO/IStream.h

SOURCES += \
    IO/xMultiFileStream.cpp \
    IO/xFileStream.cpp \
    IO/xDeviceStream.cpp \
    IO/xDeviceFileStream.cpp \
    IO/IStream.cpp

HEADERS += \
    stdafx.h \
    typedefs.h \
    xexception.h

SOURCES += \
    stdafx.cpp
