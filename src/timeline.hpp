#ifndef _TIMELINE_HPP
#define _TIMELINE_HPP

#include <set>
#include <memory>

#include <QWidget>
#include <QPen>

#include <ros/time.h>

#include "annotation.hpp"

class Timeline : public QWidget {
    Q_OBJECT

   public:
    Timeline(QWidget* parent = nullptr);

    Q_SIGNAL void timeJump(ros::Time timepoint);

    Q_SLOT void initialize(ros::Time begin, ros::Time end);
    Q_SLOT void setPlayhead(ros::Time time);

    Q_SLOT void newAnnotation(StreamType stream, AnnotationType annotation);

   protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

   private:

    double timescale_;

    ros::Time begin_, end_, current_;

    QColor _color_background;
    QColor _color_playhead;
    QColor _color_light;
    QColor _color_dark;
    QColor _color_null;
    QColor _color_bg_text;

    QPen _pen_playhead;
    QPen _pen_light;
    QPen _pen_dark;
    QPen _pen_null;

    QBrush _brush_background;

    void drawTimeline(QPainter *painter, const QRectF &rect);
    ros::Time pointToTimestamp(QPoint point);

    Annotations generalAnnotations;
    Annotations purpleAnnotations;
    Annotations yellowAnnotations;
};

#endif
