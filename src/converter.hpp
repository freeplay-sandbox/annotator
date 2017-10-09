#ifndef CONVERTER_H
#define CONVERTER_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QBasicTimer>

enum RotateCode {ROTATE_90_CLOCKWISE, ROTATE_180, ROTATE_90_COUNTERCLOCKWISE};

class Converter : public QObject {
    Q_OBJECT

    QBasicTimer m_timer;
    cv::Mat m_frame;
    bool m_processAll = true;

    static void matDeleter(void* mat);

    void queue(const cv::Mat & frame);

    void process(cv::Mat frame);

    void timerEvent(QTimerEvent * ev);

    bool rotate_;
    RotateCode rotateCode_;

public:
    explicit Converter(QObject * parent = nullptr);
    void setProcessAll(bool all);
    void applyRotation(RotateCode rotateCode) {rotate_=true; rotateCode_=rotateCode;}
    Q_SIGNAL void imageReady(const QImage &);
    Q_SLOT void processFrame(const cv::Mat & frame);
};


#endif // CONVERTER_H
