// https://github.com/KubaO/stackoverflown/tree/master/questions/opencv-21246766
#include <memory>

#include <QtWidgets>
#include <QTimer>
#include <QPushButton>
#include <QSettings>

#include <opencv2/opencv.hpp>
#include <ros/time.h>
#include <audio_common_msgs/AudioData.h>
#include <boost/asio.hpp>

#include "annotatorwindow.hpp"
#include "bagreader.hpp"
#include "imageviewer.hpp"
#include "converter.hpp"
#include "timeline.hpp"
#include "gstaudioplay.hpp"

#include "ajaxhandler.hpp"
#include "http_server/server.hpp"

Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(ros::Time)
Q_DECLARE_METATYPE(ros::Duration)
Q_DECLARE_METATYPE(audio_common_msgs::AudioDataConstPtr)

using namespace std;

class Thread final : public QThread { public: ~Thread() { quit(); wait(); } };

int main(int argc, char *argv[])
{
    qRegisterMetaType<cv::Mat>();
    qRegisterMetaType<ros::Time>();
    qRegisterMetaType<ros::Duration>();
    qRegisterMetaType<audio_common_msgs::AudioDataConstPtr>();


    gst_init(&argc, &argv);

    ////////////////////////////////////////////////////////
    /// \brief HTTP server
    ///
    qDebug() << "Listening for clients...";
    http::server::server<AjaxHandler> s("0.0.0.0", "8080");

    // boost.asio already captures termination signals to stop the server.
    // I add here my own handler to also quit the main loop
    //boost::asio::signal_set signals(s.io_service);
    //signals.add(SIGINT);
    //signals.add(SIGTERM);
    //signals.add(SIGQUIT);

    ////////////////////////////////////////////////////////


    QApplication app(argc, argv);

    GstAudioPlay gstAudioPlayer;
    AnnotatorWindow aw;

    Timeline *timeline = aw.findChild<Timeline*>("timeline");
    timeline->setFocus();

    ImageViewer *envView = aw.findChild<ImageViewer*>("envView");
    ImageViewer *purpleView = aw.findChild<ImageViewer*>("purpleView");
    ImageViewer *yellowView = aw.findChild<ImageViewer*>("yellowView");
    ImageViewer *sandtrayView = aw.findChild<ImageViewer*>("sandtrayView");

    Converter envConverter, purpleConverter, yellowConverter, sandtrayConverter;
    //sandtrayConverter.applyRotation(2); // Rotate 270 degrees clockwise


    BagReader bagreader;
    Thread bagReadingThread, envConverterThread, purpleConverterThread, yellowConverterThread, sandtrayConverterThread;
    // Everything runs at the same priority as the gui, so it won't supply useless frames.
    envConverter.setProcessAll(false);
    purpleConverter.setProcessAll(false);
    yellowConverter.setProcessAll(false);
    sandtrayConverter.setProcessAll(false);

    bagReadingThread.setObjectName("bag reading thread");
    bagReadingThread.start();
    bagreader.moveToThread(&bagReadingThread);

    envConverterThread.start();
    envConverterThread.setObjectName("env camera converter thread");
    purpleConverterThread.start();
    purpleConverterThread.setObjectName("purple camera converter thread");
    yellowConverterThread.start();
    yellowConverterThread.setObjectName("yellow camera converter thread");
    sandtrayConverterThread.start();
    sandtrayConverterThread.setObjectName("sandtray bg converter thread");

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


    QObject::connect(&bagreader, &BagReader::started, [](){ qDebug() << "Starting to play the bag file"; });

    QObject::connect(&app, &QApplication::lastWindowClosed, [&](){bagreader.stop();});

    QObject::connect(&bagreader, &BagReader::durationUpdate, &aw, &AnnotatorWindow::showBagInfo);
    QObject::connect(&bagreader, &BagReader::timeUpdate, timeline, &Timeline::setPlayhead);

    QObject::connect(&bagreader, &BagReader::bagLoaded, timeline, &Timeline::initialize);

    QObject::connect(timeline, &Timeline::timeJump, &bagreader, &BagReader::setPlayTime);

    // HTTP server

    QObject::connect(&s.request_handler, &AjaxHandler::annotationReceived, timeline, &Timeline::newAnnotation);
    QObject::connect(&s.request_handler, &AjaxHandler::pause, &bagreader, &BagReader::pause);
    QObject::connect(&s.request_handler, &AjaxHandler::resume, &bagreader, &BagReader::resume);
    QObject::connect(&s.request_handler, &AjaxHandler::jumpBy, &bagreader, &BagReader::jumpBy);
    QObject::connect(&s.request_handler, &AjaxHandler::jumpTo, &bagreader, &BagReader::jumpTo);
    QObject::connect(&bagreader, &BagReader::paused, &s.request_handler, &AjaxHandler::paused );
    QObject::connect(&bagreader, &BagReader::resumed, &s.request_handler, &AjaxHandler::resumed);


    // buttons
    auto pauseBtn = aw.findChild<QPushButton*>("pauseBtn");
    QObject::connect(pauseBtn, &QPushButton::clicked, &bagreader, &BagReader::togglePause);
    QObject::connect(&bagreader, &BagReader::paused, [&](){pauseBtn->setIcon(QIcon::fromTheme("media-playback-start-symbolic"));});
    QObject::connect(&bagreader, &BagReader::resumed, [&](){pauseBtn->setIcon(QIcon::fromTheme("media-playback-pause-symbolic"));});

    QObject::connect(timeline, &Timeline::togglePause, &bagreader, &BagReader::togglePause);

    auto jumpBack10Btn = aw.findChild<QPushButton*>("jumpBack10Btn");
    QObject::connect(jumpBack10Btn, &QPushButton::clicked, [&](){bagreader.jumpBy(-10);});
    auto jumpBack1Btn = aw.findChild<QPushButton*>("jumpBack1Btn");
    QObject::connect(jumpBack1Btn, &QPushButton::clicked, [&](){bagreader.jumpBy(-1);});
    auto jumpFwd1Btn = aw.findChild<QPushButton*>("jumpFwd1Btn");
    QObject::connect(jumpFwd1Btn, &QPushButton::clicked, [&](){bagreader.jumpBy(1);});
    auto jumpFwd10Btn = aw.findChild<QPushButton*>("jumpFwd10Btn");
    QObject::connect(jumpFwd10Btn, &QPushButton::clicked, [&](){bagreader.jumpBy(10);});

    auto jumpStartBtn = aw.findChild<QPushButton*>("jumpStartBtn");
    QObject::connect(jumpStartBtn, &QPushButton::clicked, [&](){bagreader.jumpTo(0);});
    auto jumpEndBtn = aw.findChild<QPushButton*>("jumpEndBtn");
    QObject::connect(jumpEndBtn, &QPushButton::clicked, [&](){bagreader.jumpTo(-1);});


    // Load bag file and start!

    //aw.showFullScreen();
    aw.show();

    //bagreader.loadBag("/home/slemaignan/freeplay_sandox/data/2017-06-12-143746652201/freeplay.bag");
    //bagreader.loadBag("/home/skadge/freeplay_sandox/data/2017-05-18-145157833880/freeplay.bag");
    QSettings settings("PlymouthUniversity", "FreeplayDatasetAnnotator");

    QString recent = settings.value("recent").toString();
    QString fileName = QFileDialog::getOpenFileName(nullptr, "Load bag file",
                                                    recent,
                                                    "bag file (*.bag)");
    if(fileName.isEmpty()) {return 1;}
        
    settings.setValue("recent", fileName);
    bagreader.loadBag(fileName.toStdString());

    QMetaObject::invokeMethod(&bagreader, "start");

    // Configure HTTP server polling
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]{s.poll();});
    timer.start();

    return app.exec();

}


