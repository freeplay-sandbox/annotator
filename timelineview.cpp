
#include <QWheelEvent>
#include <QScrollBar>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QGraphicsItem>

#include <iostream>


#include "timelineview.hpp"

TimelineView::TimelineView(QWidget *parent)
    : TimelineView(nullptr, parent) {}

TimelineView::TimelineView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent),
      _panning(true)
{
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                   QPainter::HighQualityAntialiasing |
                   QPainter::SmoothPixmapTransform);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setResizeAnchor(NoAnchor);
    setTransformationAnchor(AnchorUnderMouse);
}

void TimelineView::resizeEvent(QResizeEvent *event) {
    // always have the origin in the top left corner
    static bool first_resize = true;
    if (first_resize) {
        // TODO: scale awareness
        centerOn(width() / 2 - 50, height() / 2 - 50);
        first_resize = false;
    }
    QGraphicsView::resizeEvent(event);
}

void TimelineView::wheelEvent(QWheelEvent *event) {
    //if (event->modifiers() & Qt::ControlModifier) {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        double scaleFactor = 1.25;
        if (event->delta() > 0) {
            // zoom in
            scale(scaleFactor, scaleFactor);
        } else {
            // zoom out
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
        event->accept();
    //} else {
    //    QGraphicsView::wheelEvent(event);
    //}
}

void TimelineView::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
        case Qt::MiddleButton:
            middleMouseButtonPress(event);
            break;
        case Qt::LeftButton:
            leftMouseButtonPress(event);
            break;
        default:
            QGraphicsView::mousePressEvent(event);
    }
}

void TimelineView::middleMouseButtonRelease(QMouseEvent *event) {
    QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(),
                          event->windowPos(), Qt::LeftButton,
                          event->buttons() & ~Qt::LeftButton,
                          event->modifiers());
    QGraphicsView::mouseReleaseEvent(&fakeEvent);
    setDragMode(QGraphicsView::NoDrag);
}

void TimelineView::leftMouseButtonRelease(QMouseEvent *event) {
//    if (_drag_event) {
//        auto sock = socket_at(event->pos());
//        if (!sock || !can_accept_edge(sock)) {
//            _drag_event->e->disconnect();
//        } else {
//            switch (_drag_event->mode) {
//                case EdgeDragEvent::move_to_source:
//                case EdgeDragEvent::connect_to_source:
//                    _drag_event->e->connect_source(sock);
//                    break;
//
//                case EdgeDragEvent::move_to_sink:
//                case EdgeDragEvent::connect_to_sink:
//                    _drag_event->e->connect_sink(sock);
//                    break;
//            }
//        }
//        delete _drag_event;
//        _drag_event = nullptr;
//    } else if (_resize_event) {
//        // TODO: finally set the width/height of the node ?
//        delete _resize_event;
//        _resize_event = nullptr;
//        setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
//    }
    QGraphicsView::mouseReleaseEvent(event);
    setDragMode(QGraphicsView::NoDrag);
}

void TimelineView::mouseReleaseEvent(QMouseEvent *event) {
    viewport()->setCursor(Qt::ArrowCursor);

    switch (event->button()) {
        case Qt::MiddleButton:
            middleMouseButtonRelease(event);
            break;
        case Qt::LeftButton:
            leftMouseButtonRelease(event);
            break;
        default:
            QGraphicsView::mouseReleaseEvent(event);
    }
}

void TimelineView::mouseMoveEvent(QMouseEvent *event) {
//    // if the left mouse button was pressed and we actually have a mode and
//    // temporary edge already set
//    if (_drag_event && (event->buttons() & Qt::LeftButton)) {
//        // set start/stop depending on drag mode
//        switch (_drag_event->mode) {
//            case EdgeDragEvent::move_to_source:
//            case EdgeDragEvent::connect_to_source:
//                _drag_event->e->set_start(mapToScene(event->pos()));
//                break;
//
//            case EdgeDragEvent::move_to_sink:
//            case EdgeDragEvent::connect_to_sink:
//                _drag_event->e->set_stop(mapToScene(event->pos()));
//                break;
//        }
//
//        // update visual feedback
//        auto item = socket_at(event->pos());
//        if (!item) {
//            viewport()->setCursor(Qt::ClosedHandCursor);
//        } else if (item->type() == GraphicsNodeItemTypes::TypeSocket) {
//            GraphicsNodeSocket *sock = static_cast<GraphicsNodeSocket *>(item);
//            if (!can_accept_edge(sock))
//                viewport()->setCursor(Qt::ForbiddenCursor);
//            else {
//                viewport()->setCursor(Qt::DragMoveCursor);
//            }
//        }
//    } else if (_resize_event && (event->buttons() & Qt::LeftButton)) {
//        QPointF size =
//            mapToScene(event->pos()) - _resize_event->node->mapToScene(0, 0);
//        _resize_event->node->setSize(size);
//    } else {
//        // no button is pressed, so indicate what the user can do with
//        // the item by changing the cursor
//        if (event->buttons() == 0) {
//            auto item = socket_at(event->pos());
//            if (item) {
//                QPointF scenePos = mapToScene(event->pos());
//                QPointF itemPos = item->mapFromScene(scenePos);
//                if (item->isInSocketCircle(itemPos))
//                    viewport()->setCursor(Qt::OpenHandCursor);
//                else
//                    viewport()->setCursor(Qt::ArrowCursor);
//            } else
//                viewport()->setCursor(Qt::ArrowCursor);
//        }
        QGraphicsView::mouseMoveEvent(event);
//    }
}

void TimelineView::leftMouseButtonPress(QMouseEvent *event) {
    QGraphicsView::mousePressEvent(event);
//    // GUI logic: if we click on a socket, we need to handle
//    // the event appropriately
//    QGraphicsItem *item = itemAt(event->pos());
//    if (!item) {
    if (event->modifiers() & Qt::ControlModifier) {
        setDragMode(QGraphicsView::RubberBandDrag);
    } else {
        setDragMode(QGraphicsView::ScrollHandDrag);
    }
    QGraphicsView::mousePressEvent(event);
    return;
//    }
//
//    switch (item->type()) {
//        case GraphicsNodeItemTypes::TypeSocket: {
//            QPointF scenePos = mapToScene(event->pos());
//            QPointF itemPos = item->mapFromScene(scenePos);
//            GraphicsNodeSocket *sock = static_cast<GraphicsNodeSocket *>(item);
//            if (sock->isInSocketCircle(itemPos)) {
//                viewport()->setCursor(Qt::ClosedHandCursor);
//
//                // initialize a new drag mode event
//                _drag_event = new EdgeDragEvent();
//                auto edges = sock->get_edges();
//                if (edges.size() == 1) {
//                    // get the first edge
//                    for (auto e : edges) {
//                        _tmp_edge = e;
//                        break;
//                    }
//
//                    _drag_event->e = _tmp_edge;
//                    if (sock->socket_type() == Port::Direction::IN) {
//                        _drag_event->e->disconnect_sink();
//                        _drag_event->e->set_stop(mapToScene(event->pos()));
//                        _drag_event->mode = EdgeDragEvent::move_to_sink;
//                    } else {
//                        _drag_event->e->disconnect_source();
//                        _drag_event->e->set_start(mapToScene(event->pos()));
//                        _drag_event->mode = EdgeDragEvent::move_to_source;
//                    }
//                } else {
//                    Timeline *gscene =
//                        dynamic_cast<Timeline *>(scene());
//
//                    _drag_event->e = gscene->make_edge();
//
//                    if (sock->socket_type() == Port::Direction::IN) {
//                        _drag_event->e->set_start(mapToScene(event->pos()));
//                        _drag_event->e->connect_sink(sock);
//                        _drag_event->mode = EdgeDragEvent::connect_to_source;
//                    } else {
//                        _drag_event->e->connect_source(sock);
//                        _drag_event->e->set_stop(mapToScene(event->pos()));
//                        _drag_event->mode = EdgeDragEvent::connect_to_sink;
//                    }
//                }
//                event->ignore();
//            } else {
//                QGraphicsView::mousePressEvent(event);
//            }
//            break;
//        }
//
//        case GraphicsNodeItemTypes::TypeNode: {
//            QPointF scenePos = mapToScene(event->pos());
//            QPointF itemPos = item->mapFromScene(scenePos);
//            GraphicsNode *node = static_cast<GraphicsNode *>(item);
//
//            if (itemPos.x() > (node->width() - 10) &&
//                (itemPos.y() > (node->height() - 10))) {
//                setViewportUpdateMode(
//                    QGraphicsView::BoundingRectViewportUpdate);
//                _resize_event = new NodeResizeEvent();
//                _resize_event->node = node;
//                _resize_event->orig_width = node->width();
//                _resize_event->orig_height = node->height();
//                _resize_event->pos = event->pos();
//                event->ignore();
//            } else {
//                QGraphicsView::mousePressEvent(event);
//            }
//            break;
//        }
//
//        default:
//            QGraphicsView::mousePressEvent(event);
//    }
}

void TimelineView::middleMouseButtonPress(QMouseEvent *event) {
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(),
                             event->screenPos(), event->windowPos(),
                             Qt::LeftButton, 0, event->modifiers());
    QGraphicsView::mouseReleaseEvent(&releaseEvent);
    setDragMode(QGraphicsView::ScrollHandDrag);
    QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(),
                          event->windowPos(), Qt::LeftButton,
                          event->buttons() | Qt::LeftButton,
                          event->modifiers());
    QGraphicsView::mousePressEvent(&fakeEvent);
}
