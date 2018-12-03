//
// Created by michal on 03.12.18.
//

#ifndef PROJEKT_REFERENCE_H
#define PROJEKT_REFERENCE_H

#include "../LibraryInterface/Object.h"

class Reference: public Object {
private:
    Object *object;
    std::string name;
    Object *evaluate(Objects &arguments);
public:
    Reference();
    Reference *clone() const;
    Reference *clone(const std::string &reference) const override;
    Object *evaluateMethod(const std::string &name, Objects &arguments) override;
    void setObject(Object *object);
    void setName(const std::string &name);
};

#endif //PROJEKT_REFERENCE_H
