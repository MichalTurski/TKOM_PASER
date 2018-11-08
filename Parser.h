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
#include "GrammarNode.h"

class Parser {
    Scaner &scan;
    Source &src;

    std::unique_ptr<std::string> errString(const TextPos &pos, const std::string &exp, const std::string &got);
    std::unique_ptr<InstructionSet> parseInstructionSet();
    std::unique_ptr<FunctionDefinition> parseFunctionDefinition();
    std::unique_ptr<GroupDefinition> parseGroupDefinition();
    std::unique_ptr<std::list<std::unique_ptr<ArgumentPair>>> parseArgumentsList();
    std::unique_ptr<ArgumentPair> parseArgumentPair();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::unique_ptr<ForStatement> parseForStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<Instruction> parseInstruction();
    std::unique_ptr<Assignment> parseAssignment(std::string &name);
    std::unique_ptr<VariableDefinition> parseVariableDefinition();
    std::unique_ptr<VariableDefinition> parseVariableDefinition(std::string &name);
    std::unique_ptr<LogicExpr> parseLogicExpr();
    std::unique_ptr<LogicExpr> parseLogicExpr(std::string &name);
    std::unique_ptr<LogicExpr> parseLogicExprCmn(std::unique_ptr<CmpExpr> cmpExpr);
    std::unique_ptr<CmpExpr> parseCmpExpr();
    std::unique_ptr<CmpExpr> parseCmpExpr(std::string &name);
    std::unique_ptr<CmpExpr> parseCmpExprCmn(std::unique_ptr<AddExpr> addExpr);
    std::unique_ptr<AddExpr> parseAddExpr();
    std::unique_ptr<AddExpr> parseAddExpr(std::string &name);
    std::unique_ptr<AddExpr> parseAddExprCmn(std::unique_ptr<MultExpr> multExpr);
    std::unique_ptr<MultExpr> parseMultExpr();
    std::unique_ptr<MultExpr> parseMultExpr(std::string &name);
    std::unique_ptr<MultExpr> parseMultExprCmn(std::unique_ptr<ExprArgument> argument);
    std::unique_ptr<ExprArgument> parseExprArgument();
    std::unique_ptr<ExprArgument> parseExprArgument(std::string &name);
    std::unique_ptr<Variable> parseVariable(std::string &name);
    std::unique_ptr<Number> parseNumber();
    std::unique_ptr<FunctionCall> parseFunctionCall(std::string &name);
    std::unique_ptr<MethodCall> parseMethodCall(std::string &name);
    std::unique_ptr<std::list<std::unique_ptr<FunArgument>>> parseCallArguments();
    std::unique_ptr<FunArgument> parseFunArgument();
    std::unique_ptr<FunctionRef> parseFunReference(std::string &name);
    std::unique_ptr<MethodRef> parseMethodReference(std::string &name);
    std::unique_ptr<String> parseString();
public:
    Parser(Source &src, Scaner &scan): scan(scan), src(src) {}
    std::unique_ptr<Program> parseProgram();
};


#endif //PROJEKT_PARSER_H
