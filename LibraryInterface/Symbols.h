//
// Created by michal on 29.11.18.
//

#ifndef PROJEKT_SYMBOLS_H
#define PROJEKT_SYMBOLS_H

#include <string>
#include <map>
#include <bits/unique_ptr.h>
#include "Object.h"
#include "ClassFactory.h"

class Function;
class FunctionDefinition;
class ClassFactory;

class Symbols {
private:
    std::map<std::string, FunctionDefinition&> localFunctions;
    std::map<std::string, ExternFunction*> externFunctions;
    ClassFactory classFactory;
public:
    Symbols();
    ~Symbols();

    Function *getFunction(const std::string &name);
    Object *createObject(const std::string &name, const std::string &reference);

    void addLocalFunction(FunctionDefinition &function);
    void addExternFunction(const std::string &name, ExternFunction *function);
    void addClass(const std::string &name, Object *prototype);
};

#endif //PROJEKT_SYMBOLS_H
