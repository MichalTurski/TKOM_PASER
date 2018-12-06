//
// Created by michal on 13.04.18.
//

#include <iostream>
#include <cassert>
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

    c = skipUnused();
    curr.begin = src.getTexstPos();
    --curr.begin.num; //First character have been consumed by skipUnused() yet.
    if (c == EOF) {
        curr.type = eof;
    } else if (isalpha(c)) {
        curr = getIdent(c);
    } else if (isdigit(c)) {
        curr = getNum(c);
    } else if (c == '"') {
        curr = getString(c);
    } else {
        curr = getOperator(c);
    }
    return (&curr);
}

Token Scaner::getIdent(int first) {
    int c;
    Token token;
    std::string curString;

    c = first;
    curString += (char)c;
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

Token Scaner::getNum(int first) {
    int c;
    Token token;
    std::string curString;

    c = first;
    if (!isdigit(c)) {
        throw std::runtime_error("getNum() called on not a number");
    }
    if (c == '0') {
        curString += (char) c;
        token.string = std::move(curString);
        token.type = number;
        return (token);
    }
    curString += (char) c;
    c = src.getChar();
    while (isdigit(c)) {
        curString += (char) c;
        src.moveForward();
        c = src.getChar();
    }
    token.string = std::move(curString);
    token.type = number;
    return (token);
}

Token Scaner::getOperator(int first) {
    int c;
    Token token;
    std::string curString;

    c = first;
    curString += (char) c;
    switch (c) {
        case '!':
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
            if (src.getChar() != '&') {
                token.type = refOp;
            } else {
                curString += (char) c;
                token.type = andOp;
                src.moveForward();
            }
            token.string =  std::move(curString);
            return (token);
        case '|':
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
            token.type = addOp;
            token.string = std::move(curString);
            return (token);
        case '-':
            token.type = subOp;
            token.string = std::move(curString);
            return (token);
        case '*':
            src.moveForward();
            token.type = multOp;
            token.string = std::move(curString);
            return (token);
        case '/':
            token.type = divOp;
            token.string = std::move(curString);
            return (token);
        case '{':
            token.type = lBrace;
            token.string = std::move(curString);
            return (token);
        case '}':
            token.type = rBrace;
            token.string = std::move(curString);
            return (token);
        case '(':
            token.type = lPar;
            token.string = std::move(curString);
            return (token);
        case ')':
            token.type = rPar;
            token.string = std::move(curString);
            return (token);
        case ';':
            token.type = colon;
            token.string = std::move(curString);
            return (token);
        case ',':
            token.type = comma;
            token.string = std::move(curString);
            return (token);
        case '.':
            token.type = dot;
            token.string = std::move(curString);
            return (token);
        default:
            token.type = NaT;
            return (token);
    }
}

Token Scaner::getString(int first) {
    int c;
    Token token;
    std::string curString;

    c = first;
    if (c != '"') {
        token.type = NaT;
        return (token);
    }
    c = src.getChar();
    src.moveForward();
    while (c != '"') {
        curString += (char) c;
        c = src.getChar();
        src.moveForward();
    }
    token.type = string;
    token.string = std::move(curString);
    return (token);
}

/*while (isspace(c) || c == '/') {
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
}*/
int Scaner::skipUnused() {
    int c = src.getChar();
    src.moveForward();
    while (isspace(c) || c == '/') {
        if (c == '/') {
            c = src.getChar();
            if (c != '*') {
                return '/';
            } else {
                skipComment();
            }
        }
        c = src.getChar();
        src.moveForward();
    }
    return c;
}
void Scaner::skipComment() {
    int c;

    c = src.getChar();
    assert(c == '*');
    do {
        do {
            src.moveForward();
            c = src.getChar();
            if (c == EOF) {
                std::cerr << "Comment reached end of file.\n";
                exit (1);
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
