//
// Created by michal on 20.04.18.
//

#ifndef PROJEKT_PARSER_H
#define PROJEKT_PARSER_H

#include <list>
#include <bits/unique_ptr.h>
#include <memory>

#include "Source.h"
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
    std::string name;
    std::unique_ptr<FunctionCall> function;
public:
    MethodCall(std::string &&name, std::unique_ptr<FunctionCall> &&function):
            name(name),
            function(std::move(function)) {}
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

class Program: public ParserNode {
    std::unique_ptr<InstructionSet> instructionSet;
    std::list<std::unique_ptr<FunctionDefinition>> Functions;
public:
    Program(std::list<std::unique_ptr<FunctionDefinition>> &&Functions,
            std::unique_ptr<InstructionSet> &&instructionSet):
            Functions(std::move(Functions)),
            instructionSet(std::move(instructionSet)) {};
    void printValue(int setw) const override;
//    void parse(Source &src, Scaner &scan);1
};

class Parser {
    Scaner &scan;
    Source &src;
//    Token getToken(std::list<Token> &unused);
//    Token nextToken(std::list<Token> &unused);

    std::unique_ptr<std::string> errString(const TextPos &pos, const std::string &exp, const std::string &got);
    std::unique_ptr<InstructionSet> parseInstructionSet();
    std::unique_ptr<FunctionDefinition> parseFunctionDefinition();
    std::unique_ptr<std::list<std::unique_ptr<ArgumentPair>>> parseArgumentsList();
    std::unique_ptr<ArgumentPair> parseArgumentPair();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::unique_ptr<ForStatement> parseForStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<Instruction> parseInstruction();
    std::unique_ptr<Assignment> parseAssignment();
    std::unique_ptr<VariableDefinition> parseVariableDefinition();
  //  std::unique_ptr<LogicExpr> parseLogicExpr();
    std::unique_ptr<LogicExpr> parseLogicExpr();
    std::unique_ptr<CmpExpr> parseCmpExpr();
    std::unique_ptr<AddExpr> parseAddExpr();
    std::unique_ptr<MultExpr> parseMultExpr();
    std::unique_ptr<Argument> parseArgument();
    std::unique_ptr<Variable> parseVariable();
    std::unique_ptr<Number> parseNumber();
    std::unique_ptr<FunctionCall> parseFunctionCall();
    std::unique_ptr<MethodCall> parseMethodCall();
    std::unique_ptr<String> parseString();
public:
    Parser(Source &src, Scaner &scan): scan(scan), src(src) {}
    std::unique_ptr<Program> parseProgram();
};


#endif //PROJEKT_PARSER_H
