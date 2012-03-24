#-------------------------------------------------
#
# Project created by QtCreator 2011-12-30T11:32:37
#
#-------------------------------------------------

QT       += core gui

TARGET = Up
TEMPLATE = app
DEFINES += QT_NO_DEBUG_OUTPUT


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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../FATX-Win/release/ -lFATX
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../FATX-Win/debug/ -lFATX

INCLUDEPATH += $$PWD/../FATX-Win/debug
DEPENDPATH += $$PWD/../FATX-Win/debug
