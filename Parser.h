//
// Created by michal on 20.04.18.
//

#ifndef PROJEKT_PARSER_H
#define PROJEKT_PARSER_H

#include <list>
#include <bits/shared_ptr.h>
#include <memory>

#include "Source.h"
#include "Scaner.h"

class ParserNode {
public:
    virtual void printValue(int setw) const = 0;
};

class InstructionSet;
class LogicExpr;

class Argument: public ParserNode {
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
    std::list<std::shared_ptr<LogicExpr>> arguments;
public:
    FunctionCall(std::string &&name, std::list<std::shared_ptr<LogicExpr>> &&arguments): name(name),
                                                                                         arguments(arguments) {}
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
    std::shared_ptr<FunctionCall> function;
public:
    MethodCall(std::string &&name, std::shared_ptr<FunctionCall> &&function): name(name), function(function) {}
    void printValue(int setw) const override;
};

class MultExpr: public ParserNode {
private:
    std::list<std::shared_ptr<Argument>> exprList;
    std::list<TokenType> operators;
public:
    MultExpr(std::list<std::shared_ptr<Argument>> &&exprList,
                std::list<TokenType> &&operators): exprList(exprList),
                                                   operators(operators){}
    void printValue(int setw) const override;
};

class AddExpr: public ParserNode {
private:
    std::list<std::shared_ptr<MultExpr>> exprList;
    std::list<TokenType> operators;
public:
    AddExpr(std::list<std::shared_ptr<MultExpr>> &&exprList,
              std::list<TokenType> &&operators): exprList(exprList),
                                                 operators(operators){}
    void printValue(int setw) const override;
};

class CmpExpr: public ParserNode {
private:
    std::list<std::shared_ptr<AddExpr>> exprList;
    std::list<TokenType> operators;
public:
    CmpExpr(std::list<std::shared_ptr<AddExpr>> &&exprList,
              std::list<TokenType> &&operators): exprList(exprList),
                                                 operators(operators){}
    void printValue(int setw) const override;
};

class Statement: public  ParserNode {};

class Instruction: public Statement {};

class LogicExpr: public Instruction {
private:
    bool negated;
    std::list<std::shared_ptr<CmpExpr>> exprList;
    std::list<TokenType> operators;
public:
    LogicExpr(bool negated, std::list<std::shared_ptr<CmpExpr>> &&exprList,
        std::list<TokenType> &&operators): negated(negated),
                                           exprList(exprList),
                                           operators(operators){}
    void printValue(int setw) const override;
};

class Assignment: public Instruction {
private:
    std::string lVal;
    std::shared_ptr<LogicExpr> rVal;
public:
    Assignment(std::string &&lVal, std::shared_ptr<LogicExpr> &&rVal): lVal(lVal), rVal(rVal) {}
    void printValue(int setw) const override;
};

class VariableDefinition: public Instruction {
private:
    std::string type;
    std::string name;
    std::string reference;
public:
    VariableDefinition(std::string &&type, std::string &&name, std::string &&reference): type(type),
                                                                                         name(name),
                                                                                         reference(reference){}
    void printValue(int setw) const override;
};

class IfStatement: public Statement {
private:
    std::shared_ptr<LogicExpr> condition;
    std::shared_ptr<InstructionSet> body;
public:
    IfStatement(std::shared_ptr<LogicExpr> &&condition,
                std::shared_ptr<InstructionSet> &&body): condition(condition),
                                                    body(body) {}
    void printValue(int setw) const override;
};

class ForStatement: public Statement {
private:
    std::string iteratorName;
    std::string variable;
    std::shared_ptr<InstructionSet> body;
public:
    ForStatement(std::string &&iterName, std::string &&variable,
            std::shared_ptr<InstructionSet> &&body): iteratorName(iterName),
                                                     variable(variable),
                                                     body(body) {}
    void printValue(int setw) const override;

};

class ReturnStatement: public Statement {
    std::shared_ptr<LogicExpr> value;
public:
    ReturnStatement(std::shared_ptr<LogicExpr> value): value(value) {}
    void printValue(int setw) const override;
};

class InstructionSet:public ParserNode {
private:
    std::list<std::shared_ptr<Statement>> statements;
public:
    InstructionSet(std::list<std::shared_ptr<Statement>> &&statements): statements(statements){}
    void printValue(int setw) const override;
};

class ArgumentPair: public ParserNode {
private:
    std::string type;
    std::string name;
public:
    ArgumentPair(std::string &&type, std::string &&name): type(type), name(name) {}
    void printValue(int setw) const override;
};

class FunctionDefinition: public ParserNode {
private:
    std::string name;
    std::list<std::shared_ptr<ArgumentPair>> argumentsList;
    std::shared_ptr<InstructionSet> body;
public:
    FunctionDefinition(std::string &&name, std::list<std::shared_ptr<ArgumentPair>> &&argumentsList,
        std::shared_ptr<InstructionSet> &&body): name(name), argumentsList(argumentsList),
                                               body(body) {}
    void printValue(int setw) const override;
};

class Program: public ParserNode {
    std::shared_ptr<InstructionSet> instructionSet;
    std::list<std::shared_ptr<FunctionDefinition>> Functions;
public:
    Program(std::list<std::shared_ptr<FunctionDefinition>> &&Functions,
            std::shared_ptr<InstructionSet> &&instructionSet):
            Functions(Functions), instructionSet(instructionSet) {};
    void printValue(int setw) const override;
//    void parse(Source &src, Scaner &scan);1
};

class Parser {
    Scaner &scan;
    Source &src;
    Token getToken(std::list<Token> &unused);
    Token nextToken(std::list<Token> &unused);

    void printCantParse(const TextPos &pos, const std::string &exp, const std::string &got);
    std::shared_ptr<InstructionSet> parseInstructionSet(std::list<Token> &unused);
    std::shared_ptr<FunctionDefinition> parseFunctionDefinition(std::list<Token> &unused);
    std::shared_ptr<ArgumentPair> parseArgumentPair(std::list<Token> &unused);
    std::shared_ptr<IfStatement> parseIfStatement(std::list<Token> &unused);
    std::shared_ptr<ForStatement> parseForStatement(std::list<Token> &unused);
    std::shared_ptr<ReturnStatement> parseReturnStatement(std::list<Token> &unused);
    std::shared_ptr<Instruction> parseInstruction(std::list<Token> &unused);
    std::shared_ptr<Assignment> parseAssignment(std::list<Token> &unused);
    std::shared_ptr<VariableDefinition> parseVariableDefinition(std::list<Token> &unused);
    std::shared_ptr<LogicExpr> parseLogicExpr(std::list<Token> &unused);
    std::shared_ptr<CmpExpr> parseCmpExpr(std::list<Token> &unused);
    std::shared_ptr<AddExpr> parseAddExpr(std::list<Token> &unused);
    std::shared_ptr<MultExpr> parseMultExpr(std::list<Token> &unused);
    std::shared_ptr<Argument> parseArgument(std::list<Token> &unused);
    std::shared_ptr<Variable> parseVariable(std::list<Token> &unused);
    std::shared_ptr<Number> parseNumber(std::list<Token> &unused);
    std::shared_ptr<FunctionCall> parseFunctionCall(std::list<Token> &unused);
    std::shared_ptr<MethodCall> parseMethodCall(std::list<Token> &unused);
    std::shared_ptr<String> parseString(std::list<Token> &unused);
public:
    Parser(Source &src, Scaner &scan): scan(scan), src(src) {}
    std::shared_ptr<Program> parseProgram();
};


#endif //PROJEKT_PARSER_H
