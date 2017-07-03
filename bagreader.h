#ifndef BAGREADER_H
#define BAGREADER_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QBasicTimer>

#include <rosbag/bag.h>
#include <rosbag/view.h>

class BagReader : public QObject
{
    Q_OBJECT
    QBasicTimer m_timer;
    QScopedPointer<cv::VideoCapture> m_videoCapture;
public:
    BagReader(QObject * parent = {});
    Q_SIGNAL void started();
    Q_SLOT void start(int cam = {});
    Q_SLOT void stop();

    Q_SIGNAL void envImgReady(const cv::Mat &);
    Q_SIGNAL void purpleImgReady(const cv::Mat &);
    Q_SIGNAL void yellowImgReady(const cv::Mat &);

    void loadBag(const std::string& path);
    Q_SLOT void processBag();

private:

    rosbag::Bag bag;
    rosbag::View bagview;


};

#endif // BAGREADER_H
