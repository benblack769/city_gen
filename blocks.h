#pragma once
#include "constants.h"
#include <Array2d.hpp>

// purpose of class is to act as a way to efficiently display data that is stored in the people
// does not hold any useful data of its own that needs to be saved.
class blocks
{
public:
    //people:

    //owner
    //people living in it
    //people working in it
    //people visiting it

    //government level stats:

    //transportation infrastructure inventment/level


    //other viewable stats/figure:

    //pop density
    //means of all the personal statistics(posibly also medians, and other statistics)
    //buissness density
    //means of all buisness statsitics

    using count_ty = Array2d<size_t,WORLD_SIZE,WORLD_SIZE>;
    using people_holder = Array2d<vector<size_t>,WORLD_SIZE,WORLD_SIZE>;
    count_ty num_residents;

    blocks();
};
