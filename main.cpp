//
// Created by michal on 18.04.18.
//
#include <iostream>
#include <cstring>
#include <memory>
#include <iomanip>

#include "Scaner.h"
#include "Source.h"
#include "Parser.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage:" << std::endl << argv[0] << " lex|pars file" << std::endl;
        return -1;
    }

    Source src(argv[2]);
    Scaner scaner(src);
    if (strcmp(argv[1], "lex") == 0 ) {
        Token *token;

        do {
            token = scaner.next();
            if (token->type != NaT)
                std::cout << token->string << std::endl;
            else
                std::cout << "Wrong Token (line " << token->begin.line
                          << " column " << token->begin.num + 1 << ")" << std::endl;
        } while (token->type != NaT && token->type != eof);
    } else if (strcmp(argv[1], "pars") == 0 ) {
        Parser parser(src, scaner);
        std::unique_ptr<Program> program = parser.parseProgram();
        program->printValue(0);
    } else {
        std::cerr<<"Usage:" <<std::endl << argv[0] << " lex|pars file" <<std::endl;
    }
}
