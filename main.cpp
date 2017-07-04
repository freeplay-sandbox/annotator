// https://github.com/KubaO/stackoverflown/tree/master/questions/opencv-21246766
#include <memory>

#include <QtWidgets>
#include <opencv2/opencv.hpp>
#include <ros/time.h>
#include <audio_common_msgs/AudioData.h>

#include "annotatorwindow.h"
#include "bagreader.h"
#include "imageviewer.h"
#include "converter.h"
#include "timeline.hpp"
#include <gstaudioplay.h>

Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(ros::Time)
Q_DECLARE_METATYPE(audio_common_msgs::AudioDataConstPtr)

using namespace std;

class Thread final : public QThread { public: ~Thread() { quit(); wait(); } };

int main(int argc, char *argv[])
{
    qRegisterMetaType<cv::Mat>();
    qRegisterMetaType<ros::Time>();
    qRegisterMetaType<audio_common_msgs::AudioDataConstPtr>();

    QApplication app(argc, argv);

    gst_init(&argc, &argv);

    GstAudioPlay gstAudioPlayer;

    AnnotatorWindow aw;

    Timeline *timeline = aw.findChild<Timeline*>("timeline");

    ImageViewer *envView = aw.findChild<ImageViewer*>("envView");
    ImageViewer *purpleView = aw.findChild<ImageViewer*>("purpleView");
    ImageViewer *yellowView = aw.findChild<ImageViewer*>("yellowView");
    ImageViewer *sandtrayView = aw.findChild<ImageViewer*>("sandtrayView");
    Converter envConverter, purpleConverter, yellowConverter, sandtrayConverter;


    BagReader bagreader;
    Thread bagReadingThread, envConverterThread, purpleConverterThread, yellowConverterThread, sandtrayConverterThread;
    // Everything runs at the same priority as the gui, so it won't supply useless frames.
    envConverter.setProcessAll(false);
    purpleConverter.setProcessAll(false);
    yellowConverter.setProcessAll(false);
    sandtrayConverter.setProcessAll(false);

    bagReadingThread.start();


    bagreader.moveToThread(&bagReadingThread);

    envConverterThread.start();
    purpleConverterThread.start();
    yellowConverterThread.start();
    sandtrayConverterThread.start();

    envConverter.moveToThread(&envConverterThread);
    purpleConverter.moveToThread(&purpleConverterThread);
    yellowConverter.moveToThread(&yellowConverterThread);
    sandtrayConverter.moveToThread(&sandtrayConverterThread);

    QObject::connect(&bagreader, &BagReader::envImgReady, &envConverter, &Converter::processFrame);
    QObject::connect(&envConverter, &Converter::imageReady, envView, &ImageViewer::setImage);

    QObject::connect(&bagreader, &BagReader::purpleImgReady, &purpleConverter, &Converter::processFrame);
    QObject::connect(&purpleConverter, &Converter::imageReady, purpleView, &ImageViewer::setImage);

    QObject::connect(&bagreader, &BagReader::yellowImgReady, &yellowConverter, &Converter::processFrame);
    QObject::connect(&yellowConverter, &Converter::imageReady, yellowView, &ImageViewer::setImage);

    QObject::connect(&bagreader, &BagReader::sandtrayImgReady, &sandtrayConverter, &Converter::processFrame);
    QObject::connect(&sandtrayConverter, &Converter::imageReady, sandtrayView, &ImageViewer::setImage);

    QObject::connect(&bagreader, &BagReader::audioFrameReady, &gstAudioPlayer, &GstAudioPlay::audioMsgReady);

    //aw.showFullScreen();
    aw.show();

    QObject::connect(&bagreader, &BagReader::started, [](){ qDebug() << "Starting to play the bag file"; });

    QObject::connect(&app, &QApplication::lastWindowClosed, [&](){bagreader.stop();});

    QObject::connect(&bagreader, &BagReader::timeUpdate, &aw, &AnnotatorWindow::showFPS);
    QObject::connect(&bagreader, &BagReader::timeUpdate, timeline, &Timeline::setPlayhead);

    QObject::connect(&bagreader, &BagReader::bagLoaded, timeline, &Timeline::initialize);

    QObject::connect(timeline, &Timeline::timeJump, &bagreader, &BagReader::setPlayTime);

    bagreader.loadBag("/home/slemaignan/freeplay_sandox/data/2017-06-13-102226367218/freeplay.bag");

    QMetaObject::invokeMethod(&bagreader, "start");

    return app.exec();

}

//#include "main.moc"

