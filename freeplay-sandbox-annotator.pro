QT       += core gui widgets network

TARGET = freeplay-sandbox-annotator
TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += opencv roscpp rosbag gstreamer-audio-1.0 audio_common_msgs jsoncpp

CONFIG += c++11



SOURCES +=\
    src/http_server/connection.cpp \
    src/http_server/connection_manager.cpp \
    src/http_server/reply.cpp \
    src/http_server/request_handler.cpp \
    src/http_server/request_parser.cpp \
    src/main.cpp \
    src/ajaxresponder.cpp \
    src/annotation.cpp \
    src/annotatorwindow.cpp \
    src/bagreader.cpp \
    src/converter.cpp \
    src/imageviewer.cpp \
    src/gstaudioplay.cpp \
    src/timeline.cpp

HEADERS  += \
    src/http_server/connection.hpp \
    src/http_server/connection_manager.hpp \
    src/http_server/header.hpp \
    src/http_server/reply.hpp \
    src/http_server/request.hpp \
    src/http_server/request_handler.hpp \
    src/http_server/request_parser.hpp \
    src/http_server/server.hpp \
    src/ajaxresponder.h \
    src/annotation.h \
    src/annotatorwindow.h \
    src/bagreader.h \
    src/converter.h \
    src/imageviewer.h \
    src/gstaudioplay.h \
    src/timeline.hpp

FORMS    += annotatorwindow.ui
