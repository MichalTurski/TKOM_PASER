//
// Created by michal on 06.12.18.
//

#ifndef PROJEKT_COLLECTION_H
#define PROJEKT_COLLECTION_H


#include "../LibraryInterface/Object.h"

class Collection: public Object {
    std::vector<Object**> objects;
    int currObjIdx;
    std::string collectionType;

    void addElements(Objects &arguments);
public:
    Collection();
//    void addObject(Object *object);
    Object **iterate();
    Object *clone(const std::string &str) const override;
    Object *evaluateMethod(const std::string &name, Objects &arguments) override;
};


#endif //PROJEKT_COLLECTION_H
