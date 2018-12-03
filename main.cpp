//
// Created by michal on 18.04.18.
//
#include <iostream>
#include <cstring>
#include <memory>
#include <iomanip>
#include <cassert>

#include "Lexer/Scaner.h"
#include "Lexer/Source.h"
#include "Parser/Parser.h"

void loadLibrary(std::string name) {

}

int main(int argc, char **argv) {
    int opt;
    bool lex, pars, interpret;
    lex = false;
    pars = false;
    interpret = false;
    std::string srcName;
    std::list<std::string> libNames;

    while ((opt = getopt(argc, argv, "l:p:i:L:")) != EOF) {
        switch (opt) {
            case 'l':
                srcName = optarg;
                lex = true;
                break;
            case 'p':
                srcName = optarg;
                pars = true;
                break;
            case 'i':
                srcName = optarg;
                interpret = true;
                break;
            case 'L':
                libNames.emplace_back(optarg);
                break;
            default:
                std::cerr<<"Usage:" <<std::endl << argv[0] << " -l|-p|-i file {-L library}" <<std::endl;
                return -1;
        }
    }
    Source src(argv[2]);
    Scaner scaner(src);
    if (lex && !(pars || interpret)) {
        Token *token;
        do {
            token = scaner.next();
            if (token->type != NaT)
                std::cout << token->string << std::endl;
            else
                std::cout << "Wrong Token (line " << token->begin.line
                          << " column " << token->begin.num + 1 << ")" << std::endl;
        } while (token->type != NaT && token->type != eof);
    } else if (pars && !(lex || interpret)) {
        Parser parser(src, scaner);
        try {
            std::unique_ptr<Program> program = parser.parseProgram();
            program->printValue(0);
        } catch (std::runtime_error &error) {
            std::cerr << error.what();
            return -1;
        }
    } else if (interpret && !(lex || pars)) {
        Parser parser(src, scaner);
        try {
            std::unique_ptr<Program> program = parser.parseProgram();
            return program->execute(libNames);
        } catch (std::runtime_error &error) {
            std::cerr << error.what();
            return -1;
        }
    } else {
        std::cerr<<"Usage:" <<std::endl << argv[0] << " -l|-p|-i file {-L library}" <<std::endl;
        return 1;
    }
    /*if (argc != 3) {
        std::cerr<<"Usage:" <<std::endl << argv[0] << " lex|pars|interpret file" <<std::endl;
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
    }  else if (strcmp(argv[1], "interpret") == 0) {
        Parser parser(src, scaner);
        std::unique_ptr<Program> program = parser.parseProgram();
        program->execute();
        return 40;
    } else {
        std::cerr<<"Usage:" <<std::endl << argv[0] << " lex|pars|interpret file" <<std::endl;
    }*/
}
