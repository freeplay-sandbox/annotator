#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <map>
#include <vector>
#include <tuple>
#include <memory>

#include <QPen>
#include <QBrush>
#include <ros/time.h>

#include <yaml-cpp/yaml.h>

enum class StreamType {PURPLE, YELLOW, GLOBAL};

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

const std::map<AnnotationType, std::string> AnnotationNames {
                {AnnotationType::OTHER,"other"},
                {AnnotationType::GOALORIENTED,"goaloriented"},
                {AnnotationType::AIMLESS,"aimless"},
                {AnnotationType::ADULTSEEKING,"adultseeking"},
                {AnnotationType::NOPLAY,"noplay"},

                {AnnotationType::SOLITARY,"solitary"},
                {AnnotationType::ONLOOKER,"onlooker"},
                {AnnotationType::PARALLEL,"parallel"},
                {AnnotationType::ASSOCIATIVE,"associative"},
                {AnnotationType::COOPERATIVE,"cooperative"},

                {AnnotationType::PROSOCIAL,"prosocial"},
                {AnnotationType::ADVERSARIAL,"adversarial"},
                {AnnotationType::ASSERTIVE,"assertive"},
                {AnnotationType::FRUSTRATED,"frustrated"},
                {AnnotationType::PASSIVE,"passive"}
    };


AnnotationType annotationFromName(const std::string& name);

struct Annotation
{
    static std::map<AnnotationType, QPen> Styles;

    AnnotationType type;
    ros::Time start;
    ros::Time stop;
};

typedef typename std::shared_ptr<Annotation> AnnotationPtr;

class Annotations
{

public:
    void updateCurrentAnnotationEnd(ros::Time time);

    friend YAML::Emitter& operator<< (YAML::Emitter& out, const Annotations& a);

    typedef typename std::vector<AnnotationPtr>::iterator iterator;
    typedef typename std::vector<AnnotationPtr>::const_iterator const_iterator;

    iterator begin() {return annotations.begin();}
    const_iterator begin() const {return annotations.begin();}
    const_iterator cbegin() const {return annotations.cbegin();}
    iterator end() {return annotations.end();}
    const_iterator end() const {return annotations.end();}
    const_iterator cend() const {return annotations.cend();}

    void add(Annotation annotation);
    void clear() {annotations.clear();}

private:

    AnnotationPtr getAnnotationAt(ros::Time time);
    AnnotationPtr getAnnotationAtApprox(ros::Time time);

    std::vector<AnnotationPtr> annotations;

};


#endif // ANNOTATION_H
