#pragma once
#include <vector>
#include "point.hpp"
using namespace std;


class firms
{
public:
    vector<size_t> owner;
    vector<Point> location;
    firms();
    size_t size(){
        return location.size();
    }
};
