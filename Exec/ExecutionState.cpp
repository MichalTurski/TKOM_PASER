//
// Created by michal on 08.11.18.
//

#include <assert.h>
#include "ExecutionState.h"
#include "../LibraryInterface/BuildIn.h"
#include "../LibraryInterface/Symbols.h"
#include "GrammarNode.h"
#include "Group.h"

ExecutionState::~ExecutionState() {
    for (auto&&i : localObjects) {
        free(i);
    }
}
Object* ExecutionState::getObject(const std::string &name) {
    Object **pObject = getObjectPtr(name);
    if (pObject != nullptr) {
        return *pObject;
    } else {
        return nullptr;
    }
}
Object** ExecutionState::getObjectPtr(const std::string &name) {
    auto&& objectIter = objects.find(name);
    if (objectIter == objects.end()) {
        return nullptr;
    } else {
        return objectIter->second;
    }
}
void ExecutionState::addObject(const std::string &name, Object *object) {
    localObjects.emplace_back(object);
    Object **pObject = &localObjects.back();
    handleObject(name, pObject);
    /* In fact, we shouldn't allow to overwrite variable, hence we remove this code */
    /*Object *oldObject = *objects[name];
    if (oldObject) {
        for (auto &&i = localObjects.begin(); i != localObjects.end(); ++i) {
            if (*i == oldObject) {
                localObjects.erase(i);
                delete(oldObject);
                break;
            }
        }
    }
    objects[name] = object;*/
}
void ExecutionState::modifyObject(const std::string &name, Object *object) {
    Object **pObject = objects[name];
    if (pObject) {
        delete *pObject;
        *pObject = object;
        /*for (auto &&i = localObjects.begin(); i != localObjects.end(); ++i) {
            if (*i == *pObject) {
                localObjects.erase(i);
                delete(*pObject);
                localObjects.emplace_back(object)
                objects[name] = &localObjects.back();
            }
        }*/
    } else {
        throw std::runtime_error("There is no object called " + name);
    }
}
void ExecutionState::handleObject(const std::string &name, Object **object) {
    Object **oldObject = objects[name];
    if (oldObject == nullptr) {
        objects[name] = object;
    } else {
        throw std::runtime_error("Object name already in usage");
    }
}
void ExecutionState::handleObjects(const Objects &newObjects,
                                   const std::list<std::unique_ptr<ArgumentPair>> &argsNames) {
    auto &&namesIter = argsNames.begin();
    auto &&objectIter = newObjects.begin();
    for (;namesIter != argsNames.end() || objectIter != newObjects.end(); ++namesIter, ++objectIter) {
        if ((*namesIter)->getType() != (**objectIter)->getType()) {
            throw std::runtime_error("Wrong argument type: expected " + (*namesIter)->getType() +
                                     "get" + (**objectIter)->getType());
        }
        handleObject((*namesIter)->getName(), *objectIter);
    }
    if (namesIter != argsNames.end()) {
        throw std::runtime_error("Not enough arguments");
    }
    if (objectIter != newObjects.end()) {
        throw std::runtime_error("Too many arguments");
    }
}
void ExecutionState::addGroup(GroupDefinition &groupDefinition) {
    Group *group = new Group(groupDefinition);
    localObjects.emplace_back(group);
    handleObject(group->getName(), &localObjects.back());
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
