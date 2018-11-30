//
// Created by michal on 08.11.18.
//

#include <assert.h>
#include "ExecutionState.h"
#include "BuildIn.h"
#include "Symbols.h"
#include "GrammarNode.h"

ExecutionState::~ExecutionState() {
    for (auto&&i : localObjects) {
        free(i);
    }
}
Object* ExecutionState::getObject(const std::string &name) {
    auto&& objectIter = objects.find(name);
    if (objectIter == objects.end()) {
        return nullptr;
    } else {
        return objectIter->second;
    }
}
void ExecutionState::addObject(const std::string &name, Object *object) {
    localObjects.emplace_back(object);
    objects.emplace(name, object);
}
void ExecutionState::addObjects(const Objects &newObjects,
                                const std::list<std::unique_ptr<ArgumentPair>> &argsNames) {
    auto &&namesIter = argsNames.begin();
    auto &&objectIter = newObjects.begin();
    for (;namesIter != argsNames.end() || objectIter != newObjects.end(); ++namesIter, ++objectIter) {
        if ((*namesIter)->getType() != (*objectIter)->getType()) {
            throw std::runtime_error("Wrong argument type: expected " + (*namesIter)->getType() +
                                     "get" + (*objectIter)->getType());
        }
        objects.emplace((*namesIter)->getName(), *objectIter);
    }
    if (namesIter != argsNames.end()) {
        throw std::runtime_error("Not enough arguments");
    }
    if (objectIter != newObjects.end()) {
        throw std::runtime_error("Too many arguments");
    }
}

bool ExecutionState::isReturning() {
    return retFlag;
}
void ExecutionState::setReturn(Object *object) {
    assert(retVal == nullptr);
    if (!object->Anonymous()) {
        /*
         * This might be named object stored somewhere else, but if it is in local scope,
         * make sure it survives execution state deletion.
         */
        for (auto &&i = localObjects.begin(); i != localObjects.end(); ++i) {
            if (*i == retVal) {
                (*i)->makeAnonymous();
                localObjects.erase(i);
                break;
            }
        }
    }
    retVal = object;
    retFlag = true;
}
Object* ExecutionState::getReturn() {
    return retVal;
}
