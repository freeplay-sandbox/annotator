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

                           SOLITARY,
                           ONLOOKER,
                           PARALLEL,
                           ASSOCIATIVE,
                           COOPERATIVE,

                           PROSOCIAL,
                           ADVERSARIAL,
                           ASSERTIVE,
                           FRUSTRATED,
                           PASSIVE};

const std::map<AnnotationType, std::pair<std::string, AnnotationCategory>> AnnotationNames {
                {AnnotationType::GOALORIENTED, {"goaloriented", AnnotationCategory::TASK_ENGAGEMENT}},
                {AnnotationType::AIMLESS, {"aimless", AnnotationCategory::TASK_ENGAGEMENT}},
                {AnnotationType::ADULTSEEKING, {"adultseeking", AnnotationCategory::TASK_ENGAGEMENT}},
                {AnnotationType::NOPLAY, {"noplay", AnnotationCategory::TASK_ENGAGEMENT}},

                {AnnotationType::SOLITARY, {"solitary", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::ONLOOKER, {"onlooker", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::PARALLEL, {"parallel", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::ASSOCIATIVE, {"associative", AnnotationCategory::SOCIAL_ENGAGEMENT}},
                {AnnotationType::COOPERATIVE, {"cooperative", AnnotationCategory::SOCIAL_ENGAGEMENT}},

                {AnnotationType::PROSOCIAL, {"prosocial", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::ADVERSARIAL, {"adversarial", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::ASSERTIVE, {"assertive", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::FRUSTRATED, {"frustrated", AnnotationCategory::SOCIAL_ATTITUDE}},
                {AnnotationType::PASSIVE, {"passive", AnnotationCategory::SOCIAL_ATTITUDE}}
};

AnnotationType annotationFromName(const std::string& name);

struct Annotation
{
    static std::map<AnnotationType, QPen> Styles;

    AnnotationType type;
    ros::Time start;
    ros::Time stop;

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

private:

    std::map<AnnotationCategory, bool> lockedCategories;

    std::vector<AnnotationPtr> getAnnotationsAt(ros::Time time);
    std::vector<AnnotationPtr> getAnnotationsAtApprox(ros::Time time);

    std::vector<AnnotationPtr> annotations;

    AnnotationPtr getNextInCategory(AnnotationPtr ref);
    AnnotationPtr getClosestStopTime(ros::Time time, AnnotationCategory category);
};


#endif // ANNOTATION_H
