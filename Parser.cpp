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
    while (funDef = std::move(parseFunctionDefinition())) {
        functionDefinitions.emplace_back(std::move(funDef));
    }
    if (!(instructionSet = parseInstructionSet())) {
        return (nullptr);
    } else {
        return (std::make_unique<Program>(std::move(functionDefinitions), std::move(instructionSet)));
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
                        return std::make_unique<FunctionDefinition>(std::move(name), std::move(argumentsList),
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
                scan.next();
            } else {
                throw std::runtime_error(
                        *errString(token->begin, "argument pair", token->string));
            }
        }
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
    std::unique_ptr<Instruction> instruction;

    if (!(instruction = std::move(parseVariableDefinition()))) {
        if (!(instruction = std::move(parseAssignment()))) {
            if (!(instruction = std::move(parseLogicExpr()))) {
                token = scan.getCurr();
                throw std::runtime_error
                        (*errString(token->begin, "operator, ( or identifier", token->string));
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

std::unique_ptr<Assignment> Parser::parseAssignment() {
    std::string lVal;
    std::unique_ptr<LogicExpr> rVal;
    Token *token, *next;
    token = scan.getCurr();
    if (token->type == identifier) {
        next = scan.getFuture();
        if (next->type == assignOp) {
            lVal = std::move(token->string);
            scan.next();
            token = scan.next();
            if (rVal = std::move(parseLogicExpr())) {
                return std::make_unique<Assignment>(std::move(lVal), std::move(rVal));
            } else {
                throw std::runtime_error(*errString(token->begin, "logic expression", token->string));
            }
        }
    }
    return nullptr;
}

std::unique_ptr<VariableDefinition> Parser::parseVariableDefinition() {
    std::string type, name, reference;
    Token *token, *next;
    token = scan.getCurr();
    if (token->type == identifier) {
        next = scan.getFuture();
        if (next->type == identifier) {
            type = std::move(token->string);
            name = std::move(next->string);
            scan.next();
            token = scan.next();
            if (token->type == string) {
                reference = std::move(token->string);
                scan.next();
            }
            return std::make_unique<VariableDefinition>
                    (std::move(type), std::move(name), std::move(reference));
        }
    }
    return nullptr;
}

std::unique_ptr<LogicExpr> Parser::parseLogicExpr() {
    bool negated;
    std::unique_ptr<CmpExpr> cmpExpr;
    std::list<std::unique_ptr<CmpExpr>> exprList;
    std::list<TokenType> operators;
    Token *token = scan.getCurr();
    if (token->type == negOp) {
        negated = true;
        token = scan.next();
        if (!(cmpExpr = std::move(parseCmpExpr()))) {
            throw std::runtime_error(*errString(token->begin, "operator", token->string));
        }
    } else {
        negated = false;
        if (!(cmpExpr = parseCmpExpr())) {
            return nullptr;
        }
    }
    exprList.push_back(std::move(cmpExpr));
    token = scan.getCurr();
    while (token->type == orOp || token->type == andOp) {
        operators.push_back(token->type);
        scan.next();
        if (!(cmpExpr = std::move(parseCmpExpr()))) {
            token = scan.getCurr();
            throw std::runtime_error(*errString(token->begin, "operator", token->string));
        }
        exprList.push_back(std::move(cmpExpr));
        token = scan.getCurr();
    }
    return std::make_unique<LogicExpr>(negated, std::move(exprList), std::move(operators));
}

std::unique_ptr<CmpExpr> Parser::parseCmpExpr() {
    std::unique_ptr<AddExpr> addExpr;
    std::list<std::unique_ptr<AddExpr>> exprList;
    std::list<TokenType> operators;
    Token *token;

    if (addExpr = std::move(parseAddExpr())) {
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
    return nullptr;
}

std::unique_ptr<AddExpr> Parser::parseAddExpr() {
    std::unique_ptr<MultExpr> multExpr;
    std::list<std::unique_ptr<MultExpr>> exprList;
    std::list<TokenType> operators;
    Token *token;

    if (multExpr = std::move(parseMultExpr())) {
        exprList.push_back(std::move(multExpr));
        token = scan.getCurr();
        while (token->type == addOp || token->type == subOp) {
            operators.push_back(token->type);
            scan.next();
            if (!(multExpr = std::move(parseMultExpr()))){
                token = scan.getCurr();
                throw std::runtime_error(*errString(token->begin, "operator", token->string));
            }
            exprList.push_back(std::move(multExpr));
            token = scan.getCurr();
        }
        return std::make_unique<AddExpr> (std::move(exprList), std::move(operators));
    }
    return nullptr;
}

std::unique_ptr<MultExpr> Parser::parseMultExpr() {
    std::unique_ptr<Argument> argument;
    std::list<std::unique_ptr<Argument>> exprList;
    std::list<TokenType> operators;
    Token *token;

    if (argument = std::move(parseArgument())) {
        exprList.push_back(std::move(argument));
        token = scan.getCurr();
        while (token->type == multOp || token->type == divOp) {
            operators.push_back(token->type);
            scan.next();
            if (!(argument = std::move(parseArgument()))){
                token = scan.getCurr();
                throw std::runtime_error(*errString(token->begin, "argument", token->string));
            }
            exprList.push_back(std::move(argument));
            token = scan.getCurr();
        }
        return std::make_unique<MultExpr> (std::move(exprList), std::move(operators));
    }
    return nullptr;
}

std::unique_ptr<Argument> Parser::parseArgument() {
    Token *token;
    std::unique_ptr<Argument> argument;
    if (!(argument = std::move(parseFunctionCall()))) {
        if (!(argument = std::move(parseMethodCall()))) {
            if (!(argument = std::move(parseVariable()))) {
                if (!(argument = std::move(parseNumber()))) {
                    if (!(argument = std::move(parseString()))) {
                        token = scan.getCurr();
                        if (token->type == lPar) {
                            token = scan.next();
                            if (argument = std::move(parseLogicExpr())) {
                                if (scan.getCurr()->type != rPar) {
                                    throw std::runtime_error(*errString(token->begin, ")", token->string));
                                }
                                scan.next();
                            } else {
                                throw std::runtime_error(*errString(token->begin, "logic expression", token->string));
                            }
                        } else {
                            return nullptr;
                        }
                    }
                }
            }
        }
    }
    return argument;
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

std::unique_ptr<Number> Parser::parseNumber() {
    Token *token = scan.getCurr();
    if (token->type == number) {
        scan.next();
        return std::make_unique<Number>(std::move(atoi(token->string.c_str())));
    }
    return nullptr;
}

std::unique_ptr<FunctionCall> Parser::parseFunctionCall() {
    std::string name;
    std::unique_ptr<LogicExpr> argument;
    std::list<std::unique_ptr<LogicExpr>> argumentsList;
    Token *token, *next;
    token = scan.getCurr();
    if (token->type == identifier) {
        next = scan.getFuture();
        if (next->type == lPar) {
            name = std::move(token->string);
            scan.next();
            token = scan.next();
            if (token->type != rPar) {
                if (argument = parseLogicExpr()) {
                    argumentsList.push_back(std::move(argument));
                    token = scan.getCurr();
                    while (token->type != rPar) {
                        if (token->type == comma) {
                            scan.next();
                            if (argument = parseLogicExpr()) {
                                argumentsList.push_back(std::move(argument));
                                token = scan.getCurr();
                            } else {
                                token = scan.getCurr();
                                throw std::runtime_error(*errString
                                        (token->begin, "logic expression", token->string));
                            }
                        } else {
                            throw std::runtime_error(*errString(
                                    token->begin, ",", token->string));
                        }
                    }
                    scan.next();
                } else {
                    throw std::runtime_error(*errString(token->begin, "logic expression", token->string));
                }
            } else {
                scan.next();
            }
            return std::make_unique<FunctionCall>(std::move(name), std::move(argumentsList));
        }
    }
    return nullptr;
}

std::unique_ptr<MethodCall> Parser::parseMethodCall() {
    std::string name;
    std::unique_ptr<FunctionCall> functionCall;
    Token *token, *next;
    token = scan.getCurr();
    if (token->type != identifier) {
        next = scan.getFuture();
        if (next->type == dot) {
            name = std::move(token->string);
            scan.next();
            token = scan.next();
            if (functionCall = parseFunctionCall()) {
                return std::make_unique<MethodCall> (std::move(name), std::move(functionCall));
            } else {
                throw std::runtime_error(*errString(token->begin, "function call", token->string));
            }
        }
    }
    return nullptr;
}

std::unique_ptr<String> Parser::parseString() {
    Token *token = scan.getCurr();
    if (token->type == string) {
        scan.next();
        return std::make_unique<String>(std::move(token->string));
    }
    return nullptr;
}

/*
std::unique_ptr<Argument> Parser::parseArgument() {
    std::unique_ptr<Argument> argument;
    Token *token = scan.getCurr();
    switch (token->type) {
        case identifier:
            token = scan.getFuture();
            switch (token->type){
                case lPar:
                    return parseFunctionCall();
                case dot:
                    return parseMethodCall();
                default:
                    return parseVariable();
            }
        case number:
            return parseNumber();
        case string:
            return parseString();
        case lPar:
             scan.next();
             parseLogicExpr();
             argument = std::move(parseLogicExpr());
             token = scan.getCurr();
             if (token->type != rPar) {
                 throw std::runtime_error(*errString(token->begin, ")", token->string));
             }
             scan.next();
            return argument;
        default:
            throw std::runtime_error
                    (*errString(token->begin, "const number, variable, function, string or method call", token->string));
    }
}

std::unique_ptr<Variable> Parser::parseVariable() {
    std::string name;
    Token *token = scan.getCurr();
    if (token->type == identifier) {
        name = std::move(token->string);
        scan.next();
        return std::make_unique<Variable>(std::move(name));
    }
    throw std::runtime_error(*errString(token->begin, "variable name", token->string));
}

std::unique_ptr<Number> Parser::parseNumber() {
    Token *token = scan.getCurr();
    if (token->type == number) {
        scan.next();
        return std::make_unique<Number>(std::move(atoi(token->string.c_str())));
    }
    throw std::runtime_error(*errString(token->begin, "const number", token->string));
}

std::unique_ptr<FunctionCall> Parser::parseFunctionCall() {
    std::string name;
    std::unique_ptr<LogicExpr> argument;
    std::list<std::unique_ptr<LogicExpr>> argumentsList;
    Token *token, *prev;
    prev = scan.getCurr();
    if (prev->type == identifier) {
        token = scan.getFuture();
        if (token->type == lPar) {
            name = std::move(prev->string);
            token = scan.next();
            if (token->type != rPar) {
                scan.next();
                if (argument = parseLogicExpr()) {
                    argumentsList.push_back(std::move(argument));
                    token = scan.getCurr();
                    while (token->type != rPar) {
                        if (token->type == comma) {
                            scan.next();
                            if (argument = parseLogicExpr()) {
                                argumentsList.push_back(std::move(argument));
                                token = scan.getCurr();
                            } else {
                                token = scan.getCurr();
                                throw std::runtime_error(*errString
                                        (token->begin, "logic expression", token->string));
                            }
                        } else {
                            throw std::runtime_error(*errString(
                                    token->begin, ",", token->string));
                        }
                    }
                } else {
                    throw std::runtime_error(*errString(token->begin, "logic expression", token->string));
                }
            } else {
                scan.next();
            }
            scan.next();
            return std::make_unique<FunctionCall>(std::move(name), std::move(argumentsList));
        }
    }
    return nullptr;
}

std::unique_ptr<MethodCall> Parser::parseMethodCall() {
    std::string name;
    std::unique_ptr<FunctionCall> functionCall;
    Token *token, *prev;
    prev = scan.getCurr();
    if (prev->type != identifier) {
        token = scan.getFuture();
        if (token->type == dot) {
            name = std::move(prev->string);
            token = scan.next();
            scan.next();
            if (functionCall = parseFunctionCall()) {
                return std::make_unique<MethodCall> (std::move(name), std::move(functionCall));
            } else {
                throw std::runtime_error(*errString(token->begin, "function call", token->string));
            }
        }
    }
    return nullptr;
}

std::unique_ptr<String> Parser::parseString() {
    Token *token = scan.getCurr();
    if (token->type == string) {
        scan.next();
        return std::make_unique<String>(std::move(token->string));
    }
    return nullptr;
}
 */

void Variable::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Variable" << std::endl;
     std::cout << std::string(setw, ' ') << "value = " << name << std::endl;
}
void Number::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Number" << std::endl;
    std::cout << std::string(setw, ' ') << "value = " << value << std::endl;
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
void String::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = String" << std::endl;
     std::cout << std::string(setw, ' ') << "value = " << value << std::endl;
}
void MethodCall::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = MethodCall" << std::endl;
    std::cout << std::string(setw, ' ') << "base object name = " << name << std::endl;
    std::cout << std::string(setw, ' ') << "method: {" << std::endl;
    function->printValue(setw + 4);
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
void Assignment::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Assigment" << std::endl;
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
    std::cout << std::string(setw, ' ') << "value {" << std::endl;
    value->printValue(setw + 4);
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
    for (auto &&i: *argumentsList) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw, ' ') << "body {" << std::endl;
    body->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}
void Program::printValue(int setw) const {
    std::cout << std::string(setw, ' ') << "Node type = Program" << std::endl;
    std::cout << std::string(setw, ' ') << "functions {" << std::endl;
    for (std::unique_ptr<FunctionDefinition> const &i: Functions) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw + 4, ' ') << "instruction set {" << std::endl;
    instructionSet->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}

