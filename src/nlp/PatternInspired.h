//
// Created by zhsyourai on 6/9/17.
//

#ifndef JARVIS_PATTERNINSPIRED_H
#define JARVIS_PATTERNINSPIRED_H

#include <string>
#include "common.h"

class PatternInspired {
public:
    PatternInspired();

    virtual ~PatternInspired();

    void start();

    void feed(std::string source, std::string relate, std::string target);

    Action end();
};


#endif //JARVIS_PATTERNINSPIRED_H
