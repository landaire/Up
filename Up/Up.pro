#-------------------------------------------------
#
# Project created by QtCreator 2011-12-30T11:32:37
#
#-------------------------------------------------

QT       += core gui

TARGET = Up
TEMPLATE = app
DEFINES += QT_NO_DEBUG_OUTPUT
<<<<<<< HEAD

=======
>>>>>>> master

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

<<<<<<< HEAD
INCLUDEPATH += $$PWD/../FATX-Win/debug
DEPENDPATH += $$PWD/../FATX-Win/debug
=======
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
>>>>>>> master
