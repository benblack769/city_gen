#pragma once
#include "constants.h"

class people
{
public:
    people();
    std::vector<Point> home;
    std::vector<Point> work;
    std::vector<size_t> income;
    
    size_t size(){
        return home.size();
    }
    void add_person(Point phome, Point pwork){
        home.push_back(phome);
        work.push_back(pwork);
    }
};
