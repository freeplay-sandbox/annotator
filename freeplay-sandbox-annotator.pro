QT       += core gui widgets

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
    timeline.cpp

HEADERS  += annotatorwindow.h \
    bagreader.h \
    imageviewer.h \
    converter.h \
    timeline.hpp

FORMS    += annotatorwindow.ui
