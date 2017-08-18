#pragma once
/*
This file is a general purpse intelligence which has the 
following API

It is important that it depends on no information 
about the system being acted upon, i.e. constants, types, etc.

All additions to the API must be carefully justified

constructor:
    parameters:
        input_size:int
        output_size:int
        
    justification:
        Basic requirements for static sized intelligence,
        static size is necessary for reasonable operation and efficiency

impose_cost:
    parameters:
        cost: float - Value the intelligence tries to minimize
                This value is needs to be relative to itself 
                over time, no need for any absolute value. 
        
next_step:
    parameters:
        inputs: vector<float>: Required [0 < value < 1]
*/
#include "intelligence/math.h"
#include "utils.h"
int make_choice(const vecfloat & out_data){
    //max of
}

class ChoiceMaker{
public:
    vector<vecfloat> weights;
    vecfloat biases;
    ChoiceMaker(int in_size, int out_size){
        weights.assign(out_size,vecfloat(in_size));
        for(int i = 0; i < out_size; i++){
            for(int j = 0; j < in_size; j++){
                weights[i][j] = nrand();
            }
        }
    }
};









