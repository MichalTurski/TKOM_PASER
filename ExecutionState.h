//
// Created by michal on 08.11.18.
//

#ifndef PROJEKT_EXECUTIONSTATE_H
#define PROJEKT_EXECUTIONSTATE_H


#include <string>
#include <bits/unique_ptr.h>
#include <map>
#include <list>
#include "Object.h"
#include "GrammarNode.h"

class ExecutionState {
private:
    std::list<Object *> localObjects;
    std::map<std::string, Object*> objects;

    Object *retVal = nullptr;
    bool retFlag = false;
public:
    ~ExecutionState();

    Object *getObject(const std::string &name);

    void addObject(const std::string &name, Object *object);
//    bool addVariable(const std::string &type, const std::string &name,
//                      const std::string &reference);
    void addObjects(const Objects &newObjects, const std::list<std::unique_ptr<ArgumentPair>> &argsNames);

    bool isReturning();
    void setReturn(Object *object);
    Object *getReturn();
};

#endif //PROJEKT_EXECUTIONSTATE_H
