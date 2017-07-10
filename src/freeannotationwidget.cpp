#include <QDebug>

#include "freeannotationwidget.hpp"

FreeAnnotationWidget::FreeAnnotationWidget(ros::Time time, FreeAnnotationType type, const std::string &content, QWidget* parent):
    QLabel(parent),
    time(time),
    type(type),
    content(content)
{
    setText(QString::fromStdString(content));
    setStyleSheet("QLabel { background-color : red; color : blue; }");

}

void FreeAnnotationWidget::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << "Mouse on annotation";

}
