//
// Created by michal on 20.11.18.
//

#ifndef PROJEKT_OBJECT_H
#define PROJEKT_OBJECT_H

//#include "ExecutionState.h"


#include <string>
#include <boost/any.hpp>
#include <map>
#include <list>

class Object;
class ExternFunction;

using methodsMap = std::map<std::string, ExternFunction>;
using Objects = std::list<Object *>;


class Object {
protected:
    const std::string type;
    bool anonymous; /*If this field is true, free this object when unused.*/
public:
    explicit Object(const std::string &type);
    Object(const Object &other);
    virtual Object *clone() const = 0;
    virtual Object *clone(const std::string &reference) const = 0;
    virtual Object *evaluateMethod(const std::string &name, Objects &arguments) = 0;
    void makeNamed();
    void makeAnonymous();
    const std::string &getType();
    bool Anonymous();
};

class Class {
public:
    std::string name;
    Object *prototype;

    Class(const std::string &name, Object *prototype);
    Class(Class &&other);
    ~Class();
};

class ClassFactory {
private:
    std::map<std::string, Class> classes;
public:
    void addClass(const std::string &name, Object *prototype);
    Object *getObject(const std::string &name, const std::string &reference);
};

#endif //PROJEKT_OBJECT_H
