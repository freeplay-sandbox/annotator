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

private:
    Ui::AnnotatorWindow *ui;
};

#endif // ANNOTATORWINDOW_H
