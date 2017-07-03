// https://github.com/KubaO/stackoverflown/tree/master/questions/opencv-21246766
#include <memory>

#include <QtWidgets>
#include <opencv2/opencv.hpp>

#include "annotatorwindow.h"
#include "bagreader.h"
#include "imageviewer.h"
#include "converter.h"
#include "timeline.hpp"
#include "timelineview.hpp"

Q_DECLARE_METATYPE(cv::Mat)

using namespace std;

class Thread final : public QThread { public: ~Thread() { quit(); wait(); } };

int main(int argc, char *argv[])
{
    qRegisterMetaType<cv::Mat>();
    QApplication app(argc, argv);

    AnnotatorWindow aw;

    // create and configure timeline
    auto timeline = make_shared<Timeline>();


    timeline->setSceneRect(-32000, -32000, 64000, 64000);


    TimelineView *timelineView = aw.findChild<TimelineView*>("timeline");

    timelineView->setScene(timeline.get());

    ImageViewer *envView = aw.findChild<ImageViewer*>("envView");
    ImageViewer *purpleView = aw.findChild<ImageViewer*>("purpleView");
    ImageViewer *yellowView = aw.findChild<ImageViewer*>("yellowView");
    Converter envConverter, purpleConverter, yellowConverter;


    BagReader bagreader;
    bagreader.loadBag("/home/skadge//freeplay_sandox/data/2017-05-18-145157833880/freeplay.bag");
    Thread captureThread, envConverterThread, purpleConverterThread, yellowConverterThread;
    // Everything runs at the same priority as the gui, so it won't supply useless frames.
    envConverter.setProcessAll(false);
    purpleConverter.setProcessAll(false);
    yellowConverter.setProcessAll(false);

    captureThread.start();
    bagreader.moveToThread(&captureThread);

    envConverterThread.start();
    purpleConverterThread.start();
    yellowConverterThread.start();

    envConverter.moveToThread(&envConverterThread);
    purpleConverter.moveToThread(&purpleConverterThread);
    yellowConverter.moveToThread(&yellowConverterThread);

    QObject::connect(&bagreader, &BagReader::envImgReady, &envConverter, &Converter::processFrame);
    QObject::connect(&envConverter, &Converter::imageReady, envView, &ImageViewer::setImage);

    QObject::connect(&bagreader, &BagReader::purpleImgReady, &purpleConverter, &Converter::processFrame);
    QObject::connect(&purpleConverter, &Converter::imageReady, purpleView, &ImageViewer::setImage);

    QObject::connect(&bagreader, &BagReader::yellowImgReady, &yellowConverter, &Converter::processFrame);
    QObject::connect(&yellowConverter, &Converter::imageReady, yellowView, &ImageViewer::setImage);

    aw.showFullScreen();

    QObject::connect(&bagreader, &BagReader::started, [](){ qDebug() << "capture started"; });

    //QMetaObject::invokeMethod(&capture, "start");
    QMetaObject::invokeMethod(&bagreader, "processBag");

    return app.exec();
}

//#include "main.moc"

