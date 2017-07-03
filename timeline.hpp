#ifndef __GRAPHICSNODESCENE_HPP__7F9E4C1E_8F4E_4BD2_BDF7_3D4ECEC206B5
#define __GRAPHICSNODESCENE_HPP__7F9E4C1E_8F4E_4BD2_BDF7_3D4ECEC206B5

#include <set>
#include <memory>

#include <QGraphicsScene>

class Timeline : public QGraphicsScene {
    Q_OBJECT

   public:
    Timeline(QObject* parent = 0);

    bool dontGrabKeyPresses;

   protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

   private:
    QColor _color_background;
    QColor _color_light;
    QColor _color_dark;
    QColor _color_null;
    QColor _color_bg_text;

    bool _paintBackground;

    QPen _pen_light;
    QPen _pen_dark;
    QPen _pen_null;

    QBrush _brush_background;

};

#endif /* __GRAPHICSNODESCENE_HPP__7F9E4C1E_8F4E_4BD2_BDF7_3D4ECEC206B5 */
