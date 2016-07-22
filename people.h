#pragma once
#include "constants.h"

class people
{
public:
    people();
    std::vector<double> trait1;
    std::vector<Point> home;
    double trait1_update(int pi);
};
