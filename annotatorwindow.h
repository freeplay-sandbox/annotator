#ifndef ANNOTATORWINDOW_H
#define ANNOTATORWINDOW_H

#include <QMainWindow>
#include <ros/time.h>

namespace Ui {
class AnnotatorWindow;
}

class AnnotatorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnnotatorWindow(QWidget *parent = 0);
    ~AnnotatorWindow();

    Q_SLOT void showFPS(ros::Time time);

private:
    Ui::AnnotatorWindow *ui;
};

#endif // ANNOTATORWINDOW_H
