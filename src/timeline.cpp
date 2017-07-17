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
          _brush_background(_color_background)
{
}

void Timeline::initialize(ros::Time begin, ros::Time end)
{
    begin_ = begin;
    current_ = begin;
    end_ = end;

    purpleAnnotations.add({AnnotationType::PASSIVE, current_, current_});
    yellowAnnotations.add({AnnotationType::PASSIVE, current_, current_});

    freeAnnotations.push_back(make_shared<FreeAnnotationWidget>(begin_ + ros::Duration(10), FreeAnnotationType::INTERESTING, "hello world"));
    freeAnnotations.push_back(make_shared<FreeAnnotationWidget>(begin_ + ros::Duration(20), FreeAnnotationType::ISSUE, "hello world issue"));

    for(auto freeannotation : freeAnnotations) {
        freeannotation->setParent(this);
        freeannotation->show();
    }
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

    auto& rect = event->rect();

    auto left = rect.left();
    auto right = rect.right();
    auto top = rect.top();
    auto bottom = rect.bottom() - 20;

    generalAnnotationOffset_ = top + 10;
    purpleAnnotationOffset_ = top + 30;
    yellowAnnotationOffset_ = top + 50;

    auto bagLength = (end_ - begin_).toSec();

    pxPerSec_ = (right - left) * 1.0/ bagLength * timescale_;
    visibleDuration_ = (right - left) / pxPerSec_;
    elapsedTime_ = (current_ - begin_).toSec();
    startTime_ = std::min(std::max(0., elapsedTime_ - visibleDuration_  * 2./3), ((end_-begin_).toSec() - visibleDuration_));

    drawTimeline(&painter, left, right, top, bottom);

    //placeFreeAnnotations();

}

void Timeline::placeFreeAnnotations() {

    // placement of free annotations
    for(auto freeannotation : freeAnnotations) {
        auto atime = (freeannotation->time - begin_).toSec();

        if ( atime > startTime_ && atime < startTime_ + visibleDuration_) {
            freeannotation->move(atime * pxPerSec_, generalAnnotationOffset_);
            if(!freeannotation->isVisible()) freeannotation->show();
        }
        else {
            if(freeannotation->isVisible()) freeannotation->hide();
        }
    }


}

void Timeline::drawTimeline(QPainter *painter, int left, int right, int top, int bottom) {


    int major_increment = 60; int minor_increment = 30;
    if (pxPerSec_ > 2) {major_increment = 30; minor_increment = 10;}
    if (pxPerSec_ > 3) {major_increment = 20; minor_increment = 5;}
    if (pxPerSec_ > 7) {major_increment = 10; minor_increment = 2;}
    if (pxPerSec_ > 30) {major_increment = 5; minor_increment = 1;}

    // compute lines to draw

    std::vector<QLine> lines_light;
    std::vector<QLine> lines_dark;

    painter->setPen(QPen(_color_light));

    double offset = major_increment - fmod(startTime_, major_increment);

    for (double t = startTime_ + offset; t <= visibleDuration_ + startTime_; t += major_increment) {

        auto x = (t - startTime_) * pxPerSec_;

            lines_light.push_back(QLine(x, top, x, bottom + 20));
            painter->drawText(QPoint(x + 2, bottom + 20), QString("%1:%2").arg(static_cast<int>(round(t)) / 60,2,10,QChar('0')).arg(static_cast<int>(round(t)) % 60,2,10,QChar('0')));

            for (auto tm = t + minor_increment; tm < t + major_increment; tm += minor_increment) {
                auto x = (tm - startTime_) * pxPerSec_;
                lines_dark.push_back(QLine(x, top, x, bottom));
            }

    }


    painter->fillRect(QRectF(left,top,right,bottom), _color_background);

    // annotation zones
    painter->fillRect(QRectF(left,generalAnnotationOffset_ - 5,right,10), _color_background.darker());
    painter->fillRect(QRectF(left,purpleAnnotationOffset_ - 5,right,10), QColor("#4c2d64"));
    painter->fillRect(QRectF(left,yellowAnnotationOffset_ - 5,right,10), QColor("#64592d"));

    // draw calls
    painter->setPen(QPen(_color_light));
    painter->drawLines(lines_light.data(), lines_light.size());
    painter->setPen(QPen(_color_light.darker()));
    painter->drawLines(lines_dark.data(), lines_dark.size());


    // playhead
    painter->setPen(QPen(_color_playhead, 2));
    painter->drawLine(QLine(left + (elapsedTime_ - startTime_) * pxPerSec_, top, left + (elapsedTime_ - startTime_) * pxPerSec_, bottom));


    // Drawing of annotations
    int radius = 4;

    for(auto a : generalAnnotations) {
        auto start = std::max(0., (a->start - begin_).toSec() - startTime_);
        auto stop = std::min((a->stop - begin_).toSec() - startTime_, startTime_ + visibleDuration_);

        painter->setPen(Annotation::Styles[a->type]);
        painter->setBrush(Annotation::Styles[a->type].brush());
        painter->drawEllipse(left - radius/2 + start * pxPerSec_, generalAnnotationOffset_ - radius/2, radius,radius);
        painter->drawLine(QLine(left + start * pxPerSec_, generalAnnotationOffset_, left + stop * pxPerSec_, generalAnnotationOffset_));
    }

    for(auto a : purpleAnnotations) {
        auto start = std::max(0., (a->start - begin_).toSec() - startTime_);
        auto stop = std::min((a->stop - begin_).toSec() - startTime_, startTime_ + visibleDuration_);

        painter->setPen(Annotation::Styles[a->type]);
        painter->setBrush(Annotation::Styles[a->type].brush());
        painter->drawEllipse(left - radius/2 + start * pxPerSec_, purpleAnnotationOffset_ - radius/2, radius,radius);
        painter->drawLine(QLine(left + start * pxPerSec_, purpleAnnotationOffset_, left + stop * pxPerSec_, purpleAnnotationOffset_));
    }

    for(auto a : yellowAnnotations) {
        auto start = std::max(0., (a->start - begin_).toSec() - startTime_);
        auto stop = std::min((a->stop - begin_).toSec() - startTime_, startTime_ + visibleDuration_);

        painter->setPen(Annotation::Styles[a->type]);
        painter->setBrush(Annotation::Styles[a->type].brush());
        painter->drawEllipse(left - radius/2 + start * pxPerSec_, yellowAnnotationOffset_ - radius/2, radius,radius);
        painter->drawLine(QLine(left + start * pxPerSec_, yellowAnnotationOffset_, left + stop * pxPerSec_, yellowAnnotationOffset_));
    }

}

void Timeline::keyPressEvent(QKeyEvent *event) {

    switch (event->key()) {
    case Qt::Key_Space:
        emit togglePause();
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

        update();
}

ros::Time Timeline::pointToTimestamp(QPoint point) {

    return begin_ + (end_ - begin_) * (point.x()/timescale_ * 1.0/size().width());
}
