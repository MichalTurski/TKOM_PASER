//
// Created by michal on 06.12.18.
//

#include "Collection.h"
#include "../LibraryInterface/Object.h"

Collection::Collection(): Object("Collection") {
    currObjIdx = 0;
}
Object** Collection::iterate() {
    Object **currObj;
    if (currObjIdx < objects.size()) {
        return  objects[currObjIdx++];
    } else {
        currObjIdx = 0;
        return nullptr;
    }
}
Object* Collection::clone(const std::string &str) const {
    return new Collection();
}
Object* Collection::evaluateMethod(const std::string &name, Objects &arguments) {
    if (name == "add") {
        addElements(arguments);
        return nullptr;
    } else {
        throw std::runtime_error("There is no method called " + name + " in collection.");
    }
}
void Collection::addElements(Objects &arguments) {
    std::string elementsType;
    Object *object;
    std::vector<Object **> candidates;

    candidates.reserve(arguments.size());
    object = *arguments.front();
    elementsType = object->getType();
    if ((!collectionType.empty()) && collectionType != elementsType) {
        throw std::runtime_error("Wrong element type, expected " + collectionType +
                                 " got " + elementsType);
    }
    for (auto &&i :arguments) {
        if (elementsType == (*i)->getType()) {
            candidates.emplace_back(i);
        } else {
            throw std::runtime_error("Wrong element type, expected " + elementsType +
                                     " got " + (*i)->getType());
        }
    }
    if (collectionType.empty()) {
        collectionType = std::move(elementsType);
    }
    objects.insert(objects.end(), candidates.begin(), candidates.end());
}
