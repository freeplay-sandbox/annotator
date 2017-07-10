#ifndef FREEANNOTATIONWIDGET_H
#define FREEANNOTATIONWIDGET_H

#include <string>
#include <QLabel>
#include <ros/time.h>

enum class FreeAnnotationType {NOTE=0,
                               INTERESTING,
                               ISSUE,
                               SCREENSHOT};

class FreeAnnotationWidget : public QLabel
{
    Q_OBJECT
public:
    explicit FreeAnnotationWidget(ros::Time atime, FreeAnnotationType type, const std::string& content, QWidget* parent = nullptr);

    virtual ~FreeAnnotationWidget() {}

    ros::Time time;
    std::string content;
    FreeAnnotationType type;

    // QWidget interface
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;

};

#endif // FREEANNOTATIONWIDGET_H
