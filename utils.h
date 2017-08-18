#pragma once
#include <stddef.h>
#include <vector>
#include <ctime>
#include <random>
#include <cassert>
#include <algorithm>
#include <type_traits>
#include <ctime>

#include <headerlib/point.hpp>
#include <headerlib/Array2d.hpp>
#include <headerlib/RangeIterator.h>

using namespace std;

using vecfloat = vector<float>;

template<class numty>
inline numty sqr(numty num){
    return num * num;
}


inline int64_t uclock() {
#ifdef __linux__
    struct timespec te;
    clock_gettime(CLOCK_REALTIME,&te); // get current time
    return te.tv_sec*1000000000LL + te.tv_nsec;
#else
    return clock()*1000000LL;
#endif
}
#define default_random_engine mt19937_64

extern default_random_engine seed_gen;

template<class generator_ty>
inline double urand(generator_ty & gen){
    uniform_real_distribution<double> dist(0,1);
    return dist(gen);
}

