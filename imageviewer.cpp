#include <QPainter>
#include <QDebug>

#include "imageviewer.h"

void ImageViewer::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawImage(0, 0, m_img.scaled(size(), Qt::KeepAspectRatio));
    m_img = {};
}

ImageViewer::ImageViewer(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void ImageViewer::setImage(const QImage &img) {
    if (!m_img.isNull()) qDebug() << "Viewer dropped frame!";
    m_img = img;
    if (m_img.size() != size()) {
        //setFixedSize(m_img.size());
    }
    update();
}
