//
// Created by michal on 29.11.18.
//

#include "Symbols.h"
#include "GrammarNode.h"
#include "BuildIn.h"

Symbols::Symbols() {
    Num *numPrototype = new Num;
    Reference *referencePrototype = new Reference;
    String *stringPrototype = new String;
    classFactory.addClass("Num", numPrototype);
    classFactory.addClass("Reference", referencePrototype);
    classFactory.addClass("String", stringPrototype);
}
Symbols::~Symbols() {
    /*for (auto &&i : localFunctions) {
        delete i.second;
    }*/
    for (auto &&i : externFunctions) {
        delete i.second;
    }
}

Function* Symbols::getFunction(const std::string &name) {
    auto  &&externFunction = externFunctions.find(name);
    if (externFunction != externFunctions.end()) {
        return externFunction->second;
    }
    auto &&localFunction = localFunctions.find(name);
    if (localFunction != localFunctions.end()) {
        return &(localFunction->second);
    }
    return nullptr;
}
Object* Symbols::createObject(const std::string &name, const std::string &reference) {
    return classFactory.getObject(name, reference);
}

void Symbols::addClass(const std::string name, Object *prototype) {
    classFactory.addClass(name, prototype);
}
void Symbols::addLocalFunction(FunctionDefinition &function) {
    localFunctions.emplace(function.getName(), function);
}
void Symbols::addExternFunction(const std::string &name, ExternFunction *function) {
    externFunctions.emplace(name, function);
}