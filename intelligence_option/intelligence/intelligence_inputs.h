#pragma once
#include <vector>
#include "map.h"
#include "people.h"
using namespace std;

void fix_inputs(vector<double> & outvals, Person pinfo, const vector<PointProperty> & points_around){
    outvals.resize(0);
    outvals.push_back(pinfo.energy);
    outvals.push_back(pinfo.health);
    for(PointProperty point : points_around){
        outvals.push_back(point.food_content);
        outvals.push_back(point.shelter_val);
        outvals.push_back(point.travel_ease);
        outvals.push_back(point.residents.size());
    }
}

