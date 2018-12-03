//
// Created by michal on 28.11.18.
//

#include "Function.h"

ExternFunction::ExternFunction(Object *(*function)(Objects &)):
        function(function) {}
Object* ExternFunction::evaluate(Objects &objects) {
    return function(objects);
}
