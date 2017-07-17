#ifndef BAGREADER_H
#define BAGREADER_H

#include <mutex>
#include <condition_variable>

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QBasicTimer>


#include <rosbag/bag.h>
#include <rosbag/time_translator.h>
#include "audio_common_msgs/AudioData.h"

class BagReader : public QObject
{
    Q_OBJECT
    QBasicTimer m_timer;
    QScopedPointer<cv::VideoCapture> m_videoCapture;
public:
    BagReader(QObject * parent = {});

    Q_SIGNAL void started();
    Q_SLOT void start();
    Q_SLOT void stop();
    Q_SLOT void pause();
    Q_SIGNAL void paused();
    Q_SLOT void resume();
    Q_SIGNAL void resumed();
    Q_SLOT void togglePause();
    Q_SLOT void jumpBy(int secs);
    /**
     * jump to a specific timestamp.
     * A negative value means 'to the end'.
     *
     * The timestamp can be either a value relative to the
     * start time or the absolute timestamp.
     */
    Q_SLOT void jumpTo(int secs);

    Q_SIGNAL void envImgReady(const cv::Mat &);
    Q_SIGNAL void purpleImgReady(const cv::Mat &);
    Q_SIGNAL void yellowImgReady(const cv::Mat &);
    Q_SIGNAL void sandtrayImgReady(const cv::Mat &);
    Q_SIGNAL void audioFrameReady(const audio_common_msgs::AudioDataConstPtr&);

    void loadBag(const std::string& path);

    Q_SIGNAL void bagLoaded(ros::Time start, ros::Time end);

    Q_SLOT void processBag();

    Q_SIGNAL void durationUpdate(ros::Duration time);
    Q_SIGNAL void timeUpdate(ros::Time time);
    Q_SLOT void setPlayTime(ros::Time time);

private:

    bool running_;
    bool paused_;
    bool restartProcess_;
    ros::Time bag_begin_, bag_end_;
    ros::Time begin_, end_; // might be different from bag_* if we are playing a subset of the bag
    ros::Time current_;

    float time_scale_;

    rosbag::TimeTranslator time_translator_;

    rosbag::Bag bag_;

};

#endif // BAGREADER_H
