#ifndef ANNOTATORWINDOW_H
#define ANNOTATORWINDOW_H

#include <QMainWindow>

namespace Ui {
class AnnotatorWindow;
}

class AnnotatorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnnotatorWindow(QWidget *parent = 0);
    ~AnnotatorWindow();

    Q_SLOT void showFPS(const QString& time);

private:
    Ui::AnnotatorWindow *ui;
};

#endif // ANNOTATORWINDOW_H
