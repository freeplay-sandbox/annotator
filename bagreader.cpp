#include <vector>
#include <string>

#include <QTimerEvent>
#include <QDebug>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sensor_msgs/CompressedImage.h>

#include "bagreader.h"

using namespace std;

const string CAM_PURPLE("camera_purple/rgb/image_raw/compressed");
const string CAM_YELLOW("camera_yellow/rgb/image_raw/compressed");
const string CAM_ENV("env_camera/qhd/image_color/compressed");

BagReader::BagReader(QObject *parent) : QObject(parent), running(false) {}

void BagReader::start() {
    running = true;
    processBag();
}

void BagReader::stop() {
    running = false;
}

void BagReader::loadBag(const std::__cxx11::string &path)
{
    qDebug() << "Loading bag file...";
    bag.open(path, rosbag::bagmode::Read);
    qDebug() << "Loading completed.";

    vector<string> topics = {CAM_ENV, CAM_PURPLE, CAM_YELLOW};

    bagview.addQuery(bag, rosbag::TopicQuery(topics));

}

void BagReader::processBag()
{
    for(rosbag::MessageInstance const m : bagview)
    {
        if(!running) break;

        auto compressed_rgb = m.instantiate<sensor_msgs::CompressedImage>();
        if (compressed_rgb != NULL) {
            auto cvimg = cv::imdecode(compressed_rgb->data,1);

            if(m.getTopic() == CAM_ENV) emit envImgReady(cvimg);
            else if(m.getTopic() == CAM_PURPLE) emit purpleImgReady(cvimg);
            else if(m.getTopic() == CAM_YELLOW) emit yellowImgReady(cvimg);
        }
    }

}
