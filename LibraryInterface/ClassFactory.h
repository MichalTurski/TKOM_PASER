//
// Created by michal on 03.12.18.
//

#ifndef PROJEKT_CLASSFACTORY_H
#define PROJEKT_CLASSFACTORY_H

#include <string>
#include "Object.h"

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

#endif //PROJEKT_CLASSFACTORY_H
