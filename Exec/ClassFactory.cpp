//
// Created by michal on 03.12.18.
//

#include "ClassFactory.h"

Class::Class(const std::string &name, Object *prototype):
        name(name),
        prototype(prototype) {}
Class::Class(Class &&other):
        name(std::move(other.name)),
        prototype(other.prototype) {
    other.prototype = nullptr;
}
Class::~Class() {
    delete prototype;
}

void ClassFactory::addClass(const std::string &name, Object *prototype) {
    Class newClass = Class(name, prototype);
    classes.emplace(name, std::move(newClass));
}
Object* ClassFactory::getObject(const std::string &name, const std::string &reference) {
    auto &&classesIter = classes.find(name);
    if (classesIter == classes.end()) {
        return nullptr;
    }
    return classesIter->second.prototype->clone(reference);
}

