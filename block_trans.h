/*
1. Essencial types and declarations used by block.
2. Aproximization functions to tweak and experiment with.
*/
#pragma once
#include <vector>
#include <cstdint>
#include <cassert>
#include <array>
#include <headerlib/Array2d.hpp>
#include <headerlib/point.hpp>
#include <headerlib/range_array.hpp>
#include "constants.h"

using namespace std;

using move_cost_ty = float;

struct Edge{
    Edge(float indis=0):
        dis(indis){
    }
    uint32_t invest = 0;
    float dis;
    move_cost_ty movecost = 0;//cached value caculated from investment
    move_cost_ty marg_benefit_invest = 0;
};
using Node = RangeArray<Edge,array<Edge,9>>;

using tier_ty = FArray2d<Node>;
using graph_ty = array<tier_ty,NUM_TIERS>;

graph_ty make_graph();

