#include <vector>
#include <string>
#include <chrono>
#include <thread> // for sleep_for when paused

#include <QTimerEvent>
#include <QDebug>
#include <QCoreApplication>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sensor_msgs/CompressedImage.h>

#include <rosbag/view.h>

#include "bagreader.hpp"

using namespace std;
using namespace std::chrono;

const string AUDIO_PURPLE("camera_purple/audio");
const string CAM_PURPLE("camera_purple/rgb/image_raw/compressed");
const string CAM_YELLOW("camera_yellow/rgb/image_raw/compressed");
const string CAM_ENV("env_camera/qhd/image_color/compressed");
const string SANDTRAY_BG("/sandtray/background/image/compressed");

BagReader::BagReader(QObject *parent) :
    QObject(parent),
    running_(false),
    paused_(false),
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
    paused_ = false;
    running_ = false;
}

void BagReader::togglePause()
{
    if (paused_) resume();
    else pause();
}

void BagReader::pause()
{
    qDebug() << "Paused";
    paused_ = true;
    emit paused();
}

void BagReader::resume()
{
    qDebug() << "Resumed";
    paused_ = false;
    emit resumed();
}

void BagReader::jumpBy(int secs)
{
    begin_ = current_ = std::min(bag_end_, std::max(bag_begin_, current_ + ros::Duration(secs)));
    emit timeUpdate(current_);
    emit durationUpdate(current_ - bag_begin_);
    restartProcess_ = true;

}

void BagReader::jumpTo(int secs)
{
    if(secs < 0) begin_ = end_;
    else if (secs < 10000) { // we assume the timestamp is relative to the start time
        begin_ = bag_begin_ + ros::Duration(secs);
    }
    else {
        begin_ = ros::Time(secs);
    }

    // clamp to the bag length
    begin_ = std::min(bag_end_, std::max(bag_begin_, begin_));

    current_ = begin_;

    emit timeUpdate(current_);
    emit durationUpdate(current_ - bag_begin_);

    restartProcess_ = true;
}

void BagReader::loadBag(const std::__cxx11::string &path)
{
    qDebug() << "Loading bag file...";
    bag_.open(path, rosbag::bagmode::Read);
    qDebug() << "Loading completed.";

    rosbag::View bagview(bag_);

    bag_begin_ = begin_ = current_ = bagview.getBeginTime();
    bag_end_ = end_ = bagview.getEndTime();

    emit bagLoaded(bag_begin_, bag_end_);
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

            if(paused_) {
                auto paused_time_ = ros::WallTime::now();
                while(paused_ && running_) {
                    QCoreApplication::processEvents();
                    std::this_thread::sleep_for(milliseconds(10));
                }
                ros::WallDuration shift = ros::WallTime::now() - paused_time_;
                time_translator_.shift(ros::Duration(shift.sec, shift.nsec));
            }

            if(!running_ || restartProcess_) {
                restartProcess_ = false;
                break;
            }

            ros::Time const& time = m.getTime();
            current_ = time;
            emit timeUpdate(time);
            emit durationUpdate(time - bag_begin_);

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
    begin_ = current_ = time;
    emit timeUpdate(time);
    emit durationUpdate(time - bag_begin_);
    restartProcess_ = true;
}

