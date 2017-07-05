#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>

class ImageViewer : public QWidget {
    Q_OBJECT
    QImage m_img;
    void paintEvent(QPaintEvent *);
public:
    ImageViewer(QWidget * parent = nullptr);
    Q_SLOT void setImage(const QImage & img);
};


#endif // IMAGEVIEWER_H
