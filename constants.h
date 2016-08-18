#pragma once
#include <stddef.h>
#include <headerlib/point.hpp>
#include <vector>
#include <ctime>

using namespace std;

//#define RUN_TESTS
//#define EXEC_PARRELELL

constexpr size_t NUM_TIERS = 3;//implicitly there is at least 1 tier
constexpr size_t TRANS_TIER_1_UNDERLINGS = 7;
constexpr size_t TRANS_TIER_2_UNDERLINGS = 7;
constexpr size_t WORLD_SIZE = TRANS_TIER_2_UNDERLINGS*TRANS_TIER_1_UNDERLINGS*12;

static_assert(TRANS_TIER_1_UNDERLINGS%2 == 1,"TRANS_TIER_1_UNDERLINGS must be odd");
static_assert(TRANS_TIER_2_UNDERLINGS%2 == 1,"TRANS_TIER_2_UNDERLINGS must be odd");
static_assert(WORLD_SIZE%(TRANS_TIER_1_UNDERLINGS*TRANS_TIER_2_UNDERLINGS) == 0,"WORLD_SIZE must a multiple of the tier underlings");

constexpr size_t NUM_PEOPLE = 200;
constexpr size_t HOME_WORK_MAX_DIS = 10000;


constexpr size_t DJISTA_ITERS_AFTER_DEST_FOUND = 1;//WORLD_SIZE;//WORLD_SIZE*WORLD_SIZE/16;

const int max_slide = 20;
const int max_time = 500;

inline int slide_time(int slide){
    //todo:change to proper logarithmic scale
    return (slide *  max_time) / max_slide;
}

template<size_t size=WORLD_SIZE>
inline PIterContainter iter_all(){
    return PIterContainter(0,0,size,size);
}
template<size_t size=WORLD_SIZE>
inline PIterContainter iter_rect(int startx,int starty,int endx,int endy){
    return PIterContainter(max(0,startx),max(0,starty),min(int(size),endx),min(int(size),endy));
}
template<size_t size=WORLD_SIZE>
inline PIterContainter iter_around(Point cen,int32_t maxdis){
    return iter_rect<size>(cen.X-maxdis,cen.Y-maxdis,cen.X+maxdis+1,cen.Y+maxdis+1);
}
template<size_t size=WORLD_SIZE>
inline PIterContainter iter_square(Point start,int32_t sqr_size){
    return iter_rect<size>(start.X,start.Y,start.X + sqr_size,start.Y + sqr_size);
}
template<size_t size=WORLD_SIZE>
inline PIterContainter iter_square(int32_t sqr_size){
    return iter_square<size>(Point(0,0),sqr_size);
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
