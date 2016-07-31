#pragma once
#include <thread>
#include <vector>
using namespace std;


inline unsigned int num_threads(){
#ifdef EXEC_PARRELELL
    return thread::hardware_concurrency();
#else
    return 1;
#endif
}

template<typename func_ty>
void par_for_tid(size_t start,size_t end,func_ty func){
    //thread function created
    auto sequ = [&](size_t start,size_t end,size_t tid){
        for(size_t i = start; i < end; i++){
            func(i,tid);
        }
    };
#ifdef EXEC_PARRELELL
    size_t num_ts = num_threads();
    //creates space for threads without calling any constructors
    vector<thread> ts;
    ts.reserve(num_ts);
    

    //max size of each chunk
    size_t inc_val = (end-start)/num_ts;
    //spawns threads
    size_t curstart = start;
    for(size_t i = 0; i < num_ts; i++){
        size_t curend = min(curstart + inc_val,end);
        ts.emplace_back(sequ,curstart,curend,i);
        curstart = curend;
    }
    //joins threads for return
    for(size_t i = 0; i < num_ts; i++){
        ts[i].join();
    }
#else
    sequ(start,end,0);
#endif
}

template<typename func_ty>
void par_for(size_t start,size_t end,func_ty func){
    par_for_tid(start,end,[&](size_t bufidx,size_t tid){return func(bufidx);});
}
template<typename content,typename func_ty>
vector<content> accum_content(size_t size,func_ty func){
    vector<content> res(size);
    par_for(0,size,[&](size_t idx){
        res[idx] = func(idx);
    });
    return res;
}
