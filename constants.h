#pragma once
#include <stddef.h>
#include <point.hpp>
#include <vector>

using namespace std;

//#define RUN_TESTS

constexpr size_t WORLD_SIZE = 100;

constexpr size_t NUM_PEOPLE = 200;

constexpr size_t DJISTA_ITERS_AFTER_DEST_FOUND = WORLD_SIZE*WORLD_SIZE/16;

const int max_slide = 20;
const int max_time = 500;

inline int slide_time(int slide){
    //todo:change to proper logarithmic scale
    return (slide *  max_time) / max_slide;
}

using point_iter = PointIter<WORLD_SIZE,WORLD_SIZE>;
using point_iter_cont = PIterContainter<WORLD_SIZE,WORLD_SIZE>;

inline point_iter_cont iter_all(){
    return point_iter_cont(0,0,WORLD_SIZE,WORLD_SIZE);
}
inline point_iter_cont iter_around(Point cen,size_t maxdis){
    return point_iter_cont(cen,maxdis);
}
template<typename fn_ty>
inline void iter_around1(Point cen,fn_ty fn){
    if(cen.X > 0){
        fn(Point{cen.X-1,cen.Y});
    }
    if(cen.X < WORLD_SIZE-1){
        fn(Point{cen.X+1,cen.Y});
    }
    if(cen.Y > 0){
        fn(Point{cen.X,cen.Y-1});
    }
    if(cen.Y < WORLD_SIZE-1){
        fn(Point{cen.X,cen.Y+1});
    }
}

inline vector<Point> iter_around_1(Point cen){
    return vector<Point>{Point{max(cen.X-1,0),cen.Y},Point{min(cen.X+1,int32_t(WORLD_SIZE-1)),cen.Y},Point{cen.X,max(cen.Y-1,0)},Point{cen.X,min(cen.Y+1,int32_t(WORLD_SIZE-1))}};
}
template<class numty>
inline numty sqr(numty num){
    return num * num;
}
