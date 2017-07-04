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
          _color_background(QColor("#393939")),
          _color_playhead(QColor("#FF2F00")),
          _color_light(QColor("#2F2F2F")),
          _color_dark(QColor("#292929")),
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
}

void Timeline::setPlayhead(ros::Time time)
{
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

    auto pxPerSec = (size().width() - 2 * margin) * 1.0/ bagLength;

    // compute lines to draw and
    std::vector<QLine> lines_light;
    for (auto x = left; x <= right; x += pxPerSec * 60) {
            lines_light.push_back(QLine(x, top, x, bottom));
        //else
        //    lines_dark.push_back(QLine(x, top, x, bottom));
    }

    // nullspace lines
    std::vector<QLine> lines_null;
    lines_null.push_back(QLine(0, top, 0, bottom));
    lines_null.push_back(QLine(left, 0, right, 0));

    painter->fillRect(QRectF(left,top,right,bottom), _color_light);

    // draw calls
    painter->setPen(_pen_dark);
    painter->drawLines(lines_light.data(), lines_light.size());

    painter->setPen(_pen_null);
    painter->drawLines(lines_null.data(), lines_null.size());

    // playhead
    painter->setPen(_pen_playhead);
    painter->drawLine(QLine(elapsedTime * pxPerSec, top, elapsedTime * pxPerSec, bottom + 2));
}

void Timeline::keyPressEvent(QKeyEvent *event) {

    switch (event->key()) {
    ////// MISC DEBUG
    case Qt::Key_Enter:
    case Qt::Key_Return:
        cout << "Hello" << endl;
        QWidget::keyPressEvent(event);
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

    auto time = begin_ + (end_ - begin_) * (point.x() * 1.0/size().width());

    qDebug() << QString("Playhead time: %1").arg(time.toSec(),13, 'f', 1) << " sec";
    return time;
}
