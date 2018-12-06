//
// Created by michal on 04.12.18.
//

#include "Group.h"

extern Symbols symbols;

Group::Group(GroupDefinition &groupDefinition): Object("Group") {
    groupName = groupDefinition.name;
    for (auto &&function : groupDefinition.methodsList) {
        functions.emplace(function->getName(), *function);
    }
    for (auto &&field : groupDefinition.fieldsList) {
        fields.emplace(field->getName(), symbols.createObject(field->getType(), ""));
    }
}
Group::~Group() {
    for (auto && field : fields) {
        delete(field.second);
    }
}
const std::string& Group::getName() const {
    return groupName;
}
Object* Group::clone(const std::string &reference) const {
    return nullptr;
}
Object* Group::evaluateMethod(const std::string &name, Objects &arguments) {
    ExecutionState state;
    for (auto &&field : fields) {
        state.handleObject(field.first, &field.second);
    }
    auto &&functionsIter = functions.find(name);
    if (functionsIter != functions.end()) {
        return functionsIter->second.evaluate(arguments, state);
    }
    throw std::runtime_error("There is not a method \"" + name + " in group " + groupName);
}