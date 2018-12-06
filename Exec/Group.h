//
// Created by michal on 04.12.18.
//

#ifndef PROJEKT_GROUP_H
#define PROJEKT_GROUP_H


#include "../LibraryInterface/Object.h"
#include "GrammarNode.h"

class Group: public Object {
private:
    std::string groupName;
    std::map<std::string, FunctionDefinition &> functions;
    std::map<std::string, Object *> fields;
public:
    Group(GroupDefinition &groupDefinition);
    ~Group();
    const std::string &getName() const;
    Object *clone(const std::string &reference) const override;
    Object *evaluateMethod(const std::string &name, Objects &arguments) override;
};

#endif //PROJEKT_GROUP_H
