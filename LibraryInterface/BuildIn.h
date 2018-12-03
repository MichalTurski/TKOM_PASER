//
// Created by michal on 10.11.18.
//

#ifndef PROJEKT_BUILDIN_H
#define PROJEKT_BUILDIN_H

#include "Object.h"
#include "../Exec/ExecutionState.h"

class Num: public Object {
public:
    int value;

    Num();
    Num *clone() const;
    Num *clone(const std::string &reference) const override;
    Object *evaluateMethod(const std::string &name, Objects &arguments) override;
};
class String: public Object {
public:
    std::string value;

    String();
    String *clone() const;
    String *clone(const std::string &reference) const override;
    Object *evaluateMethod(const std::string &name, Objects &arguments) override;
};

#endif //PROJEKT_BUILDIN_H
