#-------------------------------------------------
#
# Project created by QtCreator 2011-12-30T11:32:37
#
#-------------------------------------------------

QT       += core gui

TARGET = Up
TEMPLATE = app
DEFINES += QT_NO_DEBUG_OUTPUT UNICODE _UNICODE


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


RESOURCES += \
    MainForm.qrc


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../FATX-Windows-Debug/release/ -lFATX
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../FATX-Windows-Debug/debug/ -lFATX

INCLUDEPATH += $$PWD/../FATX-Windows-Debug/debug
DEPENDPATH += $$PWD/../FATX-Windows-Debug/debug

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../FATX-Windows-Debug/release/FATX.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../FATX-Windows-Debug/debug/FATX.lib
