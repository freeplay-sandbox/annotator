QT       += core gui widgets network

TARGET = freeplay-sandbox-annotator
TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += opencv roscpp rosbag gstreamer-audio-1.0 audio_common_msgs

CONFIG += c++11



SOURCES += main.cpp\
        annotatorwindow.cpp \
    bagreader.cpp \
    imageviewer.cpp \
    converter.cpp \
    timeline.cpp \
    gstaudioplay.cpp \
    annotation.cpp \
    ajaxresponder.cpp

HEADERS  += annotatorwindow.h \
    bagreader.h \
    imageviewer.h \
    converter.h \
    timeline.hpp \
    gstaudioplay.h \
    annotation.h \
    ajaxresponder.h

FORMS    += annotatorwindow.ui
