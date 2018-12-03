//
// Created by michal on 08.11.18.
//

#include <iostream>
#include <assert.h>
#include <dlfcn.h>

#include "GrammarNode.h"
#include "BuildIn.h"
#include "../LibraryInterface/Symbols.h"
#include "ExecutionState.h"
#include "../LibraryInterface/Function.h"

Symbols symbols;

int Program::execute(const std::list<std::string> &libNames) {
    ExecutionState state;
    int retVal;
    for (auto &&lib: libNames) {
        loadLibrary(lib);
    }
    for (auto &&function : functions){
        symbols.addLocalFunction(*function);
    }
    /* TODO:
    for (auto &&group : groups){
        state.variables.addGroup(*group);
        group->init(state);
    }*/
    instructionSet->execute(state);
    if (state.isReturning()) {
        Object *retObj = state.getReturn();
        if (Num *retNum = dynamic_cast<Num *>(retObj)) {
            retVal = retNum->value;
            free(retObj);
            return retVal;
        } else {
            error("Can't return not-Num from program.");
        }
    }
}
void Program::loadLibrary(const std::string &name) {
    void (*loader)(Symbols &);
    void *lib = dlopen(name.c_str(), RTLD_NOW);
    if (lib) {
        loader = (void (*)(Symbols &))dlsym(lib, "init");
        if (!dlerror()) {
            loader(symbols);
            dlclose(lib);
            return;
        }
    }
    error(dlerror());
}
Object *FunctionDefinition::evaluate(Objects &arguments) {
    ExecutionState calleeState;
    calleeState.addObjects(arguments, argumentsList);
    body->execute(calleeState);
    return calleeState.getReturn();
}
std::string& FunctionDefinition::getName() {
    return name;
}
const std::string& ArgumentPair::getType() const {
    return type;
}
const std::string& ArgumentPair::getName() const {
    return name;
}
void InstructionSet::execute(ExecutionState &state) {
    for (auto &i: statements) {
        i->execute(state);
        if (state.isReturning()) {
            return;
        }
    }
}
void ReturnStatement::execute(ExecutionState &state) {
    state.setReturn(expr->evaluate(state));
}
void IfStatement::execute(ExecutionState &state) {
    Object *conditionObj = condition->evaluate(state);
    Num *conditionVal;
    if (conditionVal = dynamic_cast<Num *>(conditionVal)) {
        if (conditionVal->value > 0) {
            body->execute(state);
        }
    } else {
        error("Improper type of condition");
    }
}
void ForStatement::execute(ExecutionState &state) {
    //TODO
}
void Assignment::execute(ExecutionState &state) {
    Object *object;
    object = rVal->evaluate(state);
    object->makeNamed(); /* Make object not anonymous to avoid freeing its memory */
    state.addObject(lVal, object);
}
void VariableDefinition::execute(ExecutionState &state) {
    Object *newObject;
    if (!(newObject = symbols.createObject(type, reference))) {
        error("There is not such a type");
    }
    state.addObject(name, newObject);
}
Object *LogicExpr::evaluate(ExecutionState &state) {
    Object *object;
    Num *curr, *accumulator;
    auto exprIter = exprList.begin();
    object = (*exprIter)->evaluate(state);
    if(!operators.empty()) {
        if (accumulator = dynamic_cast<Num *>(object)) {
            if (!accumulator->Anonymous()) {
                accumulator = accumulator->clone();
            }
            for (auto &&i: operators) {
                object = (*++exprIter)->evaluate(state);
                if (!(curr = dynamic_cast<Num *>(object))) {
                    error("Can't add not-Num object.");
                }
                switch (i) {
                    case orOp:
                        accumulator->value = accumulator->value || curr->value;
                        break;
                    case andOp:
                        accumulator->value = accumulator->value && curr->value;
                        break;
                    default:
                        error("Wrong operator.");
                }
                if (curr->Anonymous()) {
                    delete curr;
                }
            }
            if (negated) {
                accumulator->value = ! accumulator->value;
            }
            object = accumulator;
        } else {
            error("Can't add not-Num object.");
        }
    }
    return object;
}
void  LogicExpr::negate() {
    negated = !negated;
}
Object *CmpExpr::evaluate(ExecutionState &state) {
    Object *object;
    Num *curr, *accumulator;
    auto exprIter = exprList.begin();
    object = (*exprIter)->evaluate(state);
    if(!operators.empty()) {
        if (accumulator = dynamic_cast<Num *>(object)) {
            if (!accumulator->Anonymous()) {
                accumulator = accumulator->clone();
            }
            for (auto &&i: operators) {
                object = (*++exprIter)->evaluate(state);
                if (!(curr = dynamic_cast<Num *>(object))) {
                    error("Can't add not-Num object.");
                }
                switch (i) {
                    case moreOp:
                        accumulator->value = accumulator->value > curr->value;
                        break;
                    case lessOp:
                        accumulator->value = accumulator->value < curr->value;
                        break;
                    case eqOp:
                        accumulator->value = accumulator->value == curr->value;
                        break;
                    case neqOp:
                        accumulator->value = accumulator->value != curr->value;
                        break;
                    case moreEqOp:
                        accumulator->value = accumulator->value >= curr->value;
                        break;
                    case lessEqOp:
                        accumulator->value = accumulator->value <= curr->value;
                        break;
                    default:
                        error("Wrong operator.");
                }
                if (curr->Anonymous()) {
                    delete curr;
                }
            }
            object = accumulator;
        } else {
            error("Can't add not-Num object.");
        }
    }
    return object;
}
Object *AddExpr::evaluate(ExecutionState &state) {
    Object *object;
    Num *curr, *accumulator;
    auto exprIter = exprList.begin();
    object = (*exprIter)->evaluate(state);
    if(!operators.empty()) {
        if (accumulator = dynamic_cast<Num *>(object)) {
            if (!accumulator->Anonymous()) {
                accumulator = accumulator->clone();
            }
            for (auto &&i: operators) {
                object = (*++exprIter)->evaluate(state);
                if (!(curr = dynamic_cast<Num *>(object))) {
                    error("Can't add not-Num object.");
                }
                switch (i) {
                    case addOp:
                        accumulator->value = accumulator->value + curr->value;
                        break;
                    case subOp:
                        accumulator->value = accumulator->value - curr->value;
                        break;
                    default:
                        error("Wrong operator.");
                }
                if (curr->Anonymous()) {
                    delete curr;
                }
            }
            object = accumulator;
        } else {
            error("Can't add not-Num object.");
        }
    }
    return object;
}
Object *MultExpr::evaluate(ExecutionState &state) {
    Object *object;
    Num *curr, *accumulator;
    auto exprIter = exprList.begin();
    object = (*exprIter)->evaluate(state);
    if(!operators.empty()) {
        if (accumulator = dynamic_cast<Num *>(object)) {
            if (!accumulator->Anonymous()) {
                accumulator = accumulator->clone();
            }
            for (auto &&i: operators) {
                object = (*++exprIter)->evaluate(state);
                if (!(curr = dynamic_cast<Num *>(object))) {
                    error("Can't add not-Num object.");
                }
                switch (i) {
                    case multOp:
                        accumulator->value = accumulator->value * curr->value;
                        break;
                    case divOp:
                        accumulator->value = accumulator->value / curr->value;
                        break;
                    default:
                        error("Wrong operator.");
                }
                if (curr->Anonymous()) {
                    delete curr;
                }
            }
            object = accumulator;
        } else {
            error("Can't add not-Num object.");
        }
    }
    return object;
}
Object *MethodCall::evaluate(ExecutionState &state) {
    Object *objectPtr, *argument;
    Objects argObjects;
    if (objectPtr = state.getObject(object)) {
        for (auto &&i : arguments) {
            argument = state.getObject(i->getName());
            if (argument == nullptr) {
                error("Wrong argument name.");
            }
            argObjects.emplace_back(argument);
        }
        return objectPtr->evaluateMethod(method, argObjects);
    }/* else {
        try {
            GroupDefinition &group = state.getGroup(object);
            return group.executeMethod(state, method, arguments);
        } catch (std::runtime_error &except) {
            error(except.what());
        }
    }*/
    error("There is not such an object.");
}
void MethodCall::execute(ExecutionState &state) {
    Object *obj = evaluate(state);
    if (obj != nullptr) {
        assert(obj->Anonymous());
        delete (obj);
    }
}
Object* FunctionCall::evaluate(ExecutionState &state) {
    Object *referenceObj, *argument;
    Objects argObjects;
    Function *function;
    for (auto &&i : arguments) {
        argument = state.getObject(i->getName());
        if (argument == nullptr) {
            error("Wrong argument name.");
        }
        argObjects.emplace_back(argument);
    }
    if ((referenceObj = state.getObject(name)) && (referenceObj->getType() == "Reference")) {
        return referenceObj->evaluateMethod("evaluate", argObjects);
    } else if (function = symbols.getFunction(name)) {
        return function->evaluate(argObjects);
    } else {
        error("There is not such a function");
    }
}
void FunctionCall::execute(ExecutionState &state) {
    Object *obj = evaluate(state);
    if (obj != nullptr) {
        assert(obj->Anonymous());
        delete (obj);
    }
}
Object* FunctionRef::evaluate(ExecutionState &state) {
    Reference *reference = new(Reference);
    reference->setName(name);
    return reference;
}
Object* MethodRef::evaluate(ExecutionState &state) {
    Reference *reference = new(Reference);
    Object *objectPtr = state.getObject(this->object);
    if (objectPtr == nullptr) {
        error("Not such a object");
    }
    reference->setName(method);
    reference->setObject(objectPtr);
    return reference;
}
Object *ConstString::evaluate(ExecutionState &state) {
    String *string = new(String);
    string->value = value;
    return string;
}
Object *ConstNum::evaluate(ExecutionState &state) {
    Num *num = new(Num);
    num->value = value;
    return num;
}
Object *Variable::evaluate(ExecutionState &state) {
    Object *obj;
    if (obj = state.getObject(name)) {
        return obj;
    }
    error("There is not such an object");
}
const std::string &Variable::getName() const {
    return name;
}
//TODO:: add other methods there

void Node::error(const char *msg) {
    std::string errorMsg = "Can't execute code in line " + textPos.line;
    errorMsg += " column " + textPos.num;
    errorMsg += "\n";
    errorMsg += msg;
    errorMsg += "\n";
    throw std::runtime_error(errorMsg);
}

void Variable::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Variable" << std::endl;
    std::cout << std::string(setw, ' ') << "expr = " << name << std::endl;
}
void ConstNum::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = ConstNum" << std::endl;
    std::cout << std::string(setw, ' ') << "expr = " << value << std::endl;
}
void FunctionCall::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = FunctionCall" << std::endl;
    std::cout << std::string(setw, ' ') << "function name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "arguments: {" << std::endl;
    for (auto &&i: arguments) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void ConstString::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = ConstString" << std::endl;
    std::cout << std::string(setw, ' ') << "expr = " << value << std::endl;
}
void MethodCall::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = MethodCall" << std::endl;
    std::cout << std::string(setw, ' ') << "object name = " << object << std::endl;
    std::cout << std::string(setw, ' ') << "method name = " << method << std::endl;
    std::cout << std::string(setw, ' ') << "arguments: {" << std::endl;
    for (auto &&i: arguments) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void MultExpr::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = MultExpr" << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto&& i: exprList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "operators {" << std::endl;
    for (TokenType i: operators) {
        std::cout << std::string(setw + 4, ' ') << i << std::endl;
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void AddExpr::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = AddExpr" << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto&& i: exprList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "operators {" << std::endl;
    for (TokenType i: operators) {
        std::cout << std::string(setw + 4, ' ') << i << std::endl;
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void CmpExpr::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = CmpExpr" << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto&& i: exprList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "operators {" << std::endl;
    for (TokenType i: operators) {
        std::cout << std::string(setw + 4, ' ') << i << std::endl;
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void LogicExpr::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = LogicExpr" << std::endl;
    std::cout << std::string(setw, ' ') << "negated = " << negated << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto&& i: exprList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "operators {" << std::endl;
    for (TokenType i: operators) {
        std::cout << std::string(setw + 4, ' ') << i << std::endl;
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void FunctionRef::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = FunctionRef" << std::endl;
    std::cout << std::string(setw, ' ') << "function name = " << name << std::endl;
}
void MethodRef::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = MethodRef" << std::endl;
    std::cout << std::string(setw, ' ') << "class name = " << object << std::endl;
    std::cout << std::string(setw, ' ') << "method name = " << method << std::endl;
}
void Assignment::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Assignment" << std::endl;
    std::cout << std::string(setw, ' ') << "lValue = " << lVal << std::endl;
    std::cout << std::string(setw, ' ') << "rValue {" << std::endl;
    rVal->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void VariableDefinition::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = VariableDefinition" << std::endl;
    std::cout << std::string(setw, ' ') << "type = " << type << std::endl;
    std::cout << std::string(setw, ' ') << "name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "reference = " << reference << std::endl;
}
void IfStatement::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = IfStatement" << std::endl;
    std::cout << std::string(setw, ' ') << "conditon {" << std::endl;
    condition->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "body {" << std::endl;
    body->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void ForStatement::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = ForStatement" << std::endl;
    std::cout << std::string(setw, ' ') << "iteratorName = " << iteratorName << std::endl;
    std::cout << std::string(setw, ' ') << "variable = " << variable << std::endl;
    std::cout << std::string(setw, ' ') << "body {" << std::endl;
    body->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void ReturnStatement::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = ReturnStatement" << std::endl;
    std::cout << std::string(setw, ' ') << "expr {" << std::endl;
    expr->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void InstructionSet::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = InstructionSet {" << std::endl;
    for (std::unique_ptr<Statement> const &i: statements) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void ArgumentPair::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = ArgumentPair" << std::endl;
    std::cout << std::string(setw, ' ') << "type = " << type << std::endl;
    std::cout << std::string(setw, ' ') << "name = " << name << std::endl;
}
void FunctionDefinition::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = FunctionDefinition" << std::endl;
    std::cout << std::string(setw, ' ') << "name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "arguments {" << std::endl;
    for (auto &&i: argumentsList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "body {" << std::endl;
    body->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void GroupDefinition::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = GroupDefinition" << std::endl;
    std::cout << std::string(setw, ' ') << "name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "methods {" << std::endl;
    for (auto &&i: methodsList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void Program::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Program" << std::endl;
    std::cout << std::string(setw, ' ') << "functions {" << std::endl;
    for (std::unique_ptr<FunctionDefinition> const &i: functions) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "groups {" << std::endl;
    for (auto &&i: groups) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw + 4, ' ') << "instruction set {" << std::endl;
    instructionSet->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
