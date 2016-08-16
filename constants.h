#pragma once
#include <stddef.h>
#include <point.hpp>
#include <vector>
#include <ctime>

using namespace std;

//#define RUN_TESTS
#define EXEC_PARRELELL

constexpr size_t WORLD_SIZE = 200;

constexpr size_t NUM_PEOPLE = 100;
constexpr size_t HOME_WORK_MAX_DIS = 100000;

constexpr size_t DJISTA_ITERS_AFTER_DEST_FOUND = 1;//WORLD_SIZE;//WORLD_SIZE*WORLD_SIZE/16;

const int max_slide = 20;
const int max_time = 500;

inline int slide_time(int slide){
    //todo:change to proper logarithmic scale
    return (slide *  max_time) / max_slide;
}

inline PIterContainter iter_all(){
    return PIterContainter(0,0,WORLD_SIZE,WORLD_SIZE);
}
inline PIterContainter iter_rect(int xs,int ys,int xe,int ye){
    return PIterContainter(max(0,xs),max(0,ys),min(xe,int(WORLD_SIZE)),min(ye,int(WORLD_SIZE)));
}
inline PIterContainter iter_around(Point cen,int32_t maxdis){
    return iter_rect(cen.X-maxdis,cen.Y-maxdis,cen.X+maxdis+1,cen.Y+maxdis+1);
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
inline size_t to_idx(Point P){
    return P.Y * WORLD_SIZE + P.X;
}

inline vector<Point> iter_around_1(Point cen){
    return vector<Point>{Point{max(cen.X-1,0),cen.Y},Point{min(cen.X+1,int32_t(WORLD_SIZE-1)),cen.Y},Point{cen.X,max(cen.Y-1,0)},Point{cen.X,min(cen.Y+1,int32_t(WORLD_SIZE-1))}};
}
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
inline double time_to_sec(int64_t time){
    return time / 1000000000.0;
}
