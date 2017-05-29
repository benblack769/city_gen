#pragma once
#include <stddef.h>
#include <headerlib/point.hpp>
#include <vector>
#include <ctime>
#include <headerlib/Array2d.hpp>
#include <headerlib/RangeIterator.h>
#include <random>
#include <cassert>
#include <algorithm>

using namespace std;

//#define RUN_TESTS
//#define EXEC_PARRELELL

constexpr size_t WORLD_SIZE = 200;

constexpr size_t NUM_PEOPLE = 100;

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

template<typename obj_ty>
class board:
        public FArray2d<obj_ty>{
public:
    board(obj_ty initval=obj_ty()):
        FArray2d<obj_ty>(WORLD_SIZE,WORLD_SIZE,initval){
    }
};

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

extern default_random_engine seed_gen;

template<class generator_ty>
inline double urand(generator_ty & gen){
    uniform_real_distribution<double> dist(0,1);
    return dist(gen);
}
template<class container_ty,class val_ty,class reduce_fn_ty>
inline val_ty reduce(container_ty container,val_ty init,reduce_fn_ty reduce_fn){
    val_ty res = init;
    for(auto & v : container){
        res = reduce_fn(res,v);
    }
    return res;
}
template<class container_ty,class convert_fn_ty>
inline auto max_of(container_ty container,convert_fn_ty convert_fn)->pair<decltype(convert_fn()),decltype(*container_ty::begin())>{
    using val_ty = decltype(*container_ty::begin());
    using num_ty = decltype(convert_fn());
    if(!(container.begin() != container.end())){
        return make_pair(num_ty(),val_ty());
    }
    else{
        auto citer = container.begin();
        val_ty res_item = *citer;
        num_ty res_val = convert_fn(res_item);
        ++citer;
        for_each(citer,container.end(),[&](val_ty item){
            num_ty val = convert_fn(item);
            if(res_val < val){
                res_val = val;
                res_item = item;
            }
        });
        return make_pair(res_val,res_item);
    }
}

template<class container_ty,class convert_fn_ty>
inline auto sum_of(container_ty container,convert_fn_ty convert_fn)->decltype(convert_fn(*container.begin())){
    using num_ty = decltype(convert_fn(*container.begin()));
    auto res = num_ty();
    for(auto item : container){
        res += convert_fn(item);
    }
    return res;
}
template<class container_ty,class weight_fn_ty>
inline auto weighted_random_choice(container_ty container,weight_fn_ty weight_fn)->decltype(*container.begin()){
    using item_ty = decltype(*container.begin());
    assert(container.begin() != container.end() && "cannot choose anything in an empty container!");
    
    double tot_weight = sum_of(container,[&](item_ty item){return exp(double(weight_fn(item)));});
    
    default_random_engine gen(seed_gen());
    
    for(auto item : container){
        double my_weight = exp(weight_fn(item));
        double prob = my_weight/tot_weight;
        
        double randv = urand(gen);
        if(randv < prob){
            return item;
        }
        
        tot_weight -= my_weight;
    }
    assert(false && "reached condition that is supposed to happen with zero probabilty");
    return *container.begin();
}
