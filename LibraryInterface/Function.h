//
// Created by michal on 28.11.18.
//

#ifndef PROJEKT_EXTERNFUNCTION_H
#define PROJEKT_EXTERNFUNCTION_H

#include <string>
#include <list>
#include "Object.h"

class Function {
public:
    virtual Object *evaluate(Objects &objects) = 0;
};

class ExternFunction: public Function {
private:
    Object *(*function)(Objects &arguments);
public:
    explicit ExternFunction(Object *(*function)(Objects &arguments));
    Object *evaluate(Objects &objects) override;
};


#endif //PROJEKT_EXTERNFUNCTION_H
