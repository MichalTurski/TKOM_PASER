//
// Created by michal on 08.11.18.
//

#ifndef PROJEKT_GRAMMARNODE_H
#define PROJEKT_GRAMMARNODE_H

#include <string>
#include <bits/unique_ptr.h>
#include <list>

#include "Scaner.h"

class Node {
public:
    virtual void printValue(int setw) const = 0;
};

class InstructionSet;
class LogicExpr;

class ExprArgument: public virtual Node {
public:
    virtual void printValue(int setw) const = 0;
};

class FunArgument: public virtual Node {
public:
    virtual void printValue(int setw) const = 0;
};

class Variable: public ExprArgument {
private:
    std::string name;
public:
    Variable(std::string &&name): name(name) {}
    void printValue(int setw) const override;
};

class Number: public ExprArgument {
private:
    int value;
public:
    Number(int val): value(val) {}
    void printValue(int setw) const override;
};

class FunctionCall: public ExprArgument {
private:
    std::string name;
    std::list<std::unique_ptr<FunArgument>> arguments;
public:
    FunctionCall(std::string &&name, std::list<std::unique_ptr<FunArgument>> &&arguments):
            name(name),
            arguments(std::move(arguments)) {}
    void printValue(int setw) const override;
};

class String: public ExprArgument {
private:
    std::string value;
public:
    String(std::string &&value): value(value) {}
    void printValue(int setw) const override;
};

class MethodCall: public ExprArgument {
private:
    std::string object;
    std::string method;
    std::list<std::unique_ptr<FunArgument>> arguments;
public:
    MethodCall(std::string &&object, std::string &&method, std::list<std::unique_ptr<FunArgument>> &&args):
            object(std::move(object)),
            method(std::move(method)),
            arguments(std::move(args)) {}
    void printValue(int setw) const override;
};

class MultExpr: public Node {
private:
    std::list<std::unique_ptr<ExprArgument>> exprList;
    std::list<TokenType> operators;
public:
    MultExpr(std::list<std::unique_ptr<ExprArgument>> &&exprList,
             std::list<TokenType> &&operators): exprList(std::move(exprList)),
                                                operators(std::move(operators)){}
    void printValue(int setw) const override;
};

class AddExpr: public Node {
private:
    std::list<std::unique_ptr<MultExpr>> exprList;
    std::list<TokenType> operators;
public:
    AddExpr(std::list<std::unique_ptr<MultExpr>> &&exprList,
            std::list<TokenType> &&operators): exprList(std::move(exprList)),
                                               operators(operators){}
    void printValue(int setw) const override;
};

class CmpExpr: public Node {
private:
    std::list<std::unique_ptr<AddExpr>> exprList;
    std::list<TokenType> operators;
public:
    CmpExpr(std::list<std::unique_ptr<AddExpr>> &&exprList,
            std::list<TokenType> &&operators): exprList(std::move(exprList)),
                                               operators(operators){}
    void printValue(int setw) const override;
};

class Statement: public virtual Node {};

class Instruction: public virtual Statement {};

class LogicExpr: public Instruction, public ExprArgument, public FunArgument {
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

class FunctionRef: public FunArgument {
private:
    std::string name;
public:
    explicit FunctionRef(std::string &&name): name(std::move(name)){}
    void printValue(int setw) const override;
};

class MethodRef: public FunArgument {
private:
    std::string group;
    std::string method;
public:
    MethodRef(std::string &&group, std::string &&method):
            group(std::move(group)),
            method(std::move(method)){}
    void printValue(int setw) const override;
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

class InstructionSet:public Node {
private:
    std::list<std::unique_ptr<Statement>> statements;
public:
    InstructionSet(InstructionSet &&set) = default;
    InstructionSet(std::list<std::unique_ptr<Statement>> &&statements):
            statements(std::move(statements)){}
    void printValue(int setw) const override;
};

class ArgumentPair: public Node {
private:
    std::string type;
    std::string name;
public:
    ArgumentPair(std::string &&type, std::string &&name):
            type(type),
            name(name) {}
    void printValue(int setw) const override;
};

class FunctionDefinition: public Node {
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

class GroupDefinition: public Node {
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

class Program: public Node {
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
