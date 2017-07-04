#ifndef BAGREADER_H
#define BAGREADER_H

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

    Q_SIGNAL void envImgReady(const cv::Mat &);
    Q_SIGNAL void purpleImgReady(const cv::Mat &);
    Q_SIGNAL void yellowImgReady(const cv::Mat &);
    Q_SIGNAL void audioFrameReady(const audio_common_msgs::AudioDataConstPtr&);

    void loadBag(const std::string& path);

    Q_SIGNAL void bagLoaded(ros::Time start, ros::Time end);

    Q_SLOT void processBag(ros::Time start=ros::TIME_MIN, ros::Time stop=ros::TIME_MAX);

    Q_SIGNAL void timeUpdate(ros::Time time);
    Q_SLOT void setPlayTime(ros::Time time);

private:

    bool running_;

    float time_scale_;

    rosbag::TimeTranslator time_translator_;

    rosbag::Bag bag_;


};

#endif // BAGREADER_H
