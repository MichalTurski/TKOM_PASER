//
// Created by michal on 08.11.18.
//

#include <iostream>
#include <assert.h>
#include <dlfcn.h>
#include <sstream>

#include "GrammarNode.h"
#include "../LibraryInterface/BuildIn.h"
#include "../LibraryInterface/Symbols.h"
#include "ExecutionState.h"
#include "../LibraryInterface/Function.h"
#include "Reference.h"

Symbols symbols;

Program::Program(const TextPos &textPos, std::list<std::unique_ptr<FunctionDefinition>> &&functions,
        std::list<std::unique_ptr<GroupDefinition>> &&groups,
        std::unique_ptr<InstructionSet> &&instructionSet):
        Node(textPos),
        functions(std::move(functions)),
        groups(std::move(groups)),
        instructionSet(std::move(instructionSet)) {};
Program::~Program() {
    for (auto &&lib :libraries) {
        dlclose(lib);
    }
}
int Program::execute(const std::list<std::string> &libNames) {
    ExecutionState state;
    int retVal;
    for (auto &&lib: libNames) {
        loadLibrary(lib);
    }
    for (auto &&function : functions){
        symbols.addLocalFunction(*function);
    }
    for (auto &&group : groups){
        state.addGroup(*group);
    }
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
    char *errStr;
    void *lib = dlopen(name.c_str(), RTLD_LAZY);
    if (lib) {
        *(void**)(&loader) = dlsym(lib, "init");
        if (!(errStr = dlerror())) {
            loader(symbols);
            libraries.emplace_back(lib);
            return;
        }
    } else {
        errStr = dlerror();
    }
    error(errStr);
}
GroupDefinition::GroupDefinition(const TextPos &textPos, std::string &&name,
                                 std::list<std::unique_ptr<VariableDefinition>> &&fieldsList,
                                 std::list<std::unique_ptr<FunctionDefinition>> &&methodsList):
                                    Node(textPos),
                                    name(std::move(name)),
                                    fieldsList(std::move(fieldsList)),
                                    methodsList(std::move(methodsList)){}
FunctionDefinition::FunctionDefinition(const TextPos &textPos, std::string &&name,
                   std::list<std::unique_ptr<ArgumentPair>> &&argumentsList,
                   std::unique_ptr<InstructionSet> &&body):
                        Node(textPos),
                        name(std::move(name)),
                        argumentsList(std::move(argumentsList)),
                        body(std::move(body)) {}
Object *FunctionDefinition::evaluate(Objects &arguments) {
    ExecutionState calleeState;
    calleeState.handleObjects(arguments, argumentsList);
    body->execute(calleeState);
    return calleeState.getReturn();
}
Object* FunctionDefinition::evaluate(Objects &arguments, ExecutionState &state) {
    state.handleObjects(arguments, argumentsList);
    body->execute(state);
    return state.getReturn();
}
std::string& FunctionDefinition::getName() {
    return name;
}
ArgumentPair::ArgumentPair(const TextPos &textPos, std::string &&type, std::string &&name):
        Node(textPos),
        type(type),
        name(name) {}
const std::string& ArgumentPair::getType() const {
    return type;
}
const std::string& ArgumentPair::getName() const {
    return name;
}
InstructionSet::InstructionSet(const TextPos &textPos, std::list<std::unique_ptr<Statement>> &&statements):
        Node(textPos),
        statements(std::move(statements)){}
void InstructionSet::execute(ExecutionState &state) {
    for (auto &i: statements) {
        i->execute(state);
        if (state.isReturning()) {
            return;
        }
    }
}
ReturnStatement::ReturnStatement(const TextPos &textPos, std::unique_ptr<LogicExpr> value):
        Statement(textPos),
        Node(textPos),
        expr(std::move(value)) {}
void ReturnStatement::execute(ExecutionState &state) {
    state.setReturn(expr->evaluate(state));
}
ForStatement::ForStatement(const TextPos &textPos, std::string &&iterName, std::string &&variable,
             std::unique_ptr<InstructionSet> &&body): Statement(textPos),
                                                      Node(textPos),
                                                      iteratorName(iterName),
                                                      variable(variable),
                                                      body(std::move(body)) {}
void ForStatement::execute(ExecutionState &state) {
    //TODO
}
IfStatement::IfStatement(const TextPos &textPos, std::unique_ptr<LogicExpr> &&condition,
                         std::unique_ptr<InstructionSet> &&body):
        Statement(textPos),
        Node(textPos),
        condition(std::move(condition)),
        body(std::move(body)) {}
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
VariableDefinition::VariableDefinition(const TextPos &textPos, std::string &&type,
                                       std::string &&name, std::string &&reference):
        Instruction(textPos),
        Node(textPos),
        type(type),
        name(name),
        reference(reference){}
void VariableDefinition::execute(ExecutionState &state) {
    Object *newObject;
    if (!(newObject = symbols.createObject(type, reference))) {
        error("There is not such a type");
    }
    state.addObject(name, newObject);
}
const std::string& VariableDefinition::getType() const {
    return type;
}
const std::string& VariableDefinition::getName() const {
    return name;
}
Assignment::Assignment(const TextPos &textPos, std::string &&lVal, std::unique_ptr<LogicExpr> &&rVal):
        Instruction(textPos),
        Node(textPos),
        lVal(lVal),
        rVal(std::move(rVal)) {}
void Assignment::execute(ExecutionState &state) {
    Object *object;
    object = rVal->evaluate(state);
    if (object) {
        object->makeNamed(); /* Make object not anonymous to avoid freeing its memory */
        state.modifyObject(lVal, object);
//        state.addObject(lVal, object);
    } else {
        error("Can't assign invalid object.");
    }
}
MethodRef::MethodRef(const TextPos &textPos, std::string &&group, std::string &&method):
        ExprArgument(textPos),
        Node(textPos),
        object(std::move(group)),
        method(std::move(method)){}
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
FunctionRef::FunctionRef(const TextPos &textPos, std::string &&name):
        ExprArgument(textPos),
        Node(textPos),
        name(std::move(name)){}
Object* FunctionRef::evaluate(ExecutionState &state) {
    Reference *reference = new(Reference);
    reference->setName(name);
    return reference;
}
LogicExpr::LogicExpr(const TextPos &textPos, bool negated, std::list<std::unique_ptr<CmpExpr>> &&exprList,
          std::list<TokenType> &&operators): ExprArgument(textPos),
                                             Node(textPos),
                                             negated(negated),
                                             exprList(std::move(exprList)),
                                             operators(operators){}
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
CmpExpr::CmpExpr(const TextPos &textPos, std::list<std::unique_ptr<AddExpr>> &&exprList,
        std::list<TokenType> &&operators): Node(textPos),
                                           exprList(std::move(exprList)),
                                           operators(operators){}
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
AddExpr::AddExpr(const TextPos &textPos, bool negated, std::list<std::unique_ptr<MultExpr>> &&exprList,
        std::list<TokenType> &&operators): Node(textPos),
                                           negated(negated),
                                           exprList(std::move(exprList)),
                                           operators(operators){}
Object *AddExpr::evaluate(ExecutionState &state) {
    Object *object;
    Num *curr, *accumulator;
    auto exprIter = exprList.begin();
    object = (*exprIter)->evaluate(state);
    if (negated) {
        if (accumulator = dynamic_cast<Num *>(object)) {
            accumulator->value = - accumulator->value;
            object = accumulator;
        } else {
            error("Can't negate not-Num object.");
        }
    }
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
            error("Can't add or subtract not-Num object.");
        }
    }
    return object;
}
MultExpr::MultExpr(const TextPos &textPos, std::list<std::unique_ptr<ExprArgument>> &&exprList,
         std::list<TokenType> &&operators): Node(textPos),
                                            exprList(std::move(exprList)),
                                            operators(std::move(operators)){}
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
MethodCall::MethodCall(const TextPos &textPos, std::string &&object, std::string &&method, std::list<std::unique_ptr<Variable>> &&args):
        Instruction(textPos),
        ExprArgument(textPos),
        Node(textPos),
        objectName(std::move(object)),
        method(std::move(method)),
        arguments(std::move(args)) {}
Object *MethodCall::evaluate(ExecutionState &state) {
    Object *object, **argumentPtr;
    Objects argObjects;
    if (object = state.getObject(objectName)) {
        for (auto &&i : arguments) {
            argumentPtr = state.getObjectPtr(i->getName());
            if (argumentPtr == nullptr) {
                error("Wrong argument name.");
            }
            argObjects.emplace_back(argumentPtr);
        }
        return object->evaluateMethod(method, argObjects);
    }
    error("There is not such an object.");
}
void MethodCall::execute(ExecutionState &state) {
    Object *obj = evaluate(state);
    if (obj != nullptr) {
        assert(obj->Anonymous());
        delete (obj);
    }
}
FunctionCall::FunctionCall(const TextPos &textPos, std::string &&name, std::list<std::unique_ptr<Variable>> &&arguments):
        Instruction(textPos),
        ExprArgument(textPos),
        Node(textPos),
        name(name),
        arguments(std::move(arguments)) {}
Object* FunctionCall::evaluate(ExecutionState &state) {
    Object *referenceObj, **argumentPtr;
    Objects argObjects;
    Function *function;
    for (auto &&i : arguments) {
        argumentPtr = state.getObjectPtr(i->getName());
        if (argumentPtr == nullptr) {
            error("Wrong argument name.");
        }
        argObjects.emplace_back(argumentPtr);
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
ConstString::ConstString(const TextPos &textPos, std::string &&value):
        ExprArgument(textPos),
        Node(textPos),
        value(value) {}
Object *ConstString::evaluate(ExecutionState &state) {
    String *string = new(String);
    string->value = value;
    return string;
}
ConstNum::ConstNum(const TextPos &textPos, int val):
        Node(textPos),
        ExprArgument(textPos),
        value(val) {}
Object *ConstNum::evaluate(ExecutionState &state) {
    Num *num = new(Num);
    num->value = value;
    return num;
}
Variable::Variable(const TextPos &textPos, std::string &&name):
        ExprArgument(textPos),
        Node(textPos),
        name(name) {}
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

const TextPos& Node::getTextPos() const {
    return textPos;
}
void Node::error(const char *msg) {
    std::stringstream ss;
    ss << "Can't execute code in line " << textPos.line << " column " << textPos.num << std::endl;
    ss << msg << std::endl;
    throw std::runtime_error(ss.str());
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
    std::cout << std::string(setw, ' ') << "object name = " << objectName << std::endl;
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
