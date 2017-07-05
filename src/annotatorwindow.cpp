#include <QDebug>
#include <QKeyEvent>

#include "annotatorwindow.h"
#include "ui_annotatorwindow.h"


AnnotatorWindow::AnnotatorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AnnotatorWindow)
{
    ui->setupUi(this);
}

AnnotatorWindow::~AnnotatorWindow()
{
    delete ui;
}

void AnnotatorWindow::showBagInfo(ros::Duration time)
{
    int nbSec = time.toSec();
    ui->statusBar->showMessage(QString("%1:%2").arg(nbSec / 60,2,10,QChar('0')).arg(nbSec % 60,2,10,QChar('0')));
}

void AnnotatorWindow::keyPressEvent(QKeyEvent *event) {

    switch (event->key()) {
    ////// MISC DEBUG
    case Qt::Key_Enter:
    case Qt::Key_Return:
        qDebug() << "Hi";
        QWidget::keyPressEvent(event);
        break;

        ////// NOT HANDLED -> pass forward
    default:
        QWidget::keyPressEvent(event);
    }
}
