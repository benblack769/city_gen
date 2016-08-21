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

template <typename data_ty>
using SqrData = RangeArray<data_ty,array<data_ty,9>>;
template <typename data_ty>
using tier_data = FArray2d<SqrData<data_ty>>;
template <typename data_ty>
using graph_data = array<tier_data<data_ty>,NUM_TIERS>;

using Node = SqrData<move_cost_ty>;
using tier_ty = tier_data<move_cost_ty>;
using graph_ty = graph_data<move_cost_ty>;

using NodeInv = SqrData<uint32_t>;
using board_inv_ty = FArray2d<NodeInv>;



board_inv_ty init_trans_inv();

