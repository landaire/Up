#-------------------------------------------------
#
# Project created by QtCreator 2011-12-30T11:32:37
#
#-------------------------------------------------

QT       += core widgets concurrent

TARGET = Up
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        MainForm.cpp \
    AboutForm.cpp \
    progressdialog.cpp

HEADERS  += MainForm.h \
    AboutForm.h \
    progressdialog.h

FORMS    += MainForm.ui \
    AboutForm.ui \
    progressdialog.ui

INCLUDEPATH += $$PWD/../FATX/FATX

RESOURCES += \
    MainForm.qrc

CONFIG(debug, debug|release) {
    macx: LIBS += -L$$PWD/../FATX-BUILD-OSX/debug/ -lFATX

    INCLUDEPATH += $$PWD/../FATX-BUILD-OSX/debug
    DEPENDPATH += $$PWD/../FATX-BUILD-OSX/debug

    macx: PRE_TARGETDEPS += $$PWD/../FATX-BUILD-OSX/debug/libFATX.a
} else {
    macx: LIBS += -L$$PWD/../FATX-OSX/release/ -lFATX

    INCLUDEPATH += $$PWD/../FATX-BUILD-OSX/release
    DEPENDPATH += $$PWD/../FATX-BUILD-OSX/release

    macx: PRE_TARGETDEPS += $$PWD/../FATX-OSX/release/libFATX.a
}

cache()
