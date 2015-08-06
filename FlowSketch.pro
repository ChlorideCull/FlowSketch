#-------------------------------------------------
#
# Project created by QtCreator 2015-08-05T22:58:06
#
#-------------------------------------------------

QT       += core gui xml svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlowSketch
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    flowwidget.cpp \
    iostreambridge.cpp \
    sketchxml.cpp

HEADERS  += mainwindow.h \
    flowwidget.h \
    flownode.h \
    iostreambridge.h \
    sketchxml.h

FORMS    += mainwindow.ui

DISTFILES += \
    notes.txt

QMAKE_CXXFLAGS += -std=c++11
