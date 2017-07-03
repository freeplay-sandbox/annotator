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
