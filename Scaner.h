//
// Created by michal on 13.04.18.
//

#ifndef SCANER_SCANER_H
#define SCANER_SCANER_H

#include <map>
#include "Source.h"

enum TokenType {
    forKw,
    eachKw,
    inKw,
    doKw,
    ifKw,
    returnKw,
    identifier,
    negOp,
    andOp,
    orOp,
    moreOp,
    lessOp,
    eqOp,
    neqOp,
    moreEqOp,
    lessEqOp,
    addOp,
    subOp,
    multOp,
    divOp,
    assignOp,
    number,
    string,
    lBrace,
    rBrace,
    lPar,
    rPar,
    colon,
    comma,
    dot,
    eof,
    NaT //not a token
};

struct Token {
    std::string string;
    TokenType type;
    TextPos begin;
//    TextPos end;
};

class Scaner {
private:
    Source& src;
    std::map<std::string, enum TokenType> kwMap;
    Token curr;

    Token getIdent();
    Token getNum();
    Token getOperator();
    void skipComment();
public:
    explicit Scaner(Source& src);
    Token next();
    Token getToken();
};

#endif //SCANER_SCANER_H
