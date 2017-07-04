#include "annotation.h"

using namespace std;

std::map<AnnotationType, QPen> Annotation::Styles = {
            {AnnotationType::HOSTILE, QPen(QBrush(QColor("#ad1d1d")), 2, Qt::SolidLine)},
            {AnnotationType::PROSOCIAL, QPen(QBrush(QColor("#1dad75")), 2, Qt::SolidLine)},
            {AnnotationType::ASSERTIVE, QPen(QBrush(QColor("#6697bd")), 2, Qt::SolidLine)},
            {AnnotationType::PASSIVE, QPen(QBrush(QColor("#bd9966")), 2, Qt::DashLine)},
            {AnnotationType::ADULTSEEKING, QPen(QBrush(QColor("#f2ce3e")), 2, Qt::DashLine)},
            {AnnotationType::IRRELEVANT, QPen(QBrush(QColor("#f2ce3e")), 2, Qt::DotLine)}
        };


void Annotations::updateCurrentAnnotationEnd(ros::Time time)
{
    AnnotationPtr current, next;

    std::tie(current, next) = getAnnotationAtAndNext(time);

    if(current) current->stop = time;
    if(next && next->start < time) next->start = time;
}

std::tuple<AnnotationPtr, AnnotationPtr> Annotations::getAnnotationAtAndNext(ros::Time time)
{
   if (annotations.empty()) return std::make_tuple(nullptr, nullptr);

   for(size_t i = 0; i < annotations.size(); i++) {

       if (i == annotations.size() - 1) {
           return std::make_tuple(annotations[i], nullptr);
       }
       if (annotations[i+1]->start > time) {
           return std::make_tuple(annotations[i], annotations[i+1]);
       }
   }
}
