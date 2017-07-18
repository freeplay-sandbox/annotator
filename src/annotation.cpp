
#include <QDebug>

#include "annotation.hpp"

using namespace std;

const ros::Duration MAX_TIME_TO_MERGE(0.5);

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
    AnnotationPtr current, next = nullptr;

    current = getAnnotationAtApprox(time);

    if(!current) return;

    if (current != annotations.back()) // we are not the last annotation
    {
        auto it = std::find(annotations.begin(), annotations.end(), current);
        next = *(it + 1);
        qDebug() << "Got next one";
    }

    if(current && current->stop < time) current->stop = time;
    if(next && next->start < time) next->start = time;
}


void Annotations::add(Annotation annotation) {
    annotations.push_back(std::make_shared<Annotation>(annotation));

    std::sort(annotations.begin(), annotations.end(), [](const AnnotationPtr a, const AnnotationPtr b) { return a->start < b->start; });
}

/**
 * @brief Returns a pointer to the annotation at given time, or nullptr if none
 * @param time
 * @return
 */
AnnotationPtr Annotations::getAnnotationAt(ros::Time time)
{
   if (annotations.empty()) return nullptr;

   for(size_t i = 0; i < annotations.size(); i++) {

       if (   i == annotations.size() - 1
           && time < annotations[i]->stop
           && time > annotations[i]->start)
       {
           return annotations[i];
       }
   }

   return nullptr;
}

AnnotationPtr Annotations::getAnnotationAtApprox(ros::Time time)
{

    auto annotation = getAnnotationAt(time);

    if(annotation) return annotation;

    //else...
   for(size_t i = 0; i < annotations.size(); i++) {

       if (   i == annotations.size() - 1
           && time < (annotations[i]->stop + MAX_TIME_TO_MERGE)
           && time > (annotations[i]->start - MAX_TIME_TO_MERGE))
       {
           return annotations[i];
       }
   }

   return nullptr;
}

YAML::Emitter& operator<< (YAML::Emitter& out, const Annotations& a)
{
    out << YAML::BeginSeq;
    for (const auto& annotation : a.annotations) {
        out << YAML::BeginMap;
        out << YAML::Key << AnnotationNames.at(annotation->type);
        out << YAML::Value << vector<double>{annotation->start.toSec(), annotation->stop.toSec()};
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
    return out;
}
