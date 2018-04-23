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
}

enum ScanerState {
    START,
    AFTER_SLASH,
    COMMENT,
    COMMENT_STAR,
    AFTER_NOT,
    AFTER_AND,
    AFTER_OR,
    AFTER_LESS,
    AFTER_MORE,
    AFTER_EQ,
    IDENT,
    NUMBER,
    STRING
};

Token Scaner::getToken() {
    int c;
    Token token;
    ScanerState state = START;
    std::string curString = "";
    TextPos prevPos;

    while(true) {
        c = src.getChar();

        switch (state) {
            case (START):
                if (c == EOF) {
                    token.type = eof;
                    token.begin = src.getTexstPos();
                    token.end = src.getTexstPos();
                    goto out;
                }
                if (isspace(c)) {
                    break;
                }
                if (c == '/') {
                    state = AFTER_SLASH;
                    curString += (char) c;
                    token.begin = src.getTexstPos();
                    break;
                }
                if (c == '!') {
                    state = AFTER_NOT;
                    curString += (char) c;
                    token.begin = src.getTexstPos();
                    break;
                }
                if (c == '&') {
                    state = AFTER_AND;
                    curString += (char) c;
                    token.begin = src.getTexstPos();
                    break;
                }
                if (c == '|') {
                    state = AFTER_OR;
                    curString += (char) c;
                    token.begin = src.getTexstPos();
                    break;
                }
                if (c == '>') {
                    state = AFTER_MORE;
                    curString += (char) c;
                    token.begin = src.getTexstPos();
                    break;
                }
                if (c == '<') {
                    state = AFTER_LESS;
                    curString += (char) c;
                    token.begin = src.getTexstPos();
                    break;
                }
                if (c == '=') {
                    state = AFTER_EQ;
                    curString += (char) c;
                    token.begin = src.getTexstPos();
                    break;
                }
                if (c == '+') {
                    token.type = addOp;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == '-') {
                    token.type = subOp;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == '*') {
                    token.type = multOp;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == '{') {
                    token.type = lBrace;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == '}') {
                    token.type = rBrace;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == '(') {
                    token.type = lPar;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == ')') {
                    token.type = rPar;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == ';') {
                    token.type = colon;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == ',') {
                    token.type = comma;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == '.') {
                    token.type = dot;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (c == '"') {
                    token.begin = src.getTexstPos();
                    state = STRING;
                    break;
                }
                if (c == '0') {
                    token.type = number;
                    token.begin = src.getTexstPos();
                    goto consume_out;
                }
                if (isdigit(c)) { //zero-beginning digits were caught by previous if
                    state = NUMBER;
                    token.begin = src.getTexstPos();
                    curString += (char) c;
                    break;
                }
                if (isalpha(c)) {
                    state = IDENT;
                    token.begin = src.getTexstPos();
                    curString += (char) c;
                    break;
                }
                //Token not recoginised
                token.type = NaT;
                token.begin = src.getTexstPos();
                goto consume_out;
            case (AFTER_SLASH):
                if (c == '*') {
                    state = COMMENT;
                    break;
                } else if (c == EOF) {
                    std::cout << "Error: Comment reached end of file." << std::endl;
                    token.type = NaT;
                    goto nonconsume_out;
                } else {
                    token.type = divOp;
                    goto nonconsume_out; //Don't move forward, current character might be part of next token
                }
            case (COMMENT):
                if (c == '*') {
                    state = COMMENT_STAR;
                } else if (c == EOF) {
                    std::cout << "Error: Comment reached end of file." << std::endl;
                    token.type = NaT;
                    goto nonconsume_out;
                }
                break;
            case (COMMENT_STAR):
                if (c == '*') {
                    state = COMMENT_STAR;
                } else if (c == '/') {
                    state = START;
                    curString.clear();
                } else if (c == EOF) {
                    token.type = NaT;
                    goto nonconsume_out;
                } else {
                    state = COMMENT;
                }
                break;
            case (AFTER_NOT):
                if (c == '=') {
                    token.type = neqOp;
                    goto consume_out;
                } else {
                    token.type = negOp;
                    goto nonconsume_out; //Don't move forward, current character might be part of next token
                }
            case (AFTER_AND):
                if (c == '&') {
                    token.type = andOp;
                    goto consume_out;
                } else {
                    token.type = NaT;
                    goto nonconsume_out;
                }
            case (AFTER_OR):
                if (c == '|') {
                    token.type = orOp;
                    goto consume_out;
                } else {
                    token.type = NaT;
                    goto nonconsume_out;
                }
            case (AFTER_MORE):
                if (c == '=') {
                    token.type = moreEqOp;
                    goto consume_out;
                } else {
                    token.type = moreOp;
                    goto nonconsume_out; //Don't move forward, current character might be part of next token
                }
            case (AFTER_LESS):
                if (c == '=') {
                    token.type = lessEqOp;
                    goto consume_out;
                } else {
                    token.type = lessOp;
                    goto nonconsume_out; //Don't move forward, current character might be part of next token
                }
            case (AFTER_EQ):
                if (c == '=') {
                    token.type = eqOp;
                    goto consume_out;
                } else {
                    token.type = assignOp;
                    goto nonconsume_out; //Don't move forward, current character might be part of next token
                }
            case (IDENT):
                if (isalnum(c) || c == '_') {
                    curString += (char) c;
                    break;
                } else {
                    token.type = identifier;
                    goto nonconsume_out; //Don't move forward, current character might be part of next token
                }
            case (NUMBER):
                if (isdigit(c)) {
                    curString += (char) c;
                    break;
                } else {
                    token.type = number;
                    goto nonconsume_out; //Don't move forward, current character might be part of next token
                }
            case (STRING):
                if (c == '"') {//This is specific case, we want to move src forward, but skip '"'
                    token.type = string;
                    src.moveForward();
                    goto out;
                } else if (isalnum(c) || isspace(c) || c == '@' || c == '.' || c == ',' || c == '_') {
                    curString += (char) c;
                    break;
                } else {
                    token.type = NaT;
                    goto consume_out;
                }
        }
        src.moveForward();
        prevPos = src.getTexstPos();
    }
nonconsume_out:
    token.end = prevPos;
    goto out;
consume_out:
    curString += c;
    src.moveForward();
    token.end = src.getTexstPos();
out:
    token.string = std::move(curString);
    if (token.type == identifier){
        auto kwTuple = kwMap.find(token.string);
        if (kwTuple != kwMap.end()){
            token.type = kwTuple->second;
        }
    }
    return token;
}
