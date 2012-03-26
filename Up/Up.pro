#-------------------------------------------------
#
# Project created by QtCreator 2011-12-30T11:32:37
#
#-------------------------------------------------

QT       += core gui

TARGET = Up
TEMPLATE = app
DEFINES += QT_NO_DEBUG_OUTPUT
#QMAKE_CXXFLAGS_RELEASE += -Zi -g

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

#win32{
#QMAKE_CFLAGS_RELEASE += -Zi
#QMAKE_CXXFLAGS_RELEASE += -Zi -g
#QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF}

INCLUDEPATH += $$PWD/../FATX/FATX

RESOURCES += \
    MainForm.qrc


CONFIG(debug, debug|release) {
macx: LIBS += -L$$PWD/../FATX-OSX/debug/ -lFATX

INCLUDEPATH += $$PWD/../FATX-OSX/debug
DEPENDPATH += $$PWD/../FATX-OSX/debug

macx: PRE_TARGETDEPS += $$PWD/../FATX-OSX/debug/libFATX.a
} else {
macx: LIBS += -L$$PWD/../FATX-OSX/release/ -lFATX

INCLUDEPATH += $$PWD/../FATX-OSX/release
DEPENDPATH += $$PWD/../FATX-OSX/release

macx: PRE_TARGETDEPS += $$PWD/../FATX-OSX/release/libFATX.a
}

win32{
    CONFIG(release, debug|release){
        LIBS += -L$$PWD/../FATX-Win/release/ -lFATX
        #message("release")
        INCLUDEPATH += $$PWD/../FATX-Win/release
        DEPENDPATH += $$PWD/../FATX-Win/release
    }
    CONFIG(debug, debug|release){
        LIBS += -L$$PWD/../FATX-Win/debug/ -lFATX
        #message("debug")
        INCLUDEPATH += $$PWD/../FATX-Win/debug
        DEPENDPATH += $$PWD/../FATX-Win/debug
    }
}
