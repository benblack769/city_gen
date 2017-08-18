#pragma once
#include <vector>
#include <cassert>
#include <cmath>
#include <random>

using namespace std;

default_random_engine rand_gen(clock());

using vecfloat = vector<float>;

float sqr(float x){
    return x*x;
}

float deriv_tanh(float x){
    return 1/(1+sqr(x));
}
float sigmoid(float x){
    return 1/(1+exp(-x));
}
float sigderiv(float x){
    return -sqr(sigmoid(x))*exp(-x)*(-1);
}

void standardize_input_mags(vector<Edge> & edges){
    float standard_mag = 1.0;
    
    float sum = 0;
    for(Edge e : edges){
        sum += e.connection_weight;
    }
    float mul_val = standard_mag/sum;
    for(Edge & e : edges){
        e.connection_weight *= mul_val;
    }
}

void sparse_mat_mul(vector<float> & outputs,vector<vector<Edge>> & edges, vector<float> & inputs){
    assert(inputs.size() == edges.size());
    
    size_t outsize = outputs.size();
    size_t insize = inputs.size();
    
    fill(outputs.begin(),outputs.end(),0);
    for(int i = 0; i < insize; i++){
        for(Edge & e : edges[i]){
            outputs[e.node_num] += e.connection_weight * inputs[i];
        }
    }
}
void update_edges(vector<vector<Edge>> & edges, const vector<float> & inputs, const vector<float> & errors){
    assert(inputs.size() == edges.size());
    size_t outsize = errors.size();
    size_t insize = inputs.size();
    
    for(int i = 0; i < insize; i++){
        for(Edge & e : edges[i]){
             float update_val = errors[e.node_num] * inputs[i];
             e.connection_weight -= update_val;
             //e.connection_weight = max(0.001f,e.connection_weight);
        }
    }
}
void degrade_weights(vector<vector<Edge>> & graph){
    for(vecEdge & node : graph){
        for(Edge & e : node){
            e.connection_weight *= 0.999;
        }
    }
}

float sum(const vector<float> & add){
    float sum = 0;
    for(float v : add){
        sum += v;
    }
    return sum;
}
void mul_by_scalar(vector<float> & out,float by){
    for(float & v : out){
        v *= by;
    }
}
void in_place_add(const vector<float> & add,vector<float> & to){
    assert(add.size() == to.size());
    for(int i = 0; i < add.size(); i++){
        to[i] += add[i];
    }
}
vector<float> vabs(const vector<float> & input){
    vector<float> res(input.size());
    for(int i = 0; i < input.size(); i++){
        res[i] = abs(input[i]);
    }
    return res;
}
template<typename fn_ty>
void apply_vec(vecfloat & outputs,fn_ty fn){
    for(float & out : outputs){
        out = fn(out);
    }
}

void normalize(vector<float> & vals){
    float tot_o = sum(vabs(vals));
    if(tot_o == 0){
        assert(false && "sum is zero");
    }
    mul_by_scalar(vals,1.0/tot_o);
}


float nrand(){
    normal_distribution<float> dist(0,1);
    return dist(rand_gen);
}

float get_rand_weight(int num_connections){
    double std_dev = 1.0/sqrt(num_connections);
    normal_distribution<float> dist(0,std_dev);
    return abs(dist(rand_gen));
}
int get_rand_num(int tot_num_nodes){
    uniform_int_distribution<int> int_dist(0,tot_num_nodes-1);
    return int_dist(rand_gen);
}
