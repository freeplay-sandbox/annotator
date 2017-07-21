
#include <QDebug>

#include "annotation.hpp"

using namespace std;

const ros::Duration MAX_TIME_TO_MERGE(0.5);

std::map<AnnotationType, QPen> Annotation::Styles = {
            {AnnotationType::GOALORIENTED, QPen(QBrush(QColor("#4CAF50")), 2, Qt::SolidLine)},
            {AnnotationType::AIMLESS, QPen(QBrush(QColor("#ff6f00")), 2, Qt::SolidLine)},
            {AnnotationType::ADULTSEEKING, QPen(QBrush(QColor("#E57373")), 2, Qt::DashLine)},
            {AnnotationType::NOPLAY, QPen(QBrush(QColor("#E3F2FD")), 2, Qt::DotLine)},
            {AnnotationType::SOLITARY, QPen(QBrush(QColor("#9fa8da")), 2, Qt::SolidLine)},
            {AnnotationType::ONLOOKER, QPen(QBrush(QColor("#00bcd4")), 2, Qt::SolidLine)},
            {AnnotationType::PARALLEL, QPen(QBrush(QColor("#e6ee9c")), 2, Qt::SolidLine)},
            {AnnotationType::ASSOCIATIVE, QPen(QBrush(QColor("#ffeb3b")), 2, Qt::SolidLine)},
            {AnnotationType::COOPERATIVE, QPen(QBrush(QColor("#ffc107")), 2, Qt::SolidLine)},
            {AnnotationType::PROSOCIAL, QPen(QBrush(QColor("#4CAF50")), 2, Qt::SolidLine)},
            {AnnotationType::ADVERSARIAL, QPen(QBrush(QColor("#ff6f00")), 2, Qt::SolidLine)},
            {AnnotationType::ASSERTIVE, QPen(QBrush(QColor("#26a69a")), 2, Qt::SolidLine)},
            {AnnotationType::FRUSTRATED, QPen(QBrush(QColor("#9c27b0")), 2, Qt::SolidLine)},
            {AnnotationType::PASSIVE, QPen(QBrush(QColor("#E3F2FD")), 2, Qt::DotLine)}
        };

AnnotationType annotationFromName(const std::string& name) {
    for (const auto& kv : AnnotationNames) {
        if (kv.second.first == name) return kv.first;
    }
    throw std::range_error("unknown annotation type " + name);
}


void Annotations::updateActive(ros::Time time)
{

    // clean up annotations: all annotations with a null (or negative) duration are erased
    annotations.erase(std::remove_if(annotations.begin(),
                                     annotations.end(),
                                     [](AnnotationPtr a){return a->start >= a->stop;}),
                      annotations.end());

    for (auto category : AnnotationCategories) {

        auto active = getClosestStopTime(time, category);
        if(active && active->stop < time) active->stop = time;

        auto next = getNextInCategory(active);
        if(next && next->start < time) next->start = time;
    }
}

/**
 * @brief Returns the annotation whose stop time is the closest to the current time (within the MAX_TIME_TO_MERGE threshold)
 * @param time
 * @param category: the returned annotation must belong to this category
 * @return
 */
AnnotationPtr Annotations::getClosestStopTime(ros::Time time, AnnotationCategory category) {

    AnnotationPtr closest = nullptr;

    for (auto a : annotations) {
        if(   a->category() == category
           && a->stop < time
           && a->stop > time - MAX_TIME_TO_MERGE) {
            if (   !closest
                || (time - a->stop) < (time - closest->stop))
            {
                closest = a;
            }
        }
    }
    return closest;
}

/**
 * @brief returns the annotation following 'ref' in the timeline, *belonging to the same category* (or nullptr is none exist)
 */
AnnotationPtr Annotations::getNextInCategory(AnnotationPtr ref) {

    bool foundmyself = false;

    // note that 'annotations' is always sorted by start time!
    for (auto a : annotations) {
        if(foundmyself && a->category() == ref->category()) return a;
        if(a == ref) foundmyself = true;
    }
    return nullptr;
}


void Annotations::add(Annotation annotation) {

    // interrupt current annotation, if any
    auto actives = getAnnotationsAt(annotation.start);
    for (auto a : actives) {
        if(a->category() == annotation.category()) {
            a->stop = annotation.start;
        }
    }

    annotation.stop += ros::Duration(0.001); // make sure our annotation has a non-null duration

    annotations.push_back(std::make_shared<Annotation>(annotation));

    std::sort(annotations.begin(), annotations.end(), [](const AnnotationPtr a, const AnnotationPtr b) { return a->start < b->start; });
}

/**
 * @brief Returns the list of annotations at given time
 * @param time
 * @return
 */
vector<AnnotationPtr> Annotations::getAnnotationsAt(ros::Time time)
{
   vector<AnnotationPtr> res;

   if (annotations.empty()) return res;

   for(size_t i = 0; i < annotations.size(); i++) {

       if (   time <= annotations[i]->stop
           && time >= annotations[i]->start)
       {
           res.push_back(annotations[i]);
       }
   }

   return res;
}



vector<AnnotationPtr> Annotations::getAnnotationsAtApprox(ros::Time time)
{

   vector<AnnotationPtr> res = getAnnotationsAt(time);

   for(size_t i = 0; i < annotations.size(); i++) {

       if (   time < (annotations[i]->stop + MAX_TIME_TO_MERGE)
           && time > (annotations[i]->start - MAX_TIME_TO_MERGE))
       {
           res.push_back(annotations[i]);
       }
   }

   return res;
}

YAML::Emitter& operator<< (YAML::Emitter& out, const Annotations& a)
{
    out << YAML::BeginSeq;
    for (const auto& annotation : a.annotations) {
        out << YAML::BeginMap;
        out << YAML::Key << AnnotationNames.at(annotation->type).first;
        out << YAML::Value << vector<double>{annotation->start.toSec(), annotation->stop.toSec()};
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
    return out;
}
