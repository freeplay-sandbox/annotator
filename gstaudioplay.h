/**
  * Code based on https://github.com/ros-drivers/audio_common/blob/master/audio_play/src/audio_play.cpp
  */
#ifndef GSTAUDIOPLAY_H
#define GSTAUDIOPLAY_H

#include <QObject>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include "audio_common_msgs/AudioData.h"

#include <thread>

class GstAudioPlay : public QObject
{
    Q_OBJECT
public:
    GstAudioPlay();

    Q_SLOT void audioMsgReady(const audio_common_msgs::AudioDataConstPtr &msg);
private:


    static void cb_newpad (GstElement *decodebin, GstPad *pad,
                           gpointer data);

    static void cb_need_data (GstElement *appsrc,
                              guint       unused_size,
                              gpointer    user_data);

    std::thread _gst_thread;

    GstElement *_pipeline, *_source, *_sink, *_decoder, *_convert, *_audio;
    GstElement *_playbin;
    GMainLoop *_loop;

    bool _paused;
};

#endif // GSTAUDIOPLAY_H
