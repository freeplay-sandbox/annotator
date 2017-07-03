#ifndef CONVERTER_H
#define CONVERTER_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QBasicTimer>

class Converter : public QObject {
    Q_OBJECT

    QBasicTimer m_timer;
    cv::Mat m_frame;
    bool m_processAll = true;

    static void matDeleter(void* mat);

    void queue(const cv::Mat & frame);

    void process(cv::Mat frame);

    void timerEvent(QTimerEvent * ev);

public:
    explicit Converter(QObject * parent = nullptr);
    void setProcessAll(bool all);
    Q_SIGNAL void imageReady(const QImage &);
    Q_SLOT void processFrame(const cv::Mat & frame);
};


#endif // CONVERTER_H
