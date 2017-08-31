#pragma once
#include "utils.h"
//#define RUN_TESTS
//#define EXEC_PARRELELL

constexpr size_t WORLD_SIZE = 20;

constexpr size_t NUM_PEOPLE = 50;

template<typename obj_ty>
class board:
        public FArray2d<obj_ty>{
public:
    board(obj_ty initval=obj_ty()):
        FArray2d<obj_ty>(WORLD_SIZE,WORLD_SIZE,initval){
    }
};
inline double time_to_sec(int64_t time){
    return time / 1000000000.0;
}

inline PIterContainter iter_all(size_t max_size=WORLD_SIZE){
    return PIterContainter(0,0,max_size,max_size);
}
inline PIterContainter iter_rect(int startx,int starty,int endx,int endy,size_t max_size=WORLD_SIZE){
    return PIterContainter(max(0,startx),max(0,starty),min(int(max_size),endx),min(int(max_size),endy));
}
inline PIterContainter iter_around(Point cen,int32_t maxdis,size_t max_size=WORLD_SIZE){
    return iter_rect(cen.X-maxdis,cen.Y-maxdis,cen.X+maxdis+1,cen.Y+maxdis+1,max_size);
}
inline PIterContainter iter_square(Point start,int32_t sqr_size,size_t max_size=WORLD_SIZE){
    return iter_rect(start.X,start.Y,start.X + sqr_size,start.Y + sqr_size,max_size);
}
inline PIterContainter iter_square(int32_t sqr_size,size_t max_size=WORLD_SIZE){
    return iter_square(Point(0,0),sqr_size,max_size);
}

template<typename fn_ty>
inline void iter_around1(Point cen,fn_ty fn){
    for(int y = max(0,cen.Y-1); y <= min(int(WORLD_SIZE-1),cen.Y+1); y++){
        for(int x = max(0,cen.X-1); x <= min(int(WORLD_SIZE-1),cen.X+1); x++){
            fn(Point{x,y});
        }
    }
}
template<typename fn_ty>
inline void iter_around1_idx(Point cen,fn_ty fn){
    int idx = 0;
    iter_around1(cen,[&](Point p){
        fn(p,idx);
        idx++;
    });
}
