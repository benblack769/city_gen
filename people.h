#pragma once
#include "constants.h"
#include <infoholder.h>
#include <random>
#include <cstdlib>

class Person{
public:
    //short term properties
    double energy;//food
    double health;//shelter

    Point location;//don't change directly! use map_set
    
public:
    //longer term properties
    //double wander_tendancy;
    //double base_move_speed;
public:
};
class People:
    public InfoHolder<Person>{
public: 
};
