//
// Created by michal on 20.04.18.
//

#include <iostream>
#include <memory>
#include <iomanip>

#include "Parser.h"

std::unique_ptr<std::string> Parser::errString(const TextPos &pos, const std::string &exp, const std::string &got) {
    std::ostringstream oss;
    std::unique_ptr<std::string> msg;
     oss << "Couldn't parse code in line " << pos.line << ", column "
                                                          << pos.num + 1 << std::endl;
    oss << "Expected " << exp << ", got \"" << got << "\"." << std::endl;
    msg = std::make_unique<std::string>(oss.str());
    return msg;
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::list<std::unique_ptr<FunctionDefinition>> functionDefinitions;
    std::unique_ptr<FunctionDefinition> funDef;
    std::unique_ptr<InstructionSet> instructionSet;
    std::list<std::unique_ptr<GroupDefinition>> groupsDefinitions;
    std::unique_ptr<GroupDefinition> groupDef;
    while (funDef = std::move(parseFunctionDefinition())) {
        functionDefinitions.emplace_back(std::move(funDef));
    }
    while (groupDef = parseGroupDefinition()) {
        groupsDefinitions.emplace_back(std::move(groupDef));
    }
    if (!(instructionSet = parseInstructionSet())) {
        return (nullptr);
    } else {
        return (std::make_unique<Program>(std::move(functionDefinitions), std::move(groupsDefinitions),
                                          std::move(instructionSet)));
    }
}

std::unique_ptr<FunctionDefinition> Parser::parseFunctionDefinition() {
    std::string name;
    std::unique_ptr<std::list<std::unique_ptr<ArgumentPair>>> argumentsList;
    std::unique_ptr<InstructionSet> instructionSet;
    Token *token = scan.getCurr();
    if (token->type != functionKw){
        return nullptr;
    }
    token = scan.next();
    if (token->type == identifier){
        name = std::move(token->string);
        scan.next();
        if (argumentsList = std::move(parseArgumentsList())) {
            token = scan.getCurr();
            if (token->type == lBrace) {
                scan.next();
                if (instructionSet = std::move(parseInstructionSet())) {
                    token = scan.getCurr();
                    if (token->type == rBrace) {
                        scan.next();
                        return std::make_unique<FunctionDefinition>(std::move(name), std::move(*argumentsList),
                                                                    std::move(instructionSet));
                    } else {
                        throw std::runtime_error(
                                *errString(token->begin, "\"}\"", token->string));
                    }
                }
            } else {
                throw std::runtime_error(
                        *errString(token->begin, "\"{\"", token->string));
            }
        } else {
            throw std::runtime_error(
                    *errString(token->begin, "arguments list", token->string));
        }
    } else {
        throw std::runtime_error(
                *errString(token->begin, "identifier", token->string));
    }
    return nullptr;
}

std::unique_ptr<GroupDefinition> Parser::parseGroupDefinition() {
    Token *token;
    std::string name;
    std::unique_ptr<VariableDefinition> varDef;
    std::list<std::unique_ptr<VariableDefinition>> varDefinitions;
    std::unique_ptr<FunctionDefinition> methodDef;
    std::list<std::unique_ptr<FunctionDefinition>> methodDefinitions;

    token = scan.getCurr();
    if (token->type == groupKw) {
        token = scan.next();
        if (token->type == identifier){
            name = std::move(token->string);
            token = scan.next();
            if (token->type == lBrace) {
                token = scan.next();
                while (varDef = parseVariableDefinition()) {
                    varDefinitions.emplace_back(std::move(varDef));
                    token = scan.getCurr();
                    if (token->type != colon) {
                        throw std::runtime_error(*errString(token->begin, ";", token->string));
                    }
                    scan.next();
                }
                while (methodDef = parseFunctionDefinition()) {
                    methodDefinitions.emplace_back(std::move(methodDef));
                }
                token = scan.getCurr();
                if (token->type == rBrace) {
                    scan.next();
                    return std::make_unique<GroupDefinition>(std::move(name), std::move(varDefinitions),
                            std::move(methodDefinitions));
                }
                throw std::runtime_error(*errString(token->begin, "}", token->string));
            }
            throw std::runtime_error(*errString(token->begin, "{", token->string));
        }
        throw std::runtime_error(*errString(token->begin, "group name", token->string));
    }
    return nullptr;
}

std::unique_ptr<std::list<std::unique_ptr<ArgumentPair>>> Parser::parseArgumentsList() {
    std::unique_ptr<std::list<std::unique_ptr<ArgumentPair>>>
            argumentsList(new (std::list<std::unique_ptr<ArgumentPair>>));
    std::unique_ptr<ArgumentPair> currPair;
    Token *token = scan.getCurr();
    if (token->type == lPar) {
        token = scan.next();
        if (token->type != rPar) {
            if (currPair = parseArgumentPair()) {
                argumentsList->push_back(std::move(currPair));
                token = scan.getCurr();
                while (token->type != rPar) {
                    if (token->type == comma) {
                        scan.next();
                        if (currPair = parseArgumentPair()) {
                            argumentsList->push_back(std::move(currPair));
                            token = scan.getCurr();
                        } else {
                            throw std::runtime_error(
                                    *errString(token->begin, "argument pair", token->string));
                        }
                    } else {
                        throw std::runtime_error(
                                *errString(token->begin, "comma or )", token->string));
                    }
                }
            } else {
                throw std::runtime_error(
                        *errString(token->begin, "argument pair", token->string));
            }
        }
        scan.next();
        return argumentsList;
    } else {
        return nullptr;
    }

}

std::unique_ptr<ArgumentPair> Parser::parseArgumentPair() {
    std::string type, name;
    std::unique_ptr<ArgumentPair> pair = nullptr;
    Token *token = scan.getCurr();
    if (token->type == identifier) {
        type = std::move(token->string);
        token = scan.next();
        if (token->type == identifier) {
            name = std::move(token->string);
            scan.next();
            pair = std::make_unique<ArgumentPair> (std::move(type), std::move(name));
        } else {
            throw std::runtime_error(
                    *errString(token->begin, "variable name", token->string));
        }
    } else {
        throw std::runtime_error(
                *errString(token->begin, "variable type", token->string));
    }
    return pair;
}

std::unique_ptr<InstructionSet> Parser::parseInstructionSet() {
    std::unique_ptr<Statement> currStatement;
    std::list<std::unique_ptr<Statement>> statements;
    Token *token = scan.getCurr();
    do {
        switch (token->type) {
            case ifKw:
                currStatement = parseIfStatement();
                break;
            case forKw:
                currStatement = parseForStatement();
                break;
            case returnKw:
                currStatement = parseReturnStatement();
                break;
            case identifier:
            case negOp:
                currStatement = parseInstruction();
                break;
            default:
                throw std::runtime_error(
                        *errString(token->begin, "identifier, for, if or return", token->string));
        }
        statements.push_back(std::move(currStatement));
        token = scan.getCurr();
    } while (!(token->type == rBrace || token->type == eof));
    return std::make_unique<InstructionSet> (std::move(statements));
}

std::unique_ptr<IfStatement> Parser::parseIfStatement() {
    std::unique_ptr<InstructionSet> body;
    std::unique_ptr<LogicExpr> cond;

    Token *token = scan.getCurr();
    if (token->type == ifKw) {
        token = scan.next();
        if (token->type == lPar) {
            scan.next();
            if (cond = std::move(parseLogicExpr())) {
                token = scan.getCurr();
                if (token->type == rPar) {
                    token = scan.next();
                    if (token->type == lBrace) {
                        scan.next();
                        body = std::move(parseInstructionSet());
                        token = scan.getCurr();
                        if (token->type == rBrace) {
                            scan.next();
                            return std::make_unique<IfStatement>
                                    (std::move(cond), std::move(body));
                        } else {
                            throw std::runtime_error(
                                    *errString(token->begin, "}", token->string));
                        }
                    } else {
                        throw std::runtime_error(
                                *errString(token->begin, "{", token->string));
                    }
                } else {
                    throw std::runtime_error(
                            *errString(token->begin, ")", token->string));
                }
            } else {
                throw std::runtime_error(
                        *errString(token->begin, "logic expression", token->string));
            }
        } else {
            throw std::runtime_error(
                    *errString(token->begin, "(", token->string));
        }
    } else {
        throw std::runtime_error(
                *errString(token->begin, "if", token->string));
    }
}

std::unique_ptr<ForStatement> Parser::parseForStatement() {
    std::unique_ptr<InstructionSet> body;
    std::string iterator, variable;
    Token *token = scan.getCurr();
    if (token->type == forKw) {
        token = scan.next();
        if (token->type == eachKw) {
            token = scan.next();
            if (token->type == identifier) {
                iterator = std::move(token->string);
                token = scan.next();
                if (token->type == inKw) {
                    token = scan.next();
                    if (token->type == identifier){
                        variable = std::move(token->string);
                        token = scan.next();
                        if (token->type == lBrace) {
                            scan.next();
                            body = std::move(parseInstructionSet());
                            token = scan.getCurr();
                            if (token->type == rBrace) {
                                scan.next();
                                return  std::make_unique<ForStatement>
                                        (std::move(iterator), std::move(variable),
                                         std::move(body));
                            } else {
                                throw std::runtime_error(
                                        *errString(token->begin, "}", token->string));
                            }
                        } else {
                            throw std::runtime_error(
                                    *errString(token->begin, "{", token->string));
                        }
                    } else {
                        throw std::runtime_error(
                                *errString(token->begin, "variable name", token->string));
                    }
                } else {
                    throw std::runtime_error(
                            *errString(token->begin, "in", token->string));
                }
            } else {
                throw std::runtime_error(
                        *errString(token->begin, "iterator name", token->string));
            }
        } else {
            throw std::runtime_error(
                    *errString(token->begin, "each", token->string));
        }
    } else {
        throw std::runtime_error(
                *errString(token->begin, "for", token->string));
    }
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
    std::unique_ptr<LogicExpr> value;
    Token *token = scan.getCurr();
    if (token->type == returnKw) {
        token = scan.next();
        if (token->type == lPar) {
            scan.next();
            value = std::move(parseLogicExpr());
            token = scan.getCurr();
            if (token->type == rPar) {
                token = scan.next();
                if (token->type == colon) {
                    scan.next();
                    return std::make_unique<ReturnStatement> (std::move(value));
                } else {
                    throw std::runtime_error(
                            *errString(token->begin, ";", token->string));
                }
            } else {
                throw std::runtime_error(
                        *errString(token->begin, ")", token->string));
            }
        } else {
            throw std::runtime_error(
                    *errString(token->begin, "(", token->string));
        }
    } else {
        throw std::runtime_error(
                *errString(token->begin, "return", token->string));
    }
}

std::unique_ptr<Instruction> Parser::parseInstruction() {
    Token *token;
    std::string name;
    std::unique_ptr<Instruction> instruction;

    token = scan.getCurr();
    if (token->type ==  identifier) {
        name = std::move(token->string);
        scan.next();
        if (!(instruction = std::move(parseVariableDefinition(name)))) {
            if (!(instruction = std::move(parseAssignment(name)))) {
                if (!(instruction = std::move(parseFunctionCall(name)))) {
                    if (!(instruction = std::move(parseMethodCall(name)))) {
                        token = scan.getCurr();
                        throw std::runtime_error
                                (*errString(token->begin, "operator, ( or identifier", token->string));
                    }
                }
            }
        }
        token = scan.getCurr();
        if (token->type != colon) {
            throw std::runtime_error(*errString(token->begin, ";", token->string));
        }
        scan.next();
        return instruction;
    }
    return nullptr;
}

std::unique_ptr<Assignment> Parser::parseAssignment(std::string &name) {
    std::string lVal;
    std::unique_ptr<LogicExpr> rVal;
    Token *token;
    token = scan.getCurr();
    if (token->type == assignOp) {
        lVal = std::move(name);
        token = scan.next();
        if (rVal = std::move(parseLogicExpr())) {
            return std::make_unique<Assignment>(std::move(lVal), std::move(rVal));
        } else {
            throw std::runtime_error(*errString(token->begin, "logic expression", token->string));
        }
    }
    return nullptr;
}

std::unique_ptr<VariableDefinition> Parser::parseVariableDefinition() {
    std::string type, varName, reference;
    Token *token;
    token = scan.getCurr();
    if (token->type == identifier) {
        type = std::move(token->string);
        token = scan.next();
        if (token->type == identifier) {
            varName = std::move(token->string);
            token = scan.next();
            if (token->type == string) {
                reference = std::move(token->string);
                scan.next();
            }
            return std::make_unique<VariableDefinition>
                    (std::move(type), std::move(varName), std::move(reference));
        }
        throw std::runtime_error(*errString(token->begin, "variable name", token->string));
    }
    return nullptr;
}

std::unique_ptr<VariableDefinition> Parser::parseVariableDefinition(std::string &name) {
    std::string varName, reference;
    Token *token;
    token = scan.getCurr();
    if (token->type == identifier) {
        varName = std::move(token->string);
        token = scan.next();
        if (token->type == string) {
            reference = std::move(token->string);
            scan.next();
        }
        return std::make_unique<VariableDefinition>
                (std::move(name), std::move(varName), std::move(reference));
    }
    return nullptr;
}

std::unique_ptr<LogicExpr> Parser::parseLogicExpr() {
    std::string name;
    bool negated;
    Token *token = scan.getCurr();
    std::unique_ptr<LogicExpr> logicExpr;
    std::unique_ptr<CmpExpr> cmpExpr;
    if (token->type == negOp) {
        negated = true;
        scan.next();
    }
    if (cmpExpr = parseCmpExpr()) {
        logicExpr = parseLogicExprCmn(std::move(cmpExpr));
        if (negated) {
            logicExpr->negate();
        }
        return logicExpr;
    }
    return nullptr;
}

std::unique_ptr<LogicExpr> Parser::parseLogicExpr(std::string &name) {
    std::unique_ptr<CmpExpr> cmpExpr;
    Token *token;
    if (cmpExpr = parseCmpExpr(name)) {
        return parseLogicExprCmn(std::move(cmpExpr));
    }
    return nullptr;
}
std::unique_ptr<LogicExpr> Parser::parseLogicExprCmn(std::unique_ptr<CmpExpr> cmpExpr) {
    std::list<std::unique_ptr<CmpExpr>> exprList;
    std::list<TokenType> operators;
    Token *token;
    exprList.push_back(std::move(cmpExpr));
    token = scan.getCurr();
    while (token->type == orOp || token->type == andOp) {
        operators.push_back(token->type);
        scan.next();
        if (!(cmpExpr = std::move(parseCmpExpr()))) {
            token = scan.getCurr();
            throw std::runtime_error(*errString(token->begin, "comparison expression", token->string));
        }
        exprList.push_back(std::move(cmpExpr));
        token = scan.getCurr();
    }
    return std::make_unique<LogicExpr>(false, std::move(exprList), std::move(operators));

}

std::unique_ptr<CmpExpr> Parser::parseCmpExpr() {
    std::unique_ptr<AddExpr> addExpr;
    if (addExpr = parseAddExpr()) {
        return parseCmpExprCmn(std::move(addExpr));
    }
    return nullptr;
}

std::unique_ptr<CmpExpr> Parser::parseCmpExpr(std::string &name) {
    std::unique_ptr<AddExpr> addExpr;
    if (addExpr = parseAddExpr(name)) {
        return parseCmpExprCmn(std::move(addExpr));
    }
    return nullptr;
}

std::unique_ptr<CmpExpr> Parser::parseCmpExprCmn(std::unique_ptr<AddExpr> addExpr) {
    std::list<std::unique_ptr<AddExpr>> exprList;
    std::list<TokenType> operators;
    Token *token;

    exprList.push_back(std::move(addExpr));
    token = scan.getCurr();
    while (token->type == moreOp || token->type == lessOp || token->type == eqOp || token->type == negOp ||
           token->type == moreEqOp || token->type == lessEqOp ){
        operators.push_back(token->type);
        scan.next();
        if (!(addExpr = std::move(parseAddExpr()))){
            token = scan.getCurr();
            throw std::runtime_error(*errString(token->begin, "operator", token->string));
        }
        exprList.push_back(std::move(addExpr));
        token = scan.getCurr();
    }
    return std::make_unique<CmpExpr> (std::move(exprList), std::move(operators));
}

std::unique_ptr<AddExpr> Parser::parseAddExpr() {
    std::unique_ptr<MultExpr> multExpr;
    if (multExpr = parseMultExpr()) {
        return parseAddExprCmn(std::move(multExpr));
    }
    return nullptr;
}

std::unique_ptr<AddExpr> Parser::parseAddExpr(std::string &name) {
    std::unique_ptr<MultExpr> multExpr;
    if (multExpr = parseMultExpr(name)) {
        return parseAddExprCmn(std::move(multExpr));
    }
    return nullptr;
}

std::unique_ptr<AddExpr> Parser::parseAddExprCmn(std::unique_ptr<MultExpr> multExpr) {
    std::list<std::unique_ptr<MultExpr>> exprList;
    std::list<TokenType> operators;
    Token *token;

    exprList.push_back(std::move(multExpr));
    token = scan.getCurr();
    while (token->type == addOp || token->type == subOp) {
        operators.push_back(token->type);
        scan.next();
        if (!(multExpr = std::move(parseMultExpr()))){
            token = scan.getCurr();
            throw std::runtime_error(*errString(token->begin, "multiplication expression", token->string));
        }
        exprList.push_back(std::move(multExpr));
        token = scan.getCurr();
    }
    return std::make_unique<AddExpr> (std::move(exprList), std::move(operators));
}

std::unique_ptr<MultExpr> Parser::parseMultExpr() {
    std::unique_ptr<ExprArgument> argument;
    if (argument = parseExprArgument()) {
        return parseMultExprCmn(std::move(argument));
    }
    return nullptr;
}

std::unique_ptr<MultExpr> Parser::parseMultExpr(std::string &name) {
    std::unique_ptr<ExprArgument> argument;
    if (argument = parseExprArgument(name)) {
        return parseMultExprCmn(std::move(argument));
    }
    return nullptr;
}

std::unique_ptr<MultExpr> Parser::parseMultExprCmn(std::unique_ptr<ExprArgument> argument) {
    std::list<std::unique_ptr<ExprArgument>> exprList;
    std::list<TokenType> operators;
    Token *token;

    exprList.push_back(std::move(argument));
    token = scan.getCurr();
    while (token->type == multOp || token->type == divOp) {
        operators.push_back(token->type);
        scan.next();
        if (!(argument = std::move(parseExprArgument()))){
            token = scan.getCurr();
            throw std::runtime_error(*errString(token->begin, "argument", token->string));
        }
        exprList.push_back(std::move(argument));
        token = scan.getCurr();
    }
    return std::make_unique<MultExpr> (std::move(exprList), std::move(operators));
}

std::unique_ptr<ExprArgument> Parser::parseExprArgument() {
    std::unique_ptr<ExprArgument> argument;
    std::string name;
    Token *token = scan.getCurr();
    switch (token->type) {
        case identifier:
            name = std::move(token->string);
            scan.next();
            return parseExprArgument(name);
        case number:
            return parseNumber();
        case string:
            return parseString();
        case lPar:
            scan.next();
            if (argument = std::move(parseLogicExpr())) {
                token = scan.getCurr();
                if (token->type != rPar) {
                    throw std::runtime_error(*errString(token->begin, ")", token->string));
                }
                scan.next();
                return argument;
            } else {
                throw std::runtime_error(*errString(token->begin, "logic expression", token->string));
            }
        case refOp:
            token = scan.next();
            if (token->type == identifier) {
                name = std::move(token->string);
                scan.next();
                if (!(argument = parseMethodReference(name))) {
                    if (!(argument = parseFunReference(name))) {
                        throw std::runtime_error(*errString(token->begin, "reference", token->string));
                    }
                }
                return argument;
            }
            throw std::runtime_error(*errString(token->begin, "identifier", token->string));
        default:
            return nullptr;
    }
}

std::unique_ptr<ExprArgument> Parser::parseExprArgument(std::string &name) {
    Token *token;
    token = scan.getCurr();
    switch (token->type) {
        case lPar:
            return parseFunctionCall(name);
        case dot:
            return parseMethodCall(name);
        default:
            return parseVariable(name);
    }
}

std::unique_ptr<Variable> Parser::parseVariable(std::string &name) {
    return std::make_unique<Variable>(std::move(name));
}

std::unique_ptr<Variable> Parser::parseVariable() {
    std::string name;
    Token *token = scan.getCurr();
    if (token->type == identifier) {
        name = std::move(token->string);
        scan.next();
        return std::make_unique<Variable>(std::move(name));
    }
    return nullptr;
}

std::unique_ptr<ConstNum> Parser::parseNumber() {
    Token *token = scan.getCurr();
    if (token->type == number) {
        scan.next();
        return std::make_unique<ConstNum>(atoi(token->string.c_str()));
    }
    return nullptr;
}

std::unique_ptr<FunctionCall> Parser::parseFunctionCall(std::string &name) {
    std::list<std::unique_ptr<Variable>> argumentsList;
    std::unique_ptr<std::list<std::unique_ptr<Variable>>> argumentsListPtr;
    Token *token;
    token = scan.getCurr();
    if (argumentsListPtr = parseCallArguments()) {
        argumentsList = std::move(*argumentsListPtr);
        return std::make_unique<FunctionCall>(std::move(name), std::move(argumentsList));
    }
    return nullptr;
}

std::unique_ptr<MethodCall> Parser::parseMethodCall(std::string &name) {
    std::unique_ptr<FunctionCall> functionCall;
    std::list<std::unique_ptr<Variable>> argumentsList;
    std::unique_ptr<std::list<std::unique_ptr<Variable>>> argumentsListPtr;
    std::string methodName;
    Token *token;
    token = scan.getCurr();
    if (token->type == dot) {
        token = scan.next();
        if (token->type == identifier){
            methodName = std::move(token->string);
            scan.next();
            if (argumentsListPtr = parseCallArguments()) {
                argumentsList = std::move(*argumentsListPtr);
                return std::make_unique<MethodCall>(std::move(name), std::move(methodName), std::move(argumentsList));
            } else {
                throw std::runtime_error(*errString(token->begin, "arguments list", token->string));
            }

        } else {
            throw std::runtime_error(*errString(token->begin, "method name", token->string));
        }
    }
    return nullptr;
}

std::unique_ptr<std::list<std::unique_ptr<Variable>>> Parser::parseCallArguments() {
    Token *token;
    std::unique_ptr<Variable> argument;
    std::unique_ptr<std::list<std::unique_ptr<Variable>>>
            argumentsList(new(std::list<std::unique_ptr<Variable>>));

    token = scan.getCurr();
    if (token->type == lPar) {
        token = scan.next();
        if (token->type != rPar) {
            if (argument = parseFunArgument()) {
                argumentsList->push_back(std::move(argument));
                token = scan.getCurr();
                while (token->type != rPar) {
                    if (token->type == comma) {
                        scan.next();
                        if (argument = parseFunArgument()) {
                            argumentsList->push_back(std::move(argument));
                            token = scan.getCurr();
                        } else {
                            token = scan.getCurr();
                            throw std::runtime_error(*errString
                                    (token->begin, "logic expression or reference", token->string));
                        }
                    } else {
                        throw std::runtime_error(*errString(token->begin, ",", token->string));
                    }
                }
            } else {
                throw std::runtime_error(*errString(token->begin, "logic expression or reference",
                                                    token->string));
            }
        }
        scan.next();
        return argumentsList;
    }
    return nullptr;
}

std::unique_ptr<Variable> Parser::parseFunArgument() {
    return parseVariable();
    /*
    switch (token->type) {
        case refOp:
            token = scan.next();
            if (token->type == identifier) {
                name = std::move(token->string);
                scan.next();
                if (!(argument = parseMethodReference(name))) {
                    if (!(argument = parseFunReference(name))) {
                        throw std::runtime_error(*errString(token->begin, "reference", token->string));
                    }
                }
                return argument;
            }
            throw std::runtime_error(*errString(token->begin, "identifier", token->string));
        case string:
            return parseString();
        case number:
            return parseNumber();
        default:
            return parseVariable();
    }*/
}

std::unique_ptr<FunctionRef> Parser::parseFunReference(std::string &name) {
    return std::make_unique<FunctionRef>(std::move(name));
}

std::unique_ptr<MethodRef> Parser::parseMethodReference(std::string &name) {
    std::string method;
    Token *token = scan.getCurr();
    if (token->type == dot) {
        token = scan.next();
        if (token->type == identifier) {
            method = std::move(token->string);
            scan.next();
            return std::make_unique<MethodRef>(std::move(name), std::move(method));
        }
        throw std::runtime_error(*errString(token->begin, "identifier", token->string));
    }
    return nullptr;
}

std::unique_ptr<ConstString> Parser::parseString() {
    Token *token = scan.getCurr();
    if (token->type == string) {
        scan.next();
        return std::make_unique<ConstString>(std::move(token->string));
    }
    return nullptr;
}
