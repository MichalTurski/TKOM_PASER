//
// Created by michal on 30.11.18.
//

#include "Object.h"
#include "Function.h"

Object::Object(const std::string &type): type(type), anonymous(true) {}
Object::Object(const Object &other): type(other.type), anonymous(true) {}
/*Object* Object::evaluateMethod(const std::string &name, Objects &arguments) {
    Objects
    auto &&methodPair = methods.find(name);
    if (methodPair == methods.end()) {
        throw std::runtime_error("No method called " + name + "in class " + type);
    }
    ExternFunction &method = methodPair->second;
    return method.evaluate(arguments);
}*/
void Object::makeAnonymous() {
    anonymous = true;
}
void Object::makeNamed() {
    anonymous = false;
}
const std::string& Object::getType() {
    return type;
}
bool Object::Anonymous() {
    return anonymous;
}
