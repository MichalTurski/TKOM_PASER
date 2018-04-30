//
// Created by michal on 20.04.18.
//

#include <iostream>
#include <memory>
#include <iomanip>

#include "Parser.h"

void Parser::printCantParse(const TextPos &pos, const std::string &exp, const std::string &got) {
    std::cerr << "Couldn't parse code in line " << pos.line <<
              ", column " << pos.num + 1 << std::endl;
    std::cerr << "Expected " << exp << ", got \"" << got << "\"." << std::endl;
}

std::shared_ptr<Program> Parser::parseProgram() {
    std::list<Token> unconsumed;
    std::list<std::shared_ptr<FunctionDefinition>> functionDefinitions;
    std::shared_ptr<FunctionDefinition> currFunDef;
    std::shared_ptr<InstructionSet> instructionSet;
    Token token = scan.getToken();
    while (token.type == identifier) {
        unconsumed.push_back(token);
        token = scan.next();
        if (token.type == lPar) {
            unconsumed.push_back(token);
            token = scan.next();
            if (token.type == identifier) {
                unconsumed.push_back(token);
                token = scan.next();
                if (token.type == identifier) {
                    currFunDef = parseFunctionDefinition(unconsumed);
                    if (currFunDef == nullptr){
                        return (nullptr);
                    } else {
                        functionDefinitions.push_back(currFunDef);
                        token = scan.getToken();
                    }
                } else {
                    break;
                }
            } else {
                break;
            }
        } else {
            break;
        }

    }
    instructionSet = parseInstructionSet(unconsumed);
    if (instructionSet == nullptr){
        return (nullptr);
    } else {
       return (std::make_shared<Program>(std::move(functionDefinitions), std::move(instructionSet)));
    }
}

std::shared_ptr<FunctionDefinition> Parser::parseFunctionDefinition(std::list<Token> &unused) {
    std::string name;
    std::list<std::shared_ptr<ArgumentPair>> argumentsList;
    std::shared_ptr<InstructionSet> instructionSet;
    std::shared_ptr<ArgumentPair> currPair;
    Token token = getToken(unused);
    if (token.type == identifier){
        name = std::move(token.string);
    } else {
        printCantParse(token.begin, "identifier", token.string);
    }
    token = nextToken(unused);
    if (token.type != lPar){
        printCantParse(token.begin, "\"(\"", token.string);
    }
    token = nextToken(unused);
    if (token.type != rPar){
        currPair = parseArgumentPair(unused);
        if (currPair == nullptr) {
            return nullptr;
        }
        argumentsList.push_back(std::move(currPair));
        token = getToken(unused);
        while (token.type != rPar){
            if (token.type != comma){
                printCantParse(token.begin, "\",\"", token.string);
                return nullptr;
            }
            token = nextToken(unused);
            currPair = parseArgumentPair(unused);
            if (currPair == nullptr) {
                return nullptr;
            }
            argumentsList.push_back(std::move(currPair));
            token = getToken(unused);
        }
    }
    token = nextToken(unused);
    if (token.type != lBrace){
        printCantParse(token.begin, "\"{\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    instructionSet = parseInstructionSet(unused);
    if (instructionSet == nullptr) {
        return nullptr;
    }
    token = getToken(unused);
    if (token.type != rBrace){
        printCantParse(token.begin, "\"}\"", token.string);
        return nullptr;
    }
    nextToken(unused);
    return std::make_shared<FunctionDefinition> (std::move(name), std::move(argumentsList),
        std::move(instructionSet));
}

std::shared_ptr<ArgumentPair> Parser::parseArgumentPair(std::list<Token> &unused) {
    std::string type, name;
    Token token = getToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "type", token.string);
        return nullptr;
    } else {
        type = std::move(token.string);
    }
    token = nextToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "variable name", token.string);
        return nullptr;
    } else {
        name = std::move(token.string);
    }
    nextToken(unused);
    return std::make_shared<ArgumentPair> (std::move(type), std::move(name));
}

std::shared_ptr<InstructionSet> Parser::parseInstructionSet(std::list<Token> &unused) {
    std::shared_ptr<Statement> currStatement;
    std::list<std::shared_ptr<Statement>> statements;
    Token token = getToken(unused);
    do {
        switch (token.type) {
            case ifKw:
                currStatement = parseIfStatement(unused);
                if (currStatement == nullptr) {
                    return nullptr;
                }
                break;
            case forKw:
                currStatement = parseForStatement(unused);            token = getToken(unused);
                if (currStatement == nullptr) {
                    return nullptr;
                }
                break;
            case returnKw:
                currStatement = parseReturnStatement(unused);
                if (currStatement == nullptr) {
                    return nullptr;
                }
                break;
            case identifier:
            case negOp:
                currStatement = parseInstruction(unused);
                if (currStatement == nullptr) {
                    return nullptr;
                }
                break;
            default:
                printCantParse(token.begin, "identifier, for, if or return", token.string);
                return nullptr;
        }
        statements.push_back(std::move(currStatement));
        token = getToken(unused);
    } while (!(token.type == rBrace || token.type == eof));
    return std::make_shared<InstructionSet> (std::move(statements));
}

std::shared_ptr<IfStatement> Parser::parseIfStatement(std::list<Token> &unused) {
    std::shared_ptr<InstructionSet> body;
    std::shared_ptr<LogicExpr> cond;
    Token token = getToken(unused);
    if (token.type != ifKw) {
        printCantParse(token.begin, "\"if\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    if (token.type != lPar) {
        printCantParse(token.begin, "\"(\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    cond = parseLogicExpr(unused);
    if (cond == nullptr){
        return nullptr;
    }
    token = getToken(unused);
    if (token.type != rPar) {
        printCantParse(token.begin, "\")\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    if (token.type != lBrace) {
        printCantParse(token.begin, "\"{\"", token.string);
        return nullptr;
    }
    nextToken(unused);
    body = parseInstructionSet(unused);
    if (body == nullptr) {
        return nullptr;
    }
    token = getToken(unused);
    if (token.type != rBrace) {
        printCantParse(token.begin, "\"}\"", token.string);
        return nullptr;
    }
    nextToken(unused);
    return std::make_shared<IfStatement> (std::move(cond), std::move(body));
}

std::shared_ptr<ForStatement> Parser::parseForStatement(std::list<Token> &unused) {
    std::shared_ptr<InstructionSet> body;
    Token token = getToken(unused);
    if (token.type != forKw) {
        printCantParse(token.begin, "\"for\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    if (token.type != eachKw) {
        printCantParse(token.begin, "\"each\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "iterator name", token.string);
        return nullptr;
    }
    std::string iterator(std::move(token.string));
    token = nextToken(unused);
    if (token.type != inKw) {
        printCantParse(token.begin, "\"in\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "variable name", token.string);
        return nullptr;
    }
    std::string variable(std::move(token.string));
    token = nextToken(unused);
    if (token.type != lBrace) {
        printCantParse(token.begin, "\"{\"", token.string);
        return nullptr;
    }
    nextToken(unused);
    body = parseInstructionSet(unused);
    if (body == nullptr) {
        return nullptr;
    }
    token = getToken(unused);
    if (token.type != rBrace) {
        printCantParse(token.begin, "\"}\"", token.string);
        return nullptr;
    }
    nextToken(unused);
    return  std::make_shared<ForStatement> (std::move(iterator), std::move(variable), std::move(body));
}

std::shared_ptr<ReturnStatement> Parser::parseReturnStatement(std::list<Token> &unused) {
    std::shared_ptr<LogicExpr> value;
    Token token = getToken(unused);
    if (token.type != returnKw) {
        printCantParse(token.begin, "\"return\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    if (token.type != lPar) {
        printCantParse(token.begin, "\"(\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    value = parseLogicExpr(unused);
    if (value == nullptr){
        return nullptr;
    }
    token = getToken(unused);
    if (token.type != rPar) {
        printCantParse(token.begin, "\")\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    if (token.type != colon) {
        printCantParse(token.begin, "\";\"", token.string);
        return nullptr;
    }
    nextToken(unused);
    return std::make_shared<ReturnStatement> (std::move(value));
}

std::shared_ptr<Instruction> Parser::parseInstruction(std::list<Token> &unused) {
    Token token;
    //std::shared_ptr<LogicExpr> logicExpr (std::move(parseLogicExpr(unused)));
    std::shared_ptr<Instruction> instruction;

    /*if (! unused.empty()) {
        token = unused.front();
    } else {
        token = scan.getToken();
    }*/
    token = getToken(unused);
    if (token.type != identifier) {
        instruction = parseLogicExpr(unused);
        if (instruction == nullptr) {
            return nullptr;
        }
        token = getToken(unused);
        if (token.type != colon) {
            printCantParse(token.begin, "\";\"", token.string);
            return nullptr;
        }
        nextToken(unused);
        return instruction;
    }

    if (unused.size() > 1){
        token = *++(unused.begin());
    } else if (unused.size() == 1){
        token = scan.getToken();
    } else {
        unused.push_back(token);
        token = scan.next();
    }
    if (token.type == assignOp){
        instruction = parseAssignment(unused);
        if (instruction == nullptr) {
            return nullptr;
        }
        token = getToken(unused);
        if (token.type != colon) {
            printCantParse(token.begin, "\";\"", token.string);
            return nullptr;
        }
        nextToken(unused);
        return instruction;
    } else if (token.type == identifier){
        instruction = parseVariableDefinition(unused);
        if (instruction == nullptr) {
            return nullptr;
        }
        token = getToken(unused);
        if (token.type != colon) {
            printCantParse(token.begin, "\";\"", token.string);
            return nullptr;
        }
        nextToken(unused);
        return instruction;
    } else {
        instruction = parseLogicExpr(unused);
        if (instruction == nullptr) {
            return nullptr;
        }
        token = getToken(unused);
        if (token.type != colon) {
            printCantParse(token.begin, "\";\"", token.string);
            return nullptr;
        }
        nextToken(unused);
        return instruction;
    }
}

std::shared_ptr<Assignment> Parser::parseAssignment(std::list<Token> &unused) {
    std::string lVal;
    std::shared_ptr<LogicExpr> rVal;
    Token token = getToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "variable name", token.string);
        return nullptr;
    }
    lVal = std::move(token.string);
    token = nextToken(unused);
    if (token.type != assignOp) {
        printCantParse(token.begin, "\"=\"", token.string);
        return nullptr;
    }
    nextToken(unused);
    rVal = parseLogicExpr(unused);
    if (rVal == nullptr) {
        return nullptr;
    }
    return std::make_shared<Assignment> (std::move(lVal), std::move(rVal));
}

std::shared_ptr<VariableDefinition> Parser::parseVariableDefinition(std::list<Token> &unused) {
    std::string type, name, reference;
    Token token = getToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "type", token.string);
        return nullptr;
    }
    type = std::move(token.string);
    token = nextToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "variable name", token.string);
        return nullptr;
    }
    name = std::move(token.string);
    token = nextToken(unused);
    if (token.type == string) {
        reference = std::move(token.string);
        nextToken(unused);
    }
    return std::make_shared<VariableDefinition> (std::move(type), std::move(name), std::move(reference));
}

std::shared_ptr<LogicExpr> Parser::parseLogicExpr(std::list<Token> &unused) {
    bool negated;
    std::shared_ptr<CmpExpr> cmpExpr;
    std::list<std::shared_ptr<CmpExpr>> exprList;
    std::list<TokenType> operators;
    Token token = getToken(unused);
    if (token.type == negOp) {
        negated = true;
        token = nextToken(unused);
    } else {
        negated = false;
    }
    cmpExpr = parseCmpExpr(unused);
    if (cmpExpr == nullptr) {
        return nullptr;
    }
    exprList.push_back(cmpExpr);
    token = getToken(unused);
    while (token.type == orOp || token.type == andOp) {
        operators.push_back(token.type);
        nextToken(unused);
        cmpExpr = parseCmpExpr(unused);
        if (cmpExpr == nullptr) {
            return nullptr;
        }
        exprList.push_back(cmpExpr);
        token = getToken(unused);
    }
    return  std::make_shared<LogicExpr>(negated, std::move(exprList), std::move(operators));
}

std::shared_ptr<CmpExpr> Parser::parseCmpExpr(std::list<Token> &unused) {
    std::shared_ptr<AddExpr> addExpr;
    std::list<std::shared_ptr<AddExpr>> exprList;
    std::list<TokenType> operators;
    Token token = getToken(unused);
    addExpr = parseAddExpr(unused);
    if (addExpr == nullptr) {
        return nullptr;
    }
    exprList.push_back(std::move(addExpr));
    token = getToken(unused);
    while (token.type == moreOp || token.type == lessOp || token.type == eqOp || token.type == negOp ||
           token.type == moreEqOp || token.type == lessEqOp ){
        operators.push_back(token.type);
        nextToken(unused);
        addExpr = parseAddExpr(unused);
        if (addExpr == nullptr){
            return nullptr;
        }
        exprList.push_back(std::move(addExpr));
        token = getToken(unused);
    }

    return std::make_shared<CmpExpr> (std::move(exprList), std::move(operators));
}

std::shared_ptr<AddExpr> Parser::parseAddExpr(std::list<Token> &unused) {
    std::shared_ptr<MultExpr> multExpr;
    std::list<std::shared_ptr<MultExpr>> exprList;
    std::list<TokenType> operators;
    Token token;
    token = getToken(unused);
    multExpr = parseMultExpr(unused);
    if (multExpr == nullptr) {
        return nullptr;
    }
    exprList.push_back(std::move(multExpr));
    token = getToken(unused);
    while (token.type == addOp || token.type == subOp){
        operators.push_back(token.type);
        nextToken(unused);
        multExpr = parseMultExpr(unused);
        if (multExpr == nullptr){
            return nullptr;
        }
        exprList.push_back(std::move(multExpr));
        token = getToken(unused);
    }

    return std::make_shared<AddExpr> (std::move(exprList), std::move(operators));
}

std::shared_ptr<MultExpr> Parser::parseMultExpr(std::list<Token> &unused) {
    std::shared_ptr<Argument> numVar;
    std::list<std::shared_ptr<Argument>> exprList;
    std::list<TokenType> operators;
    Token token = getToken(unused);
    numVar = parseArgument(unused);
    if (numVar == nullptr) {
        return nullptr;
    }
    exprList.push_back(std::move(numVar));
    token = getToken(unused);
    while (token.type == multOp || token.type == divOp){
        operators.push_back(token.type);
        nextToken(unused);
        numVar = parseArgument(unused);
        if (numVar = nullptr){
            return nullptr;
        }
        exprList.push_back(std::move(numVar));
        token = getToken(unused);
    }

    return std::make_shared<MultExpr> (std::move(exprList), std::move(operators));
}

std::shared_ptr<Argument> Parser::parseArgument(std::list<Token> &unused) {
    Token token = getToken(unused);
    switch (token.type) {
        case identifier:
            break;
        case number:
            return parseNumber(unused);
        case string:
            return parseString(unused);
        default:
            printCantParse(token.begin, "const number, variable, function or method call", token.string);
            return nullptr;
    }
    if (unused.size() > 1){
        token = *++(unused.begin());
    } else if (unused.size() == 1){
        token = scan.getToken();
    } else {
        unused.push_back(token);
        token = scan.next();
    }
    switch (token.type){
        case lPar:
            return parseFunctionCall(unused);
        case dot:
            return parseMethodCall(unused);
        default:
            return parseVariable(unused);
    }
}

std::shared_ptr<Variable> Parser::parseVariable(std::list<Token> &unused) {
    Token token = getToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "variable name", token.string);
        return nullptr;
    }
    nextToken(unused);
    return std::make_shared<Variable>(std::move(token.string));
}

std::shared_ptr<Number> Parser::parseNumber(std::list<Token> &unused) {
    Token token = getToken(unused);
    if (token.type != number) {
        printCantParse(token.begin, "const number", token.string);
        return nullptr;
    }
    nextToken(unused);
    return std::make_shared<Number>(std::move(atoi(token.string.c_str())));
}

std::shared_ptr<FunctionCall> Parser::parseFunctionCall(std::list<Token> &unused) {
    std::string name;
    std::shared_ptr<LogicExpr> argument;
    std::list<std::shared_ptr<LogicExpr>> argumentsList;
    Token token = getToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "function name", token.string);
        return nullptr;
    }
    name = std::move(token.string);
    token = nextToken(unused);
    if (token.type != lPar) {
        printCantParse(token.begin, "\"(\"", token.string);
        return nullptr;
    }
    token = nextToken(unused);
    if (token.type != rPar) {
        argument = parseLogicExpr(unused);
        if (argument == nullptr) {
            return nullptr;
        }
        argumentsList.push_back(argument);
        token = getToken(unused);
        while (token.type != rPar) {
            if (token.type != comma) {
                printCantParse(token.begin, "\",\" or \")\"", token.string);
                return nullptr;
            }
            token = nextToken(unused);
            argument = parseLogicExpr(unused);
            if (argument == nullptr) {
                return nullptr;
            }
            argumentsList.push_back(argument);
            token = getToken(unused);
        }
    }
    nextToken(unused);
    return std::make_shared<FunctionCall> (std::move(name), std::move(argumentsList));
}

std::shared_ptr<MethodCall> Parser::parseMethodCall(std::list<Token> &unused) {
    std::string name;
    std::shared_ptr<FunctionCall> functionCall;
    Token token = getToken(unused);
    if (token.type != identifier) {
        printCantParse(token.begin, "variable name", token.string);
        return nullptr;
    }
    name = std::move(token.string);
    token = nextToken(unused);
    if (token.type != dot) {
        printCantParse(token.begin, ".", token.string);
        return nullptr;
    }
    nextToken(unused);
    functionCall = parseFunctionCall(unused);
    if (functionCall == nullptr){
        return nullptr;
    }
    return std::make_shared<MethodCall> (std::move(name), std::move(functionCall));
}

std::shared_ptr<String> Parser::parseString(std::list<Token> &unused) {
    Token token = getToken(unused);
    if (token.type != string) {
        printCantParse(token.begin, "const string", token.string);
        return nullptr;
    }
    nextToken(unused);
    return std::make_shared<String>(std::move(token.string));
}

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
    for (std::shared_ptr<LogicExpr> i: arguments) {
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
    for (std::shared_ptr<Argument> i: exprList) {
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
    for (std::shared_ptr<MultExpr> i: exprList) {
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
    for (std::shared_ptr<AddExpr> i: exprList) {
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
    for (std::shared_ptr<CmpExpr> i: exprList) {
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
    for (std::shared_ptr<Statement> const &i: statements) {
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
    for (std::shared_ptr<ArgumentPair> const &i: argumentsList) {
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
    for (std::shared_ptr<FunctionDefinition> const &i: Functions) {
        i->printValue(setw + 4);
    }
    std::cout << std::string(setw, ' ') << "}" << std::endl;
    std::cout << std::string(setw + 4, ' ') << "instruction set {" << std::endl;
    instructionSet->printValue(setw + 4);
    std::cout << std::string(setw, ' ') << "}" << std::endl;
}

Token Parser::getToken(std::list<Token> &unused){
    if (unused.empty()){
        return scan.getToken();
    } else {
        return unused.front();
    }
}
Token Parser::nextToken(std::list<Token> &unused) {
    if (unused.empty()){
        return scan.next();
    } else {
        unused.pop_front();
        if (unused.empty()){
            return scan.getToken();
        } else {
            return unused.front();
        }
    }
}

