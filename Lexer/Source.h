//
// Created by michal on 13.04.18.
//

#ifndef SCANER_SOURCE_H
#define SCANER_SOURCE_H

#include <fstream>

struct TextPos{
    int line;
    int num;

    TextPos(int line = 0, int num = 0);
};

class Source {
private:
    const std::string f_name;
    std::ifstream fstream;
    TextPos textPos;
    std::string line;
    bool fileEnd;

    bool nextLine();
public:
    Source(const std::string &f_name);
    ~Source ();

    int getChar();
    struct TextPos getTexstPos();
    void moveForward();
};


#endif //SCANER_SOURCE_H
