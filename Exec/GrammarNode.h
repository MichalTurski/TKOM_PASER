//
// Created by michal on 08.11.18.
//

#ifndef PROJEKT_GRAMMARNODE_H
#define PROJEKT_GRAMMARNODE_H

#include <string>
#include <bits/unique_ptr.h>
#include <list>

#include "../Lexer/Scaner.h"
#include "ExecutionState.h"
#include "../LibraryInterface/Function.h"

class Node {
private:
    TextPos textPos;
protected:
//    void addVariableToState(Symbols &symbols, ExecutionState &state, const std::string &type, const std::string &name,
//                                  const std::string &reference);
public:
    Node (const TextPos &textPos): textPos(textPos) {}
    void error(const char* msg);
    const TextPos &getTextPos() const;
    virtual void printValue(int setw) const = 0;
};

class InstructionSet;
class LogicExpr;

class ExprArgument: public virtual Node {
public:
    explicit ExprArgument(const TextPos &textPos): Node(textPos) {}
    virtual void printValue(int setw) const = 0;
    virtual Object* evaluate(ExecutionState &state) = 0;
};

class Statement: public virtual Node {
public:
    explicit Statement(const TextPos &textPos): Node(textPos) {}
    virtual void execute(ExecutionState &state) = 0;
};

class Instruction: public Statement {
public:
    explicit Instruction(const TextPos &textPos): Node(textPos), Statement(textPos) {}
    virtual void execute(ExecutionState &state) = 0;
};

class Variable: public ExprArgument {
private:
    std::string name;
public:
    Variable(const TextPos &textPos, std::string &&name);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state) override;
    const std::string &getName() const;
};

class ConstNum: public ExprArgument {
private:
    int value;
public:
    ConstNum(const TextPos &textPos, int val);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state) override;
};

class ConstString: public ExprArgument {
private:
    std::string value;
public:
    ConstString(const TextPos &textPos, std::string &&value);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state) override;
};

class FunctionCall: public Instruction, public ExprArgument {
private:
    std::string name;
    std::list<std::unique_ptr<Variable>> arguments;
public:
    FunctionCall(const TextPos &textPos, std::string &&name, std::list<std::unique_ptr<Variable>> &&arguments);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state) override;
    void execute(ExecutionState &state) override;
};

class MethodCall: public Instruction, public ExprArgument {
private:
    std::string objectName;
    std::string method;
    std::list<std::unique_ptr<Variable>> arguments;
public:
    MethodCall(const TextPos &textPos, std::string &&object, std::string &&method,
               std::list<std::unique_ptr<Variable>> &&args);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state) override;
    void execute(ExecutionState &state) override;
};

class MultExpr: public Node {
private:
    std::list<std::unique_ptr<ExprArgument>> exprList;
    std::list<TokenType> operators;
public:
    MultExpr(const TextPos &textPos, std::list<std::unique_ptr<ExprArgument>> &&exprList,
             std::list<TokenType> &&operators);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state);
};

class AddExpr: public Node {
private:
    std::list<std::unique_ptr<MultExpr>> exprList;
    std::list<TokenType> operators;
public:
    AddExpr(const TextPos &textPos, std::list<std::unique_ptr<MultExpr>> &&exprList,
            std::list<TokenType> &&operators);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state);
};

class CmpExpr: public Node {
private:
    std::list<std::unique_ptr<AddExpr>> exprList;
    std::list<TokenType> operators;
public:
    CmpExpr(const TextPos &textPos, std::list<std::unique_ptr<AddExpr>> &&exprList,
            std::list<TokenType> &&operators);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state);
};

class LogicExpr: public ExprArgument {
private:
    bool negated;
    std::list<std::unique_ptr<CmpExpr>> exprList;
    std::list<TokenType> operators;
public:
    LogicExpr(const TextPos &textPos, bool negated, std::list<std::unique_ptr<CmpExpr>> &&exprList,
              std::list<TokenType> &&operators);
    void printValue(int setw) const override;
    void negate();
    Object *evaluate(ExecutionState &state) override;
};

class FunctionRef: public ExprArgument {
private:
    std::string name;
public:
    FunctionRef(const TextPos &textPos, std::string &&name);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state) override;
};

class MethodRef: public ExprArgument {
private:
    std::string object;
    std::string method;
public:
    MethodRef(const TextPos &textPos, std::string &&group, std::string &&method);
    void printValue(int setw) const override;
    Object *evaluate(ExecutionState &state) override;
//    const std::string &getGroup() const;
};

class Assignment: public Instruction {
private:
    std::string lVal;
    std::unique_ptr<LogicExpr> rVal;
public:
    Assignment(const TextPos &textPos, std::string &&lVal, std::unique_ptr<LogicExpr> &&rVal);
    void printValue(int setw) const override;
    void execute(ExecutionState &state) override;
};

class VariableDefinition: public Instruction {
private:
    std::string type;
    std::string name;
    std::string reference;
public:
    VariableDefinition(const TextPos &textPos, std::string &&type, std::string &&name, std::string &&reference);
    void printValue(int setw) const override;
    void execute(ExecutionState &state) override;
    const std::string &getType() const;
    const std::string &getName() const;
};

class IfStatement: public Statement {
private:
    std::unique_ptr<LogicExpr> condition;
    std::unique_ptr<InstructionSet> body;
public:
    IfStatement(const TextPos &textPos, std::unique_ptr<LogicExpr> &&condition, std::unique_ptr<InstructionSet> &&body);
    void printValue(int setw) const override;
    void execute(ExecutionState &state) override;
};

class ForStatement: public Statement {
private:
    std::string iteratorName;
    std::string variable;
    std::unique_ptr<InstructionSet> body;
public:
    ForStatement(const TextPos &textPos, std::string &&iterName, std::string &&variable,
                 std::unique_ptr<InstructionSet> &&body);
    void printValue(int setw) const override;
    void execute(ExecutionState &state) override;
};

class ReturnStatement: public Statement {
    std::unique_ptr<LogicExpr> expr;
public:
    ReturnStatement(const TextPos &textPos, std::unique_ptr<LogicExpr> value);
    void printValue(int setw) const override;
    void execute(ExecutionState &state) override;
};

class InstructionSet:public Node {
private:
    std::list<std::unique_ptr<Statement>> statements;
public:
    InstructionSet(InstructionSet &&set) = default;
    InstructionSet(const TextPos &textPos, std::list<std::unique_ptr<Statement>> &&statements);
    void printValue(int setw) const override;
    void execute(ExecutionState &state);
};

class ArgumentPair: public Node {
private:
    std::string type;
    std::string name;
public:
    ArgumentPair(const TextPos &textPos, std::string &&type, std::string &&name);
    void printValue(int setw) const override;
    const std::string &getType() const;
    const std::string &getName() const;
};

class FunctionDefinition: public Node, public Function {
private:
    std::string name;
    std::list<std::unique_ptr<ArgumentPair>> argumentsList;
    std::unique_ptr<InstructionSet> body;
public:
    FunctionDefinition(const TextPos &textPos, std::string &&name,
                       std::list<std::unique_ptr<ArgumentPair>> &&argumentsList,
                       std::unique_ptr<InstructionSet> &&body);
    std::string &getName();
    Object* evaluate(Objects &arguments) override ;
    Object* evaluate(Objects &arguments, ExecutionState &state);
    void printValue(int setw) const override;
};

class Group;

class GroupDefinition: public Node {
friend class Group;
private:
    std::string name;
    std::list<std::unique_ptr<VariableDefinition>> fieldsList;
    std::list<std::unique_ptr<FunctionDefinition>> methodsList;
//    Variables groupVariables;
public:
    GroupDefinition(const TextPos &textPos, std::string &&name, std::list<std::unique_ptr<VariableDefinition>> &&fieldsList,
                    std::list<std::unique_ptr<FunctionDefinition>> &&methodsList);
    void printValue(int setw) const override;
};

class Program: public Node {
    std::unique_ptr<InstructionSet> instructionSet;
    std::list<std::unique_ptr<FunctionDefinition>> functions;
    std::list<std::unique_ptr<GroupDefinition>> groups;
    std::list<void*> libraries;

    void loadLibrary(const std::string &name);
public:
    Program(const TextPos &textPos, std::list<std::unique_ptr<FunctionDefinition>> &&functions,
            std::list<std::unique_ptr<GroupDefinition>> &&groups,
            std::unique_ptr<InstructionSet> &&instructionSet);
    ~Program();
    void printValue(int setw) const override;
    int execute(const std::list<std::string> &libNames);
};

#endif //PROJEKT_GRAMMARNODE_H
