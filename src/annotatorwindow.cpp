#include <QDebug>
#include <QKeyEvent>

#include "annotatorwindow.hpp"
#include "ui_annotatorwindow.h"


AnnotatorWindow::AnnotatorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AnnotatorWindow)
{
    ui->setupUi(this);

    ui->statusBar->addWidget(&bagInfo);
    autosaveInfo.setAlignment(Qt::AlignRight);
    ui->statusBar->addWidget(&autosaveInfo, 1);
}

AnnotatorWindow::~AnnotatorWindow()
{
    delete ui;
}

void AnnotatorWindow::showBagInfo(ros::Duration time)
{
    int nbSec = time.toSec();
    bagInfo.setText(QString("%1:%2 (%3s)").arg(nbSec / 60,2,10,QChar('0')).arg(nbSec % 60,2,10,QChar('0')).arg(time.toSec()));
}

void AnnotatorWindow::showAutosavePath(QString path)
{
    autosaveInfo.setText(QString("Auto-saving to ") + path);

}

void AnnotatorWindow::keyPressEvent(QKeyEvent *event) {

    switch (event->key()) {
    ////// MISC DEBUG
    case Qt::Key_F11:
        if (windowState() & Qt::WindowFullScreen)
            setWindowState(Qt::WindowNoState);
        else
            setWindowState(Qt::WindowFullScreen);
        break;

        ////// NOT HANDLED -> pass forward
    default:
        QWidget::keyPressEvent(event);
    }
}
