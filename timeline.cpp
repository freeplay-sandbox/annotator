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
          _color_light(QColor("#7F7F7F")),
          _color_dark(QColor("#191919")),
          _color_null(QColor("#212121")),
          _color_bg_text(QColor("#a1a1a1")),
          _pen_playhead(QPen(QBrush(_color_playhead), 2)),
          _pen_light(QPen(_color_light)),
          _pen_dark(QPen(_color_dark)),
          _pen_null(QPen(_color_null)),
          _brush_background(_color_background)
{
    // initialize default pen settings
    for (auto p : {&_pen_light, &_pen_dark, &_pen_null}) {
        p->setWidth(0);
    }
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

   current_ = time;
   update();
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

    auto pxPerSec = (right - left) * 1.0/ bagLength;

    // compute lines to draw and
    std::vector<QLine> lines_light;
    int nbSec = 0;
    for (auto x = left; x <= right; x += pxPerSec * 60) {

        lines_light.push_back(QLine(x, top, x, bottom + 5));
        painter->drawText(QPoint(x + 5, bottom + 20), QString("%1:%2").arg(nbSec / 60,2,10,QChar('0')).arg(nbSec % 60,2,10,QChar('0')));

        nbSec += 60;
    }

    int generalAnnotationHeight = top + 10;
    int purpleAnnotationHeight = top + 30;
    int yellowAnnotationHeight = top + 50;

    painter->fillRect(QRectF(left,top,right,bottom), _color_background);
    painter->fillRect(QRectF(left,generalAnnotationHeight - 5,right,10), _color_background.darker());
    painter->fillRect(QRectF(left,purpleAnnotationHeight - 5,right,10), QColor("#4c2d64"));
    painter->fillRect(QRectF(left,yellowAnnotationHeight - 5,right,10), QColor("#64592d"));

    // draw calls
    painter->setPen(_pen_light);
    painter->drawLines(lines_light.data(), lines_light.size());


    // playhead
    painter->setPen(_pen_playhead);
    painter->drawLine(QLine(left + elapsedTime * pxPerSec, top, left + elapsedTime * pxPerSec, bottom));


    // Drawing of annotations
    int radius = 4;

    for(auto a : generalAnnotations) {
        auto start = (a->start - begin_).toSec();
        auto stop = (a->stop - begin_).toSec();

        painter->setPen(Annotation::Styles[a->type]);
        painter->setBrush(Annotation::Styles[a->type].brush());
        painter->drawEllipse(left - radius/2 + start * pxPerSec, generalAnnotationHeight - radius/2, radius,radius);
        painter->drawLine(QLine(left + start * pxPerSec, generalAnnotationHeight, left + stop * pxPerSec, generalAnnotationHeight));
    }
   for(auto a : purpleAnnotations) {
        auto start = (a->start - begin_).toSec();
        auto stop = (a->stop - begin_).toSec();

        painter->setPen(Annotation::Styles[a->type]);
        painter->setBrush(Annotation::Styles[a->type].brush());
        painter->drawEllipse(left - radius/2 + start * pxPerSec, purpleAnnotationHeight - radius/2, radius,radius);
        painter->drawLine(QLine(left + start * pxPerSec, purpleAnnotationHeight, left + stop * pxPerSec, purpleAnnotationHeight));
    }

    for(auto a : yellowAnnotations) {
        auto start = (a->start - begin_).toSec();
        auto stop = (a->stop - begin_).toSec();

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
        break;
    case Qt::Key_Down:
        timescale_ *= 0.9;
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

ros::Time Timeline::pointToTimestamp(QPoint point) {

    return begin_ + (end_ - begin_) * (point.x() * 1.0/size().width());
}
