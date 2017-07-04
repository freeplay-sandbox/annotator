
#include <QDebug>

#include "gstaudioplay.h"




GstAudioPlay::GstAudioPlay()
{
    GstPad *audiopad;


    _loop = g_main_loop_new(NULL, false);

    _pipeline = gst_pipeline_new("app_pipeline");
    _source = gst_element_factory_make("appsrc", "app_source");
    gst_bin_add( GST_BIN(_pipeline), _source);

    g_signal_connect(_source, "need-data", G_CALLBACK(cb_need_data),this);

    _decoder = gst_element_factory_make("decodebin", "decoder");
    g_signal_connect(_decoder, "pad-added", G_CALLBACK(cb_newpad),this);
    gst_bin_add( GST_BIN(_pipeline), _decoder);
    gst_element_link(_source, _decoder);

    _audio = gst_bin_new("audiobin");
    _convert = gst_element_factory_make("audioconvert", "convert");
    audiopad = gst_element_get_static_pad(_convert, "sink");
    _sink = gst_element_factory_make("autoaudiosink", "sink");
    gst_bin_add_many( GST_BIN(_audio), _convert, _sink, NULL);
    gst_element_link(_convert, _sink);
    gst_element_add_pad(_audio, gst_ghost_pad_new("sink", audiopad));
    gst_object_unref(audiopad);

    gst_bin_add(GST_BIN(_pipeline), _audio);


    gst_element_set_state(GST_ELEMENT(_pipeline), GST_STATE_PLAYING);
    //gst_element_set_state(GST_ELEMENT(_playbin), GST_STATE_PLAYING);

    _gst_thread = std::thread( std::bind(g_main_loop_run, _loop) );

    _paused = false;
}

void GstAudioPlay::audioMsgReady(const audio_common_msgs::AudioDataConstPtr &msg)
{
    if(_paused)
    {
        gst_element_set_state(GST_ELEMENT(_pipeline), GST_STATE_PLAYING);
        _paused = false;
    }

    GstBuffer *buffer = gst_buffer_new_and_alloc(msg->data.size());
    gst_buffer_fill(buffer, 0, &msg->data[0], msg->data.size());
    GstFlowReturn ret;

    g_signal_emit_by_name(_source, "push-buffer", buffer, &ret);
}

void GstAudioPlay::cb_newpad(GstElement *decodebin, GstPad *pad, gpointer data)
{
    GstAudioPlay *client = reinterpret_cast<GstAudioPlay*>(data);

    GstCaps *caps;
    GstStructure *str;
    GstPad *audiopad;

    /* only link once */
    audiopad = gst_element_get_static_pad (client->_audio, "sink");
    if (GST_PAD_IS_LINKED (audiopad))
    {
        g_object_unref (audiopad);
        return;
    }

    /* check media type */
    caps = gst_pad_query_caps (pad, NULL);
    str = gst_caps_get_structure (caps, 0);
    if (!g_strrstr (gst_structure_get_name (str), "audio")) {
        gst_caps_unref (caps);
        gst_object_unref (audiopad);
        return;
    }

    gst_caps_unref (caps);

    /* link'n'play */
    gst_pad_link (pad, audiopad);

    g_object_unref (audiopad);
}

void GstAudioPlay::cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
{
    qDebug() << "need-data signal emitted! Pausing the pipeline";
    GstAudioPlay *client = reinterpret_cast<GstAudioPlay*>(user_data);
    gst_element_set_state(GST_ELEMENT(client->_pipeline), GST_STATE_PAUSED);
    client->_paused = true;
}

