#include <vector>
#include <string>

#include <QTimerEvent>
#include <QDebug>
#include <QCoreApplication>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sensor_msgs/CompressedImage.h>

#include <rosbag/view.h>

#include "bagreader.h"

using namespace std;

const string AUDIO_PURPLE("camera_purple/audio");
const string CAM_PURPLE("camera_purple/rgb/image_raw/compressed");
const string CAM_YELLOW("camera_yellow/rgb/image_raw/compressed");
const string CAM_ENV("env_camera/qhd/image_color/compressed");
const string SANDTRAY_BG("/sandtray/background/image/compressed");

BagReader::BagReader(QObject *parent) :
    QObject(parent),
    running_(false),
    begin_(ros::TIME_MIN),
    end_(ros::TIME_MAX),
    time_scale_(1)
{


}

void BagReader::start()
{
    running_ = true;
    emit started();
    processBag();
}

void BagReader::stop()
{
    qDebug() << "Closing the bag.";
    running_ = false;
}

void BagReader::loadBag(const std::__cxx11::string &path)
{
    qDebug() << "Loading bag file...";
    bag_.open(path, rosbag::bagmode::Read);
    qDebug() << "Loading completed.";

    rosbag::View bagview(bag_);

    auto begin_time = bagview.getBeginTime();
    auto end_time = bagview.getEndTime();

    emit bagLoaded(begin_time, end_time);
}

void BagReader::processBag()
{
    vector<string> topics = {AUDIO_PURPLE, CAM_ENV, CAM_PURPLE, CAM_YELLOW, SANDTRAY_BG};

    while(running_) {
        rosbag::View view;
        view.addQuery(bag_, rosbag::TopicQuery(topics), begin_, end_);

        // configure the time translator

        time_translator_.setTimeScale(1);
        time_translator_.setRealStartTime(view.getBeginTime());
        ros::WallTime now_wt = ros::WallTime::now();
        time_translator_.setTranslatedStartTime(ros::Time(now_wt.sec, now_wt.nsec));

        for(rosbag::MessageInstance const m : view)
        {
            if(!running_ || restartProcess_) {
                restartProcess_ = false;
                break;
            }

            ros::Time const& time = m.getTime();
            emit timeUpdate(time);

            ros::Time translated = time_translator_.translate(time);
            ros::WallTime horizon = ros::WallTime(translated.sec, translated.nsec);
            ros::WallTime::sleepUntil(horizon);

            if(m.getTopic() == AUDIO_PURPLE) {
                auto msg = m.instantiate<audio_common_msgs::AudioData>();

                emit audioFrameReady(msg);

            }
            else {
                auto compressed_rgb = m.instantiate<sensor_msgs::CompressedImage>();
                if (compressed_rgb != NULL) {
                    auto cvimg = cv::imdecode(compressed_rgb->data,1);

                    if(m.getTopic() == CAM_ENV) emit envImgReady(cvimg);
                    else if(m.getTopic() == CAM_PURPLE) emit purpleImgReady(cvimg);
                    else if(m.getTopic() == CAM_YELLOW) emit yellowImgReady(cvimg);
                    else if(m.getTopic() == SANDTRAY_BG) emit sandtrayImgReady(cvimg);
                }
            }

            QCoreApplication::processEvents();
        }
    }
}

void BagReader::setPlayTime(ros::Time time)
{
    begin_ = time;
    restartProcess_ = true;
}

