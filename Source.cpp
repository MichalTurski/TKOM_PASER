//
// Created by michal on 13.04.18.
//

#include <iostream>
#include <stdexcept>
#include "Source.h"

Source::Source(const std::string &f_name) : f_name(f_name) {
   fstream.open(f_name.c_str());
   if(!fstream)
      throw std::runtime_error("Unable to open file.");
   if(fstream.eof())
       fileEnd = true;
   else
       fileEnd = false;
   nextLine();
}

Source::~Source() {
   fstream.close();
}

bool Source::nextLine() {
   if(fstream.eof())
      return false;
   getline(fstream, line);
   line.push_back('\n');

   textPos.line++;
   textPos.num = 0;
   return true;
}

int Source::getChar(){
    if(fileEnd)
        return EOF;
    return line[textPos.num];
}

struct TextPos Source::getTexstPos() {
       return textPos;
}

void Source::moveForward() {
    if(textPos.num == line.size() - 1) {
        if (!nextLine())
            fileEnd = true;
    } else {
        textPos.num++;
    }
}

TextPos::TextPos(int line, int num) : line(line), num(num) {}
