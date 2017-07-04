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

void AnnotatorWindow::showFPS(ros::Time time)
{
    ui->statusBar->showMessage(QString("Bag Time: %1").arg(time.toSec(),13, 'f', 1) + " sec");
}
