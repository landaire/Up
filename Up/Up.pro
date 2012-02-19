#-------------------------------------------------
#
# Project created by QtCreator 2011-12-30T11:32:37
#
#-------------------------------------------------

QT       += core gui

TARGET = Up
TEMPLATE = app


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

win32{
QMAKE_CFLAGS_RELEASE += -Zi
QMAKE_CXXFLAGS_RELEASE += -Zi -g
QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF}

INCLUDEPATH += $$PWD/../FATX/FATX

RESOURCES += \
    MainForm.qrc

#release {
#macx: LIBS += -L$$PWD/../FATX-OSXRelease/ -lFATX

#INCLUDEPATH += $$PWD/../FATX-OSXRelease
#DEPENDPATH += $$PWD/../FATX-OSXRelease

#macx: PRE_TARGETDEPS += $$PWD/../FATX-OSXRelease/libFATX.a
#DEFINES += QT_NO_DEBUG_OUTPUT
#}

debug {
macx: LIBS += -L$$PWD/../FATX-OSXDebug/ -lFATX

INCLUDEPATH += $$PWD/../FATX-OSXDebug
DEPENDPATH += $$PWD/../FATX-OSXDebug

macx: PRE_TARGETDEPS += $$PWD/../FATX-OSXDebug/libFATX.a
}
