#pragma once
#include "constants.h"

constexpr int num_basic_choices = 3;
constexpr int num_movement_choices = 9;
constexpr int num_choices = num_basic_choices+num_movement_choices;

enum basic_choice{SHELTER,EAT,REST,MOVE};
struct full_choice{
    basic_choice base;
    Point move_dir;
    full_choice(basic_choice basic, int in_x, int in_y){
        base = basic;
        move_dir = Point(in_x, in_y);
    
        assert(in_x <= 1 && in_x >= -1);
        assert(in_y <= 1 && in_y >= -1);
    }
    full_choice(basic_choice basic, Point p):
        full_choice(basic,p.X,p.Y){}
};

inline full_choice get_choice(int choice_idx){
    assert(choice_idx >= 0 && choice_idx < num_choices);
    if (choice_idx >= num_basic_choices){
        int move_loc = choice_idx - num_basic_choices;
        full_choice{MOVE,move_loc%3-1,move_loc/3-1};
    }
    else{
        return full_choice{static_cast<basic_choice>(choice_idx),-1,-1};
    }
}
inline int choice_idx(full_choice choice){
    int res = choice.base == MOVE ? ((choice.move_dir.Y+1)*3 + (choice.move_dir.X+1)) + num_basic_choices : choice.base;
    assert(res >= 0 && res < num_choices);
    return res;
}
