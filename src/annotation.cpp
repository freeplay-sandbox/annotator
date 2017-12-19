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
            {AnnotationType::PASSIVE, QPen(QBrush(QColor("#E3F2FD")), 2, Qt::DotLine)},

            {AnnotationType::OTHER, QPen(QBrush(QColor("#555599")), 1, Qt::SolidLine)}
        };

QPen Annotation::CONFLICT_PEN = QPen(QBrush(QColor("#FF0000")), 3, Qt::SolidLine);

AnnotationType annotationFromName(const std::string& name) {
    for (const auto& kv : AnnotationNames) {
        if (kv.second.first == name) return kv.first;
    }
    throw std::range_error("unknown annotation type " + name);
}


Annotations::Annotations() :
    lockedCategories({{AnnotationCategory::TASK_ENGAGEMENT, true},
                       {AnnotationCategory::SOCIAL_ENGAGEMENT, true},
                       {AnnotationCategory::SOCIAL_ATTITUDE, true}})
{

}

void Annotations::updateActive(ros::Time time)
{

    // clean up annotations: all annotations with a null (or negative) duration are erased
    annotations.erase(std::remove_if(annotations.begin(),
                                     annotations.end(),
                                     [](AnnotationPtr a){return a->start >= a->stop;}),
                      annotations.end());

    for (auto category : AnnotationCategories) {
        if(isLocked(category)) continue;

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

    unlock(annotation.category());

    annotation.stop += ros::Duration(0.001); // make sure our annotation has a non-null duration

    // interrupt current annotation, if any
    auto actives = getAnnotationsAt(annotation.start);
    for (auto a : actives) {
        if(a->category() == annotation.category()) {
            if(a->stop > annotation.stop) { // need to split!
                Annotation a2(*a);
                a2.start = annotation.stop;
                annotations.push_back(std::make_shared<Annotation>(a2));
            }
            a->stop = annotation.start;
        }
    }


    annotations.push_back(std::make_shared<Annotation>(annotation));

    std::sort(annotations.begin(), annotations.end(), [](const AnnotationPtr a, const AnnotationPtr b) { return a->start < b->start; });
}

void Annotations::lockAllCategories()
{
   for (auto c : AnnotationCategories) lockedCategories[c] = true;
}

void Annotations::unlockAllCategories()
{
    for (auto c : AnnotationCategories) lockedCategories[c] = false;
}

/**
 * @brief Returns the end time of the last annotation
 * @return
 */
ros::Time Annotations::lastStopTime() const
{
    auto t = ros::TIME_MIN;

    for (auto a : annotations) {
        if (a->stop > t) t = a->stop;
    }
    return t;
}

/**
 * @brief Returns the list of annotations at given time. Can be more than one when the stop time and start time of 2 annotations match
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

/**
 * Returns the type of the annotation at given time.
 * If no annotation exist at given time, returns MISSING.
 * If two annotations exist at given time (in case of identical start and stop time), return the type of the
 * annotation about to start.
 */
AnnotationType Annotations::getAnnotationTypeAt(ros::Time time) const
{
   if (annotations.empty()) return AnnotationType::MISSING;

   for(size_t i = 0; i < annotations.size(); i++) {

       if (   time < annotations[i]->stop
           && time >= annotations[i]->start)
       {
           return annotations[i]->type;
       }
   }

   return AnnotationType::MISSING;

}

Annotations Annotations::filterByCategory(AnnotationCategory category) const
{
   Annotations filtered;

   for (auto a : annotations) {
       if (AnnotationNames.at(a->type).second == category)
           filtered.add(*a);
   }

   return filtered;
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


Annotations diff(const Annotations &annotations1, const Annotations &annotations2)
{

    auto a1 = annotations1.filterByCategory(AnnotationCategory::TASK_ENGAGEMENT);
    auto a2 = annotations2.filterByCategory(AnnotationCategory::TASK_ENGAGEMENT);


    // first, calculate all the 'time splits', ie the time intervals resulting from the
    // intersection of the 2 annotations streams.
    vector<ros::Time> next_time_candidates;
    size_t idx1 = 0;
    size_t idx2 = 0;
    auto current_time = min(a1[idx1]->start, a2[idx2]->start);
    vector<ros::Time> time_splits {current_time};

    while(true) {

        if(idx1 < a1.size()) {
            if(current_time < a1[idx1]->start)
                next_time_candidates.push_back(a1[idx1]->start);
            if(current_time < a1[idx1]->stop)
                next_time_candidates.push_back(a1[idx1]->stop);
        }

        if(idx2 < a2.size()) {
            if(current_time < a2[idx2]->start)
                next_time_candidates.push_back(a2[idx2]->start);
            if(current_time < a2[idx2]->stop)
                next_time_candidates.push_back(a2[idx2]->stop);
        }

        ros::Time next_time_split = next_time_candidates[0];
        for (auto t : next_time_candidates) {
            next_time_split = min(next_time_split, t);
        }

        next_time_candidates.clear();

        time_splits.push_back(next_time_split);
        current_time = next_time_split;

        if(idx1 < a1.size() && next_time_split == a1[idx1]->stop) idx1++;
        if(idx2 < a2.size() && next_time_split == a2[idx2]->stop) idx2++;

        if(idx1 >= a1.size() && idx2 >= a2.size()) break;
    }

    // second, for each time splits, check whether the two original annotation streams
    // match. If they do not, set the resulting interval as 'CONFLICT'
    Annotations diffs;
    for (size_t t=0; t < time_splits.size() - 1; t++) {
        auto type1 = a1.getAnnotationTypeAt(time_splits[t]);
        auto type2 = a2.getAnnotationTypeAt(time_splits[t]);

        if(type1 == type2) {
            if(type1 == AnnotationType::MISSING) {continue;}
            diffs.add({type1, time_splits[t], time_splits[t+1]});
        }
        else {
            auto category = (type1 == AnnotationType::MISSING) ? AnnotationNames.at(type2).second : AnnotationNames.at(type1).second;
            switch (category) {
            case AnnotationCategory::TASK_ENGAGEMENT:
                diffs.add({AnnotationType::OTHER_TASK_ENGAGEMENT, time_splits[t], time_splits[t+1], true}); // conflicted!
                break;
            case AnnotationCategory::SOCIAL_ENGAGEMENT:
                diffs.add({AnnotationType::OTHER_SOCIAL_ENGAGEMENT, time_splits[t], time_splits[t+1], true}); // conflicted!
                break;
            case AnnotationCategory::SOCIAL_ATTITUDE:
                diffs.add({AnnotationType::OTHER_SOCIAL_ATTITUDE, time_splits[t], time_splits[t+1], true}); // conflicted!
                break;
            default:
                assert(false);
                break;
            }
        }
    }

    return diffs;

}
