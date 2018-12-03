//
// Created by michal on 10.11.18.
//

#include "BuildIn.h"
#include "Symbols.h"
#include "Function.h"

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
