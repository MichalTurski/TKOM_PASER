//
// Created by michal on 10.11.18.
//

#include "BuildIn.h"
#include "../LibraryInterface/Symbols.h"
#include "../LibraryInterface/Function.h"


Num::Num(): Object("Num") {}
Num* Num::clone() const {
    return new Num(*this);
}
Num* Num::clone(const std::string &reference) const {
    (void) reference;
    return clone();
}
Object* Num::evaluateMethod(const std::string &name, Objects &arguments) {
    return nullptr;
}

Reference::Reference(): Object("Reference"), object(nullptr) {}
Reference* Reference::clone() const {
    return new Reference(*this);
}
Reference* Reference::clone(const std::string &reference) const {
    (void) reference;
    return clone();
}
Object* Reference::evaluate(Objects &arguments) {
    if (object) {
        //There is not an owner object, this is function reference.
        return object->evaluateMethod(name, arguments);
    } else {
        Function *function = symbols.getFunction(name);
        return function->evaluate(arguments);
    }
}
Object* Reference::evaluateMethod(const std::string &name, Objects &arguments) {
    if(name == "evaluate") {
        return evaluate(arguments);
    }
    return nullptr;
}
void Reference::setObject(Object *object) {
    this->object = object;
}
void Reference::setName(const std::string &name) {
    this->name = name;
}

String::String(): Object("String"), value("") {}
String* String::clone() const {
    return new String(*this);
}
String* String::clone(const std::string &reference) const {
    (void) reference;
    return clone();
}
Object* String::evaluateMethod(const std::string &name, Objects &arguments) {
    return nullptr;
}
