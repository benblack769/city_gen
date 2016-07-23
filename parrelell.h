#pragma once
#include <thread>
#include <vector>
using namespace std;

template<typename func_ty>
void par_for(size_t start,size_t end,func_ty func){
    size_t num_ts = thread::hardware_concurrency();
    //creates space for threads without calling any constructors
    vector<thread> ts;
    ts.reserve(num_ts);
    
    //thread function created
    auto sequ = [&](size_t start,size_t end){
        for(size_t i = start; i < end; i++){
            func(i);
        }
    };

    //max size of each chunk
    size_t inc_val = (end-start)/num_ts;
    //spawns threads
    size_t curstart = start;
    for(size_t i = 0; i < num_ts; i++){
        size_t curend = min(curstart + inc_val,end);
        ts.emplace_back(sequ,curstart,curend);
        curstart = curend;
    }
    //joins threads for return
    for(size_t i = 0; i < num_ts; i++){
        ts[i].join();
    }
}
template<typename content,typename func_ty>
vector<content> accum_content(size_t size,func_ty func){
    vector<content> res(size);
    par_for(0,size,[&](size_t idx){
        res[idx] = func(idx);
    });
    return res;
}
