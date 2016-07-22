#pragma once
#include "constants.h"

class people
{
public:
    people();
    std::vector<Point> home;
    std::vector<Point> work;
    size_t size(){
        return home.size();
    }
};
