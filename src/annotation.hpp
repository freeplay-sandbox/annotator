#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <map>
#include <vector>
#include <array>
#include <tuple>
#include <memory>

#include <QPen>
#include <QBrush>
#include <ros/time.h>

#include <yaml-cpp/yaml.h>

enum class StreamType {PURPLE, YELLOW, GLOBAL};

enum class AnnotationCategory {
                    OTHER=0,
                    TASK_ENGAGEMENT,
                    SOCIAL_ENGAGEMENT,
                    SOCIAL_ATTITUDE
            };

const std::array<AnnotationCategory,3> AnnotationCategories {AnnotationCategory::TASK_ENGAGEMENT,
                                                            AnnotationCategory::SOCIAL_ENGAGEMENT,
                                                            AnnotationCategory::SOCIAL_ATTITUDE};

enum class AnnotationType {OTHER=0,

                           GOALORIENTED,
                           AIMLESS,
                           ADULTSEEKING,
                           NOPLAY,
                           OTHER_TASK_ENGAGEMENT,

                           SOLITARY,
                           ONLOOKER,
                           PARALLEL,
                           ASSOCIATIVE,
                           COOPERATIVE,
                           OTHER_SOCIAL_ENGAGEMENT,

                           PROSOCIAL,
                           ADVERSARIAL,
                           ASSERTIVE,
                           FRUSTRATED,
                           PASSIVE,
                           OTHER_SOCIAL_ATTITUDE,

                           MISSING};

const std::map<AnnotationType, std::pair<std::string, AnnotationCategory>> AnnotationNames {
                {AnnotationType::OTHER, {"?", AnnotationCategory::OTHER}},

                {AnnotationType::GOALORIENTED, {"goaloriented", AnnotationCategory::TASK_ENGAGEMENT}},
                {AnnotationType::AIMLESS, {"aimless", AnnotationCategory::TASK_ENGAGEMENT}},
                {AnnotationType::ADULTSEEKING, {"adultseeking", AnnotationCategory::TASK_ENGAGEMENT}},
                {AnnotationType::NOPLAY, {"noplay", AnnotationCategory::TASK_ENGAGEMENT}},
                {AnnotationType::OTHER_TASK_ENGAGEMENT, {"?", AnnotationCategory::TASK_ENGAGEMENT}},

                {AnnotationType::SOLITARY, {"solitary", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::ONLOOKER, {"onlooker", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::PARALLEL, {"parallel", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::ASSOCIATIVE, {"associative", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::COOPERATIVE, {"cooperative", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::OTHER_SOCIAL_ENGAGEMENT, {"?", AnnotationCategory::SOCIAL_ENGAGEMENT}},

                {AnnotationType::PROSOCIAL, {"prosocial", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::ADVERSARIAL, {"adversarial", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::ASSERTIVE, {"assertive", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::FRUSTRATED, {"frustrated", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::PASSIVE, {"passive", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::OTHER_SOCIAL_ATTITUDE, {"?", AnnotationCategory::SOCIAL_ATTITUDE}},
};

AnnotationType annotationFromName(const std::string& name);

struct Annotation
{
    static std::map<AnnotationType, QPen> Styles;
    static QPen CONFLICT_PEN;

    AnnotationType type;
    ros::Time start;
    ros::Time stop;

    bool isConflicted;

    Annotation(AnnotationType type, ros::Time start, ros::Time stop, bool isConflicted=false) :
        type(type), start(start), stop(stop), isConflicted(isConflicted) {}

    std::string name() const {return AnnotationNames.at(type).first;}
    AnnotationCategory category() const {return AnnotationNames.at(type).second;}
};

typedef typename std::shared_ptr<Annotation> AnnotationPtr;
typedef typename std::shared_ptr<const Annotation> AnnotationConstPtr;

class Annotations
{

public:

    Annotations();

    friend YAML::Emitter& operator<< (YAML::Emitter& out, const Annotations& a);

    typedef typename std::vector<AnnotationPtr>::iterator iterator;
    typedef typename std::vector<AnnotationPtr>::const_iterator const_iterator;

    iterator begin() {return annotations.begin();}
    const_iterator begin() const {return annotations.begin();}
    const_iterator cbegin() const {return annotations.cbegin();}
    iterator end() {return annotations.end();}
    const_iterator end() const {return annotations.end();}
    const_iterator cend() const {return annotations.cend();}

    AnnotationPtr& operator[](const int nIndex) {
        return annotations[nIndex];
    }
    const AnnotationPtr& operator[](const int nIndex) const {
        return annotations[nIndex];
    }
    size_t size() const {
        return annotations.size();
    }


    void updateActive(ros::Time time);
    void add(Annotation annotation);
    void clear() {annotations.clear();}

    bool isLocked(AnnotationCategory category) const {return lockedCategories.at(category);}
    void lock(AnnotationCategory category) {lockedCategories[category] = true;}
    void unlock(AnnotationCategory category) {lockedCategories[category] = false;}
    void lockAllCategories();
    void unlockAllCategories();

    ros::Time lastStopTime() const;

    AnnotationType getAnnotationTypeAt(ros::Time time) const;

    /** Returns a copy of the annotations, only keeping annotations belonging
     * to 'category'
     */
    Annotations filterByCategory(AnnotationCategory category) const;

private:

    std::map<AnnotationCategory, bool> lockedCategories;

    std::vector<AnnotationPtr> getAnnotationsAt(ros::Time time);
    std::vector<AnnotationPtr> getAnnotationsAtApprox(ros::Time time);

    std::vector<AnnotationPtr> annotations;

    AnnotationPtr getNextInCategory(AnnotationPtr ref);
    AnnotationPtr getClosestStopTime(ros::Time time, AnnotationCategory category);
};

/**
* Generates the 'diff' of 2 sets of annotations: a new annotations set representing only the differences between the 2
* provided annotations sets.
*/
Annotations diff(const Annotations& annotations1, const Annotations& annotations2);


#endif // ANNOTATION_H
