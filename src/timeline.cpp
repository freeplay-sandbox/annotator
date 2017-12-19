/* See LICENSE file for copyright and license details. */

#include <cmath>
#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QMessageBox>
#include <QDebug>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <QFileDialog>

#include <yaml-cpp/yaml.h>

#include "timeline.hpp"

using namespace std;


Timeline::Timeline(QWidget *parent):
          timescale_(1.),
          autosaveTimer(this),
          annotationPath("/tmp/freeplay-annotations.yaml"),
          _color_background(QColor("#393939")),
          _color_playhead(QColor("#FF2F00")),
          _color_light(QColor("#7F7F7FAA")),
          _color_bg_text(QColor("#a1a1a1")),
          _brush_background(_color_background),
          mergeMode(false)
{
    connect(&autosaveTimer, &QTimer::timeout, [&](){saveToFile("");});
    autosaveTimer.start(1000);
}

void Timeline::initialize(ros::Time begin, ros::Time end)
{
    begin_ = begin;
    current_ = begin;
    end_ = end;


    //freeAnnotations.push_back(make_shared<FreeAnnotationWidget>(begin_ + ros::Duration(10), FreeAnnotationType::INTERESTING, "hello world"));
    //freeAnnotations.push_back(make_shared<FreeAnnotationWidget>(begin_ + ros::Duration(20), FreeAnnotationType::ISSUE, "hello world issue"));

    for(auto freeannotation : freeAnnotations) {
        freeannotation->setParent(this);
        freeannotation->show();
    }
}

void Timeline::setPlayhead(ros::Time time)
{
    // if we perform a 'large' jump in time, lock the annotations
    // this ensure we do not unwillingly modify existing annotations
    // when seeking through the timeline
    if(  (   time > current_
          && time - current_ > ros::Duration(.5))
       ||(    time < current_
          && current_ - time > ros::Duration(.5)))
    {
        purpleAnnotations.lockAllCategories();
        yellowAnnotations.lockAllCategories();
    }

    // If we jump at the end of the annotation, unlock annotations, so that we can automatically continue to annotate
    if (time > purpleAnnotations.lastStopTime()) purpleAnnotations.unlockAllCategories();
    if (time > yellowAnnotations.lastStopTime()) yellowAnnotations.unlockAllCategories();

   purpleAnnotations.updateActive(time);
   yellowAnnotations.updateActive(time);

   current_ = time;
   update();
}

void Timeline::newAnnotation(StreamType stream, AnnotationType annotationtype)
{

    Annotation a({annotationtype, current_, current_});

    switch (stream) {
    case StreamType::PURPLE:
        purpleAnnotations.add(a);
        break;
    case StreamType::YELLOW:
        yellowAnnotations.add(a);
        break;
    default:
        break;
    }
}

void Timeline::clearAllAnnotations()
{
    emit pause();
    auto ret = QMessageBox::warning(this, "Clear all annotations",
                                   "Are you sure you want to clear all annotations?",
                                   QMessageBox::Yes | QMessageBox::No);

    if(ret == QMessageBox::Yes) {
        resetAnnotations();
    }
    emit timeJump(begin_);
}

void Timeline::resetAnnotations() {
        purpleAnnotations.clear();
        yellowAnnotations.clear();


        purpleAnnotations.add({AnnotationType::NOPLAY, begin_, begin_});
        yellowAnnotations.add({AnnotationType::NOPLAY, begin_, begin_});

        purpleAnnotations.add({AnnotationType::SOLITARY, begin_, begin_});
        yellowAnnotations.add({AnnotationType::SOLITARY, begin_, begin_});

        purpleAnnotations.add({AnnotationType::PASSIVE, begin_, begin_});
        yellowAnnotations.add({AnnotationType::PASSIVE, begin_, begin_});
}

void Timeline::setSavePath(const string &path)
{
    annotationPath = path;
}

Annotations Timeline::yamlToAnnotations(const YAML::Node node)
{
    Annotations annotations;

    for (const auto& as : node) {
        for (const auto& a : as) {
            auto type = annotationFromName(a.first.as<string>());
            auto ts = a.second.as<vector<double>>();
            annotations.add({type, ros::Time(ts[0]), ros::Time(ts[1])});
        }
    }
    return annotations;
}

void Timeline::loadFromFile(const string& path)
{
    qDebug() << "Loading " << QString::fromStdString(path);

    emit togglePause();

    YAML::Node node = YAML::LoadFile(path);


    purpleAnnotations = yamlToAnnotations(node["purple"]);
    yellowAnnotations = yamlToAnnotations(node["yellow"]);

    purpleAnnotations.lockAllCategories();
    yellowAnnotations.lockAllCategories();

    mergeMode = false;
    update();
}

void Timeline::mergeAnnotations(const string &path1, const string &path2)
{
    qDebug() << "Loading " << QString::fromStdString(path1) << " and " << QString::fromStdString(path2) << " for merging";

    emit togglePause();

    YAML::Node node1 = YAML::LoadFile(path1);
    purpleAnnotations = yamlToAnnotations(node1["purple"]);
    yellowAnnotations = yamlToAnnotations(node1["yellow"]);
    YAML::Node node2 = YAML::LoadFile(path2);
    purpleAnnotations2 = yamlToAnnotations(node2["purple"]);
    yellowAnnotations2 = yamlToAnnotations(node2["yellow"]);

    purpleDiff = diff(purpleAnnotations, purpleAnnotations2);
    yellowDiff = diff(yellowAnnotations, yellowAnnotations2);

    mergeMode = true;
    update();
}

void Timeline::saveToFile(const string& path)
{

    if(path.empty() && annotationPath.empty()) return;

    auto actualpath = path;
    if(actualpath.empty()) actualpath = annotationPath;

    YAML::Emitter out;

    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    stringstream ss;
    ss << "Annotations made by " << qgetenv("USER").toStdString() << " on the " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    out << YAML::Comment(ss.str());
    out << YAML::BeginMap;
    out << YAML::Key << "purple" << YAML::Value << purpleAnnotations;
    out << YAML::Key << "yellow" << YAML::Value << yellowAnnotations;
    out << YAML::EndMap;

    ofstream fout(actualpath);
    fout << out.c_str();
    //qDebug() << "Saved to " << QString::fromStdString(actualpath);
}

void Timeline::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto& rect = event->rect();

    auto left = rect.left();
    auto right = rect.right();
    auto top = rect.top();
    auto bottom = rect.bottom() - 20;


    auto bagLength = (end_ - begin_).toSec();

    pxPerSec_ = (right - left) * 1.0/ bagLength * timescale_;
    visibleDuration_ = (right - left) / pxPerSec_;
    elapsedTime_ = (current_ - begin_).toSec();
    startTime_ = std::min(std::max(0., elapsedTime_ - visibleDuration_  * 2./3), ((end_-begin_).toSec() - visibleDuration_));

    drawTimeline(&painter, left, right, top, bottom);

    //placeFreeAnnotations();

}

void Timeline::placeFreeAnnotations() {

    // placement of free annotations
    for(auto freeannotation : freeAnnotations) {
        auto atime = (freeannotation->time - begin_).toSec();

        if ( atime > startTime_ && atime < startTime_ + visibleDuration_) {
            freeannotation->move(atime * pxPerSec_, 0);
            if(!freeannotation->isVisible()) freeannotation->show();
        }
        else {
            if(freeannotation->isVisible()) freeannotation->hide();
        }
    }


}

void Timeline::drawTimeline(QPainter *painter, int left, int right, int top, int bottom) {

    int yellowAnnotationOffset_ = top + 10;
    int purpleAnnotationOffset_ = top + 60;

    int major_increment = 60; int minor_increment = 30;
    if (pxPerSec_ > 2) {major_increment = 30; minor_increment = 10;}
    if (pxPerSec_ > 3) {major_increment = 20; minor_increment = 5;}
    if (pxPerSec_ > 7) {major_increment = 10; minor_increment = 2;}
    if (pxPerSec_ > 30) {major_increment = 5; minor_increment = 1;}

    // compute lines to draw

    std::vector<QLine> lines_light;
    std::vector<QLine> lines_dark;

    painter->setPen(QPen(_color_light));

    double offset = major_increment - fmod(startTime_, major_increment);

    for (double t = startTime_ - (major_increment - offset); t <= visibleDuration_ + startTime_; t += major_increment) {

        auto x = (t - startTime_) * pxPerSec_;

        if(x >= left) {
            lines_light.push_back(QLine(x, top, x, bottom + 20));
            painter->drawText(QPoint(x + 2, bottom + 20), QString("%1:%2").arg(static_cast<int>(round(t)) / 60,2,10,QChar('0')).arg(static_cast<int>(round(t)) % 60,2,10,QChar('0')));
        }

        for (auto tm = t + minor_increment; tm < t + major_increment; tm += minor_increment) {
            auto x = (tm - startTime_) * pxPerSec_;
            if(x >= left) lines_dark.push_back(QLine(x, top, x, bottom));
        }

    }


    painter->fillRect(QRectF(left,top,right,bottom), _color_background);

    // annotation zones
    painter->fillRect(QRectF(left,purpleAnnotationOffset_ - 5,right,45), QColor("#4c2d64"));
    painter->fillRect(QRectF(left,yellowAnnotationOffset_ - 5,right,45), QColor("#64592d"));

    // draw time
    painter->setPen(QPen(_color_light));
    painter->drawLines(lines_light.data(), lines_light.size());
    painter->setPen(QPen(_color_light.darker()));
    painter->drawLines(lines_dark.data(), lines_dark.size());



    // Drawing of annotations
    QFont font = painter->font() ;
    font.setPointSize(8);
    painter->setFont(font);

    if (!mergeMode) {
        for(const auto a : purpleAnnotations) drawAnnotation(painter, a, purpleAnnotationOffset_, left);
        for(const auto a : yellowAnnotations) drawAnnotation(painter, a, yellowAnnotationOffset_, left);
    }
    else {
        for(const auto a : purpleAnnotations) drawAnnotation(painter, a, purpleAnnotationOffset_, left);
        for(const auto a : purpleAnnotations2) drawAnnotation(painter, a, purpleAnnotationOffset_ + 5, left);
        for(const auto a : purpleDiff) drawAnnotation(painter, a, purpleAnnotationOffset_ + 10, left);

    }
    
    // playhead
    painter->setPen(QPen(_color_playhead, 2));
    painter->drawLine(QLine(left + (elapsedTime_ - startTime_) * pxPerSec_, top, left + (elapsedTime_ - startTime_) * pxPerSec_, bottom));


}

void Timeline::drawAnnotation(QPainter *painter,
                              AnnotationConstPtr a,
                              int offset,
                              int left) {

        if(a->type == AnnotationType::MISSING) return;

        if(mergeMode && a->category() != AnnotationCategory::TASK_ENGAGEMENT) return;

        int radius = 4;
        QFontMetrics fm(painter->font());

        auto categoryOffset = 5;
        if(a->category() == AnnotationCategory::SOCIAL_ENGAGEMENT) categoryOffset = 20;
        else if(a->category() == AnnotationCategory::SOCIAL_ATTITUDE) categoryOffset = 35;

        auto y = offset + categoryOffset;

        auto start = std::max(0., (a->start - begin_).toSec() - startTime_);
        auto stop = std::min((a->stop - begin_).toSec() - startTime_, startTime_ + visibleDuration_);
        auto x1 = left + start * pxPerSec_;
        auto x2 = left + stop * pxPerSec_;

        painter->setPen(Annotation::Styles[a->type]);
        painter->drawLine(x1, y, x2-2, y);

        QPen capsPen(painter->pen());
        capsPen.setStyle(Qt::SolidLine);
        painter->setPen(capsPen);
        painter->drawLine(x1, y - radius/2, x1, y + radius/2);
        painter->drawLine(x2-2, y - radius/2, x2-2, y + radius/2);

        int annotationNameWidth = fm.width(QString::fromStdString(a->name()));
        if ((x2-x1) > annotationNameWidth + 5) {
            painter->drawText(QPoint(x1 + 2, y - 2), QString::fromStdString(a->name()));
        }

}


void Timeline::keyPressEvent(QKeyEvent *event) {

    switch (event->key()) {

    case Qt::Key_Space:
        emit togglePause();
        break;

    case Qt::Key_Up:
        timescale_ *= 1.1;
        if (timescale_ > 50.) timescale_ = 50.;
        break;
    case Qt::Key_Down:
        timescale_ *= 0.9;
        if (timescale_ < 1.) timescale_ = 1.;
        break;

    case Qt::Key_Q:
        purpleAnnotations.add({AnnotationType::PROSOCIAL, current_, current_});
        break;
    case Qt::Key_W:
        purpleAnnotations.add({AnnotationType::ADVERSARIAL, current_, current_});
        break;
    case Qt::Key_E:
        purpleAnnotations.add({AnnotationType::ASSERTIVE, current_, current_});
        break;
    case Qt::Key_A:
        purpleAnnotations.add({AnnotationType::PASSIVE, current_, current_});
        break;
    case Qt::Key_S:
        if(QApplication::keyboardModifiers() && Qt::ControlModifier) // ctrl+s
        {
            emit togglePause();
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save annotations to..."),
                                    "",
                                    tr("Annotations (*.yaml)"));
            emit togglePause();

            if(!fileName.isEmpty()) {
                annotationPath = fileName.toStdString();
                saveToFile(annotationPath);
            }
        }
        else {
            purpleAnnotations.add({AnnotationType::ADULTSEEKING, current_, current_});
        }
        break;
     case Qt::Key_D:
        purpleAnnotations.add({AnnotationType::AIMLESS, current_, current_});
        break;

    case Qt::Key_U:
        yellowAnnotations.add({AnnotationType::PROSOCIAL, current_, current_});
        break;
    case Qt::Key_I:
        yellowAnnotations.add({AnnotationType::ADVERSARIAL, current_, current_});
        break;
    case Qt::Key_O:
        if(QApplication::keyboardModifiers() && Qt::ControlModifier) // ctrl+o
        {
            emit pause();
            QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Load annotations (select 2 for merge mode)"),
                                                            "",
                                                            tr("Annotations (*.yaml)"));
            if(fileNames.size() == 1) {
                annotationPath = fileNames[0].toStdString();
                loadFromFile(annotationPath);
            }
            else if(fileNames.size() == 2) {
                mergeAnnotations(fileNames[0].toStdString(), fileNames[1].toStdString());

            }
        }
        else {
            yellowAnnotations.add({AnnotationType::ASSERTIVE, current_, current_});
        }
        break;
    case Qt::Key_J:
        yellowAnnotations.add({AnnotationType::PASSIVE, current_, current_});
        break;
    case Qt::Key_K:
        yellowAnnotations.add({AnnotationType::ADULTSEEKING, current_, current_});
        break;
     case Qt::Key_L:
        yellowAnnotations.add({AnnotationType::AIMLESS, current_, current_});
        break;
     case Qt::Key_X:
     case Qt::Key_Delete:
        clearAllAnnotations();
        break;

        ////// NOT HANDLED -> pass forward
    default:
        QWidget::keyPressEvent(event);
    }
}

void Timeline::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
            auto lastPoint = event->pos();
            emit timeJump(pointToTimestamp(lastPoint));
    }
}


void Timeline::wheelEvent(QWheelEvent *event) {
        //setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        double scaleFactor = 1.25;
        if (event->delta() > 0) {
            // zoom in
            timescale_ *= scaleFactor;
            if (timescale_ > 50.) timescale_ = 50.;
        } else {
            // zoom out
            timescale_ /= scaleFactor;
            if (timescale_ < 1.) timescale_ = 1.;
        }
        event->accept();

        update();
}

ros::Time Timeline::pointToTimestamp(QPoint point) {

    return begin_ + ros::Duration(startTime_ + (point.x())/pxPerSec_);
}
