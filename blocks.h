#pragma once
#include "constants.h"
#include "people.h"
#include "firms.h"
#include <Array2d.hpp>

template<typename obj_ty>
class board:
        public FArray2d<obj_ty>{
public:
    board(obj_ty initval=obj_ty()):
        FArray2d<obj_ty>(WORLD_SIZE,WORLD_SIZE,initval){
    }
};
//
//static version of above
//
//template<typename obj_ty>
//using board = FArray2d<obj_ty>;//Array2d<obj_ty,WORLD_SIZE,WORLD_SIZE>;

//transit types/consts
using move_cost_ty = float;
using mcarr = board<move_cost_ty>;
using count_ty = board<size_t>;
using people_holder = board<vector<size_t>>;

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
    
    people pps;
    
    count_ty num_residents;
    count_ty num_workers;
    
    count_ty trans_invest;
    static constexpr size_t inv_per_turn = 1;
    count_ty trans_usage;
    
    count_ty size_t_upgrade_vs;

    blocks();
    static constexpr size_t arrsize(){
        return WORLD_SIZE*WORLD_SIZE;
    }
    void update();
protected:
    void update_block_info();
    void update_trans();
};
