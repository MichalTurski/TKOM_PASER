//
// Created by michal on 13.04.18.
//

#include <iostream>
#include "Scaner.h"

Scaner::Scaner(Source &src): src(src) {
    kwMap.insert(std::make_pair("for", forKw));
    kwMap.insert(std::make_pair("each", eachKw));
    kwMap.insert(std::make_pair("in", inKw));
    kwMap.insert(std::make_pair("do", doKw));
    kwMap.insert(std::make_pair("if", ifKw));
    kwMap.insert(std::make_pair("return", returnKw));
    kwMap.insert(std::make_pair("function", functionKw));
    kwMap.insert(std::make_pair("group", groupKw));
    next();
}

Token* Scaner::getCurr() {
    return &curr;
}

Token* Scaner::next() {
    int c;

    c = src.getChar();
    while (isspace(c) || c == '/') {
        if (isspace(c)) {
            src.moveForward();
            c = src.getChar();
        } else { //Comment or div operator
            curr.begin = src.getTexstPos();
            src.moveForward();
            if (src.getChar() == '*') {
                skipComment();
                c = src.getChar();
            } else {
                curr.type = divOp;
                curr.string = "/";
                return &curr;
            }
        }
    }
    if (c == EOF) {
        curr.type = eof;
        curr.begin = src.getTexstPos();
    } else if (isalpha(c)) {
        curr = getIdent();
    } else if (isdigit(c)) {
        curr = getNum();
    } else if (c == '"') {
        curr = getString();
    } else {
        curr = getOperator();
    }
    return (&curr);
}

Token Scaner::getIdent() {
    int c;
    Token token;
    std::string curString;

    token.begin = src.getTexstPos();
    c = src.getChar();
    while (isalpha(c) || c == '_') {
        curString += (char)c;
        src.moveForward();
        c = src.getChar();
    }

    token.string = std::move(curString);
    auto kwTuple = kwMap.find(token.string);
    if (kwTuple != kwMap.end()){
        token.type = kwTuple->second;
    } else {
        token.type = identifier;
    }
    return (token);
}

Token Scaner::getNum() {
    int c;
    Token token;
    std::string curString;

    token.begin = src.getTexstPos();
    c = src.getChar();
    if (!isdigit(c)) {
        throw std::runtime_error("getNum() called on not a number");
    }
    if (c == '0') {
        curString += (char) c;
        src.moveForward(); //src.getChar() should return not used character
        token.string = std::move(curString);
        token.type = number;
        return (token);
    }
    while (isdigit(c)) {
        curString += (char) c;
        src.moveForward();
        c = src.getChar();
    }
    token.string = std::move(curString);
    token.type = number;
    return (token);
}

Token Scaner::getOperator() {
    int c;
    Token token;
    std::string curString;

    token.begin = src.getTexstPos();
    c = src.getChar();
    curString += (char) c;

    switch (c) {
        case '!':
            src.moveForward();
            if (src.getChar() == '=') {
                curString += '=';
                token.type = neqOp;
                src.moveForward();
            } else {
                token.type = negOp;
            }
            token.string = std::move(curString);
            return (token);
        case '&':
            src.moveForward();
            if (src.getChar() != '&') {
                token.string =  std::move(curString);
                token.type = refOp;
                return (token);
            } else {
                curString += (char) c;
                token.string = std::move(curString);
                token.type = andOp;
                src.moveForward();
            }
            return (token);
        case '|':
            src.moveForward();
            if (src.getChar() != '|') {
                token.type = NaT;
            } else {
                curString += (char) c;
                token.string = std::move(curString);
                token.type = orOp;
                src.moveForward();
            }
            return (token);
        case '>':
            src.moveForward();
            if ((c = src.getChar()) != '=') {
                token.type = moreOp;
            } else {
                curString += (char) c;
                token.type = moreEqOp;
                src.moveForward();
            }
            token.string = std::move(curString);
            return (token);
        case '<':
            src.moveForward();
            if ((c = src.getChar()) != '=') {
                token.type = lessOp;
            } else {
                curString += (char) c;
                token.type = lessEqOp;
                src.moveForward();
            }
            token.string = std::move(curString);
            return (token);
        case '=':
            src.moveForward();
            if ((c = src.getChar()) != '=') {
                token.type = assignOp;
            } else {
                curString += (char) c;
                token.type = eqOp;
                src.moveForward();
            }
            token.string = std::move(curString);
            return (token);
        case '+':
            src.moveForward();
            token.type = addOp;
            token.string = std::move(curString);
            return (token);
        case '-':
            src.moveForward();
            token.type = subOp;
            token.string = std::move(curString);
            return (token);
        case '*':
            src.moveForward();
            token.type = multOp;
            token.string = std::move(curString);
            return (token);
        case '/':
            src.moveForward();
            token.type = divOp;
            token.string = std::move(curString);
            return (token);
        case '{':
            src.moveForward();
            token.type = lBrace;
            token.string = std::move(curString);
            return (token);
        case '}':
            src.moveForward();
            token.type = rBrace;
            token.string = std::move(curString);
            return (token);
        case '(':
            src.moveForward();
            token.type = lPar;
            token.string = std::move(curString);
            return (token);
        case ')':
            src.moveForward();
            token.type = rPar;
            token.string = std::move(curString);
            return (token);
        case ';':
            src.moveForward();
            token.type = colon;
            token.string = std::move(curString);
            return (token);
        case ',':
            src.moveForward();
            token.type = comma;
            token.string = std::move(curString);
            return (token);
        case '.':
            src.moveForward();
            token.type = dot;
            token.string = std::move(curString);
            return (token);
        default:
            token.type = NaT;
            return (token);
    }
}

Token Scaner::getString() {
    int c;
    Token token;
    std::string curString;

    token.begin = src.getTexstPos();
    c = src.getChar();
    if (c != '"') {
        token.type = NaT;
        return (token);
    }
    do {
        curString += (char) c;
        src.moveForward();
        c = src.getChar();
    } while (c != '"');
    curString += (char) c;
    src.moveForward();
    token.type = string;
    token.string = std::move(curString);
    return (token);
}

void Scaner::skipComment() {
    int c;

    c = src.getChar();
    if (c != '*') {
        throw std::runtime_error("skipComment() called on not an Comment");
    }
    do {
        do {
            src.moveForward();
            c = src.getChar();
            if (c == EOF) {
                std::cerr << "Comment reached end of file.\n";
                exit (-1);
            }
        } while (c != '*');
        do {
            src.moveForward();
            c = src.getChar();
            if (c == EOF) {
                std::cerr << "Comment reached end of file.\n";
                exit (-1);
            }
        } while (c == '*');
    } while (c != '/');
    src.moveForward(); //src.getChar() should return not used character
}

