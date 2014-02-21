#-------------------------------------------------
#
# Project created by QtCreator 2011-12-30T11:35:47
#
#-------------------------------------------------
QT += core gui
QT -= widgets

TARGET = FATX
TEMPLATE = lib
DEFINES += UNICODE _UNICODE
QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -std=c++11 -stdlib=libc++
CONFIG += staticlib app_bundle c++11

# FATX Headers
HEADERS += \
    nowide/config.h             \
    nowide/convert.h            \
    nowide/cstdio.h             \
    nowide/fstream.h            \
    nowide/streambuf.h          \
    fatx/pcutils.h \
    fatx/drive.h \
    fatx/helpers.h \
    fatx/static_information.h \
    fatx/stfs_package.h \
    io/istream.h \
    io/device_file_stream.h \
    io/device_stream.h \
    io/file_stream.h \
    io/multi_file_stream.h \
    fatx/ientry.h \
    fatx/file.h \
    fatx/folder.h
SOURCES += \
    src/convert.cpp         \
    src/streambuf.cpp       \
    fatx/pcutils.cpp \
    fatx/drive.cpp \
    fatx/helpers.cpp \
    fatx/stfs_package.cpp \
    io/istream.cpp \
    io/device_file_stream.cpp \
    io/device_stream.cpp \
    io/file_stream.cpp \
    io/multi_file_stream.cpp \
    fatx/ientry.cpp \
    fatx/file.cpp \
    fatx/folder.cpp

# IO Headers
HEADERS +=

SOURCES +=

HEADERS += \
    stdafx.h \
    typedefs.h \
    xexception.h

SOURCES += \
    stdafx.cpp

cache()
