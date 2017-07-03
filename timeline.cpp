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

Timeline::Timeline(QObject *parent)
    : QGraphicsScene(parent),
      dontGrabKeyPresses(false),
      _paintBackground(true),
      _color_background(QColor("#393939")),
      _color_light(QColor("#2F2F2F")),
      _color_dark(QColor("#292929")),
      _color_null(QColor("#212121")),
      _color_bg_text(QColor("#a1a1a1")),
      _pen_light(QPen(_color_light)),
      _pen_dark(QPen(_color_dark)),
      _pen_null(QPen(_color_null)),
      _brush_background(_color_background)
{
    // initialize default pen settings
    for (auto p : {&_pen_light, &_pen_dark, &_pen_null}) {
        p->setWidth(0);
    }

    // initialize the background
    setBackgroundBrush(_brush_background);

}

void Timeline::drawBackground(QPainter *painter, const QRectF &rect) {

    if(!_paintBackground) return;

    // call parent method
    QGraphicsScene::drawBackground(painter, rect);

    // augment the painted with grid
    const int gridsize = 20;
    auto left = static_cast<int>(std::floor(rect.left()));
    auto right = static_cast<int>(std::ceil(rect.right()));
    auto top = static_cast<int>(std::floor(rect.top()));
    auto bottom = static_cast<int>(std::ceil(rect.bottom()));

    // compute indices of lines to draw
    const auto first_left = left - (left % gridsize);
    const auto first_top = top - (top % gridsize);

    // compute lines to draw and
    std::vector<QLine> lines_light;
    std::vector<QLine> lines_dark;
    for (auto x = first_left; x <= right; x += gridsize) {
        if (x % 100 != 0)
            lines_light.push_back(QLine(x, top, x, bottom));
        else
            lines_dark.push_back(QLine(x, top, x, bottom));
    }
    for (auto y = first_top; y <= bottom; y += gridsize) {
        if (y % 100 != 0)
            lines_light.push_back(QLine(left, y, right, y));
        else
            lines_dark.push_back(QLine(left, y, right, y));
    }

    // nullspace lines
    std::vector<QLine> lines_null;
    lines_null.push_back(QLine(0, top, 0, bottom));
    lines_null.push_back(QLine(left, 0, right, 0));

    // draw calls
    painter->setPen(_pen_light);
    painter->drawLines(lines_light.data(), lines_light.size());

    painter->setPen(_pen_dark);
    painter->drawLines(lines_dark.data(), lines_dark.size());

    painter->setPen(_pen_null);
    painter->drawLines(lines_null.data(), lines_null.size());
}

void Timeline::keyPressEvent(QKeyEvent *event) {
    if (dontGrabKeyPresses) {
        QGraphicsScene::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
        ////// MISC DEBUG
        case Qt::Key_Enter:
        case Qt::Key_Return:
            cout << "Hello" << endl;
            QGraphicsScene::keyPressEvent(event);
            break;

        ////// NOT HANDLED -> pass forward
        default:
            QGraphicsScene::keyPressEvent(event);
    }
}
