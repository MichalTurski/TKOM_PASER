//
// Created by michal on 10.11.18.
//

#ifndef PROJEKT_BUILDIN_H
#define PROJEKT_BUILDIN_H

#include "../LibraryInterface/Object.h"
#include "ExecutionState.h"

class Num: public Object {
public:
    int value;

    Num();
    Num *clone() const override;
    Num *clone(const std::string &reference) const override;
    Object *evaluateMethod(const std::string &name, Objects &arguments) override;
};
class Reference: public Object {
private:
    Object *object;
    std::string name;
    Object *evaluate(Objects &arguments);
public:
    Reference();
    Reference *clone() const override;
    Reference *clone(const std::string &reference) const override;
    Object *evaluateMethod(const std::string &name, Objects &arguments) override;
    void setObject(Object *object);
    void setName(const std::string &name);
};
class String: public Object {
public:
    std::string value;

    String();
    String *clone() const override;
    String *clone(const std::string &reference) const override;
    Object *evaluateMethod(const std::string &name, Objects &arguments) override;
};

#endif //PROJEKT_BUILDIN_H
