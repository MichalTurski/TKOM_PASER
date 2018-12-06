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
    functionKw,
    groupKw,
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
    refOp,
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
};

class Scaner {
private:
    Source& src;
    std::map<std::string, enum TokenType> kwMap;
    Token curr;

    Token getIdent(int first);
    Token getNum(int first);
    Token getOperator(int first);
    Token getString(int first);
    int skipUnused(); /* Returns first character after unused part. */
    void skipComment();
public:
    explicit Scaner(Source& src);
    Token *next();
    Token *getCurr();
};

#endif //SCANER_SCANER_H
