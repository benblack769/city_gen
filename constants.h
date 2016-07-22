#pragma once
#include <stddef.h>
#include <point.hpp>
#include <vector>

using namespace std;

constexpr size_t WORLD_SIZE = 1000;

constexpr size_t NUM_PEOPLE = 10000;

using point_iter = PointIter<WORLD_SIZE,WORLD_SIZE>;
using point_iter_cont = PIterContainter<WORLD_SIZE,WORLD_SIZE>;

inline point_iter_cont iter_all(){
    return point_iter_cont(0,0,WORLD_SIZE,WORLD_SIZE);
}
