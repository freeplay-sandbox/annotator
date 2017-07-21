#ifndef ANNOTATORWINDOW_H
#define ANNOTATORWINDOW_H

#include <QMainWindow>
#include <QLabel>
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

    Q_SLOT void showBagInfo(ros::Duration time);
    Q_SLOT void showAutosavePath(QString path);

    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::AnnotatorWindow *ui;

    QLabel bagInfo;
    QLabel autosaveInfo;
};

#endif // ANNOTATORWINDOW_H
