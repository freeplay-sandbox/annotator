/* See LICENSE file for copyright and license details. */

#include <cmath>
#include <QColor>
#include <QPainter>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QDebug>
#include <algorithm>
#include <iostream>

using namespace std;

#include "timeline.hpp"

Timeline::Timeline(QWidget *parent):
          timescale_(1.),
          _color_background(QColor("#393939")),
          _color_playhead(QColor("#FF2F00")),
          _color_light(QColor("#7F7F7FAA")),
          _color_bg_text(QColor("#a1a1a1")),
          _pen_playhead(QPen(QBrush(_color_playhead), 2)),
          _pen_light(QPen(_color_light)),
          _brush_background(_color_background)
{
    // initialize default pen settings
    _pen_light.setWidth(0);
}

void Timeline::initialize(ros::Time begin, ros::Time end)
{
    begin_ = begin;
    current_ = begin;
    end_ = end;

    purpleAnnotations.add({AnnotationType::PASSIVE, current_, current_});
    yellowAnnotations.add({AnnotationType::PASSIVE, current_, current_});
}

void Timeline::setPlayhead(ros::Time time)
{

   purpleAnnotations.updateCurrentAnnotationEnd(time);
   yellowAnnotations.updateCurrentAnnotationEnd(time);
   generalAnnotations.updateCurrentAnnotationEnd(time);

   current_ = time;
   update();
}

void Timeline::newAnnotation(StreamType stream, AnnotationType annotationtype)
{

    Annotation a({annotationtype, current_, current_});

    switch (stream) {
    case StreamType::PURPLE:
        purpleAnnotations.add(a);
        break;
    case StreamType::YELLOW:
        yellowAnnotations.add(a);
        break;
    case StreamType::GLOBAL:
        generalAnnotations.add(a);
        break;
    default:
        break;
    }
}

void Timeline::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);


    drawTimeline(&painter, event->rect());

}

void Timeline::drawTimeline(QPainter *painter, const QRectF &rect) {

    const int margin = 20;
    auto left = static_cast<int>(std::floor(rect.left()));
    auto right = static_cast<int>(std::ceil(rect.right()));
    auto top = static_cast<int>(std::floor(rect.top()));
    auto bottom = static_cast<int>(std::ceil(rect.bottom())) - 20;

    auto bagLength = (end_ - begin_).toSec();
    auto elapsedTime = (current_ - begin_).toSec();

    double pxPerSec = (right - left) * 1.0/ bagLength * timescale_;

    double visibleDuration = (right - left) / pxPerSec;

    auto startTime = std::max(0., elapsedTime - visibleDuration  * 2./3);

    double exactNbSecs = startTime;
    int major_increment = 60; int minor_increment = 30;
    if (pxPerSec > 2) {major_increment = 30; minor_increment = 10;}
    if (pxPerSec > 3) {major_increment = 20; minor_increment = 5;}
    if (pxPerSec > 7) {major_increment = 10; minor_increment = 2;}
    if (pxPerSec > 30) {major_increment = 5; minor_increment = 1;}

    // compute lines to draw
    std::vector<QLine> lines_light;
    std::vector<QLine> lines_dark;
    for (double x = left; x <= right; x += pxPerSec * minor_increment) {

        int nbSecs = static_cast<int>(std::round(exactNbSecs));

        if(nbSecs % major_increment == 0) {
            lines_light.push_back(QLine(x, top, x, bottom + 10));
            painter->drawText(QPoint(x + 5, bottom + 20), QString("%1:%2").arg(nbSecs / 60,2,10,QChar('0')).arg(nbSecs % 60,2,10,QChar('0')));
        }
        else {
           lines_dark.push_back(QLine(x, top, x, bottom));
        }

        exactNbSecs += minor_increment;
    }

    int generalAnnotationHeight = top + 10;
    int purpleAnnotationHeight = top + 30;
    int yellowAnnotationHeight = top + 50;

    painter->fillRect(QRectF(left,top,right,bottom), _color_background);

    // annotation zones
    painter->fillRect(QRectF(left,generalAnnotationHeight - 5,right,10), _color_background.darker());
    painter->fillRect(QRectF(left,purpleAnnotationHeight - 5,right,10), QColor("#4c2d64"));
    painter->fillRect(QRectF(left,yellowAnnotationHeight - 5,right,10), QColor("#64592d"));

    // draw calls
    painter->setPen(_pen_light);
    painter->drawLines(lines_light.data(), lines_light.size());
    painter->setPen(QPen(_color_light.darker()));
    painter->drawLines(lines_dark.data(), lines_dark.size());


    // playhead
    painter->setPen(_pen_playhead);
    painter->drawLine(QLine(left + (elapsedTime - startTime) * pxPerSec, top, left + (elapsedTime - startTime) * pxPerSec, bottom));


    // Drawing of annotations
    int radius = 4;

    for(auto a : generalAnnotations) {
        auto start = std::max(0., (a->start - begin_).toSec() - startTime);
        auto stop = std::min((a->stop - begin_).toSec() - startTime, startTime + visibleDuration);

        painter->setPen(Annotation::Styles[a->type]);
        painter->setBrush(Annotation::Styles[a->type].brush());
        painter->drawEllipse(left - radius/2 + start * pxPerSec, generalAnnotationHeight - radius/2, radius,radius);
        painter->drawLine(QLine(left + start * pxPerSec, generalAnnotationHeight, left + stop * pxPerSec, generalAnnotationHeight));
    }
   for(auto a : purpleAnnotations) {
        auto start = std::max(0., (a->start - begin_).toSec() - startTime);
        auto stop = std::min((a->stop - begin_).toSec() - startTime, startTime + visibleDuration);

        painter->setPen(Annotation::Styles[a->type]);
        painter->setBrush(Annotation::Styles[a->type].brush());
        painter->drawEllipse(left - radius/2 + start * pxPerSec, purpleAnnotationHeight - radius/2, radius,radius);
        painter->drawLine(QLine(left + start * pxPerSec, purpleAnnotationHeight, left + stop * pxPerSec, purpleAnnotationHeight));
    }

    for(auto a : yellowAnnotations) {
        auto start = std::max(0., (a->start - begin_).toSec() - startTime);
        auto stop = std::min((a->stop - begin_).toSec() - startTime, startTime + visibleDuration);

        painter->setPen(Annotation::Styles[a->type]);
        painter->setBrush(Annotation::Styles[a->type].brush());
        painter->drawEllipse(left - radius/2 + start * pxPerSec, yellowAnnotationHeight - radius/2, radius,radius);
        painter->drawLine(QLine(left + start * pxPerSec, yellowAnnotationHeight, left + stop * pxPerSec, yellowAnnotationHeight));
    }
}

void Timeline::keyPressEvent(QKeyEvent *event) {

    switch (event->key()) {
    ////// MISC DEBUG
    case Qt::Key_Enter:
    case Qt::Key_Return:
        cout << "Hello" << endl;
        QWidget::keyPressEvent(event);
        break;

    case Qt::Key_Up:
        timescale_ *= 1.1;
        if (timescale_ > 50.) timescale_ = 50.;
        break;
    case Qt::Key_Down:
        timescale_ *= 0.9;
        if (timescale_ < 1.) timescale_ = 1.;
        break;

    case Qt::Key_Q:
        purpleAnnotations.add({AnnotationType::PROSOCIAL, current_, current_});
        break;
    case Qt::Key_W:
        purpleAnnotations.add({AnnotationType::HOSTILE, current_, current_});
        break;
    case Qt::Key_E:
        purpleAnnotations.add({AnnotationType::ASSERTIVE, current_, current_});
        break;
    case Qt::Key_A:
        purpleAnnotations.add({AnnotationType::PASSIVE, current_, current_});
        break;
    case Qt::Key_S:
        purpleAnnotations.add({AnnotationType::ADULTSEEKING, current_, current_});
        break;
     case Qt::Key_D:
        purpleAnnotations.add({AnnotationType::IRRELEVANT, current_, current_});
        break;

    case Qt::Key_U:
        yellowAnnotations.add({AnnotationType::PROSOCIAL, current_, current_});
        break;
    case Qt::Key_I:
        yellowAnnotations.add({AnnotationType::HOSTILE, current_, current_});
        break;
    case Qt::Key_O:
        yellowAnnotations.add({AnnotationType::ASSERTIVE, current_, current_});
        break;
    case Qt::Key_J:
        yellowAnnotations.add({AnnotationType::PASSIVE, current_, current_});
        break;
    case Qt::Key_K:
        yellowAnnotations.add({AnnotationType::ADULTSEEKING, current_, current_});
        break;
     case Qt::Key_L:
        yellowAnnotations.add({AnnotationType::IRRELEVANT, current_, current_});
        break;

        ////// NOT HANDLED -> pass forward
    default:
        QWidget::keyPressEvent(event);
    }
}

void Timeline::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
            auto lastPoint = event->pos();
            emit timeJump(pointToTimestamp(lastPoint));
    }
}


void Timeline::wheelEvent(QWheelEvent *event) {
        //setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        double scaleFactor = 1.25;
        if (event->delta() > 0) {
            // zoom in
            timescale_ *= scaleFactor;
            if (timescale_ > 50.) timescale_ = 50.;
        } else {
            // zoom out
            timescale_ /= scaleFactor;
            if (timescale_ < 1.) timescale_ = 1.;
        }
        event->accept();
}

ros::Time Timeline::pointToTimestamp(QPoint point) {

    return begin_ + (end_ - begin_) * (point.x()/timescale_ * 1.0/size().width());
}
