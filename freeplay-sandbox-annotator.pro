#-------------------------------------------------
#
# Project created by QtCreator 2017-07-03T20:25:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = freeplay-sandbox-annotator
TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += opencv roscpp rosbag

CONFIG += c++11



SOURCES += main.cpp\
        annotatorwindow.cpp \
    bagreader.cpp \
    imageviewer.cpp \
    converter.cpp \
    timeline.cpp \
    timelineview.cpp

HEADERS  += annotatorwindow.h \
    bagreader.h \
    imageviewer.h \
    converter.h \
    timeline.hpp \
    timelineview.hpp

FORMS    += annotatorwindow.ui
