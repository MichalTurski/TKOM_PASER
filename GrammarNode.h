//
// Created by michal on 08.11.18.
//

#ifndef PROJEKT_GRAMMARNODE_H
#define PROJEKT_GRAMMARNODE_H

#include <string>
#include <bits/unique_ptr.h>
#include <list>

#include "Scaner.h"

class ParserNode {
public:
    virtual void printValue(int setw) const = 0;
};

class InstructionSet;
class LogicExpr;

class Argument: public virtual ParserNode {
public:
    virtual void printValue(int setw) const = 0;
};

class Variable: public Argument {
private:
    std::string name;
public:
    Variable(std::string &&name): name(name) {}
    void printValue(int setw) const override;
};

class Number: public Argument {
private:
    int value;
public:
    Number(int val): value(val) {}
    void printValue(int setw) const override;
};

class FunctionCall: public Argument {
private:
    std::string name;
    std::list<std::unique_ptr<LogicExpr>> arguments;
public:
    FunctionCall(std::string &&name, std::list<std::unique_ptr<LogicExpr>> &&arguments):
            name(name),
            arguments(std::move(arguments)) {}
    void printValue(int setw) const override;
};

class String: public Argument {
private:
    std::string value;
public:
    String(std::string &&value): value(value) {}
    void printValue(int setw) const override;
};

class MethodCall: public Argument {
private:
    std::string object;
    std::string method;
    std::list<std::unique_ptr<LogicExpr>> arguments;
public:
    MethodCall(std::string &&object, std::string &&method, std::list<std::unique_ptr<LogicExpr>> &&args):
            object(std::move(object)),
            method(std::move(method)),
            arguments(std::move(args)) {}
    void printValue(int setw) const override;
};

class MultExpr: public ParserNode {
private:
    std::list<std::unique_ptr<Argument>> exprList;
    std::list<TokenType> operators;
public:
    MultExpr(std::list<std::unique_ptr<Argument>> &&exprList,
             std::list<TokenType> &&operators): exprList(std::move(exprList)),
                                                operators(std::move(operators)){}
    void printValue(int setw) const override;
};

class AddExpr: public ParserNode {
private:
    std::list<std::unique_ptr<MultExpr>> exprList;
    std::list<TokenType> operators;
public:
    AddExpr(std::list<std::unique_ptr<MultExpr>> &&exprList,
            std::list<TokenType> &&operators): exprList(std::move(exprList)),
                                               operators(operators){}
    void printValue(int setw) const override;
};

class CmpExpr: public ParserNode {
private:
    std::list<std::unique_ptr<AddExpr>> exprList;
    std::list<TokenType> operators;
public:
    CmpExpr(std::list<std::unique_ptr<AddExpr>> &&exprList,
            std::list<TokenType> &&operators): exprList(std::move(exprList)),
                                               operators(operators){}
    void printValue(int setw) const override;
};

class Statement: public virtual ParserNode {};

class Instruction: public virtual Statement {};

class LogicExpr: public Instruction, public Argument {
private:
    bool negated;
    std::list<std::unique_ptr<CmpExpr>> exprList;
    std::list<TokenType> operators;
public:
    LogicExpr(bool negated, std::list<std::unique_ptr<CmpExpr>> &&exprList,
              std::list<TokenType> &&operators): negated(negated),
                                                 exprList(std::move(exprList)),
                                                 operators(operators){}
    void printValue(int setw) const override;
    void negate();
};

class Assignment: public Instruction {
private:
    std::string lVal;
    std::unique_ptr<LogicExpr> rVal;
public:
    Assignment(std::string &&lVal, std::unique_ptr<LogicExpr> &&rVal):
            lVal(lVal),
            rVal(std::move(rVal)) {}
    void printValue(int setw) const override;
};

class VariableDefinition: public Instruction {
private:
    std::string type;
    std::string name;
    std::string reference;
public:
    VariableDefinition(std::string &&type, std::string &&name, std::string &&reference):
            type(type),
            name(name),
            reference(reference){}
    void printValue(int setw) const override;
};

class IfStatement: public Statement {
private:
    std::unique_ptr<LogicExpr> condition;
    std::unique_ptr<InstructionSet> body;
public:
    IfStatement(std::unique_ptr<LogicExpr> &&condition, std::unique_ptr<InstructionSet> &&body):
            condition(std::move(condition)),
            body(std::move(body)) {}
    void printValue(int setw) const override;
};

class ForStatement: public Statement {
private:
    std::string iteratorName;
    std::string variable;
    std::unique_ptr<InstructionSet> body;
public:
    ForStatement(std::string &&iterName, std::string &&variable,
                 std::unique_ptr<InstructionSet> &&body): iteratorName(iterName),
                                                          variable(variable),
                                                          body(std::move(body)) {}
    void printValue(int setw) const override;

};

class ReturnStatement: public Statement {
    std::unique_ptr<LogicExpr> value;
public:
    ReturnStatement(std::unique_ptr<LogicExpr> value):
            value(std::move(value)) {}
    void printValue(int setw) const override;
};

class InstructionSet:public ParserNode {
private:
    std::list<std::unique_ptr<Statement>> statements;
public:
    InstructionSet(InstructionSet &&set) = default;
    InstructionSet(std::list<std::unique_ptr<Statement>> &&statements):
            statements(std::move(statements)){}
    void printValue(int setw) const override;
};

class ArgumentPair: public ParserNode {
private:
    std::string type;
    std::string name;
public:
    ArgumentPair(std::string &&type, std::string &&name):
            type(type),
            name(name) {}
    void printValue(int setw) const override;
};

class FunctionDefinition: public ParserNode {
private:
    std::string name;
    std::unique_ptr<std::list<std::unique_ptr<ArgumentPair>>> argumentsList;
    std::unique_ptr<InstructionSet> body;
public:
    FunctionDefinition(std::string &&name,
                       std::unique_ptr<std::list<std::unique_ptr<ArgumentPair>>> &&argumentsList,
    std::unique_ptr<InstructionSet> &&body):
    name(std::move(name)),
    argumentsList(std::move(argumentsList)),
    body(std::move(body)) {}
    void printValue(int setw) const override;
};

class GroupDefinition: public ParserNode {
private:
    std::string name;
    std::list<std::unique_ptr<VariableDefinition>> fieldsList;
    std::list<std::unique_ptr<FunctionDefinition>> methodsList;
public:
    GroupDefinition(std::string &&name, std::list<std::unique_ptr<VariableDefinition>> &&fieldsList,
                    std::list<std::unique_ptr<FunctionDefinition>> &&methodsList):
            name(std::move(name)),
            fieldsList(std::move(fieldsList)),
            methodsList(std::move(methodsList)){}
    void printValue(int setw) const override;
};

class Program: public ParserNode {
    std::unique_ptr<InstructionSet> instructionSet;
    std::list<std::unique_ptr<FunctionDefinition>> functions;
    std::list<std::unique_ptr<GroupDefinition>> groups;
public:
    Program(std::list<std::unique_ptr<FunctionDefinition>> &&functions,
            std::list<std::unique_ptr<GroupDefinition>> &&groups,
            std::unique_ptr<InstructionSet> &&instructionSet):
            functions(std::move(functions)),
            groups(std::move(groups)),
            instructionSet(std::move(instructionSet)) {};
    void printValue(int setw) const override;
//    void parse(Source &src, Scaner &scan);1
};

#endif //PROJEKT_GRAMMARNODE_H
