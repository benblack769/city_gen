#include "blocks.h"
#include <headerlib/RangeIterator.h>
#include <ctime>
#include <random>
#include <iostream>
#include <queue>
#include <cmath>
#include "parrelell.h"
#include "test.h"

void init_info(people & pps);

blocks::blocks():
    trans_invest(init_trans_inv())
{
    init_info(pps);
}
Point rand_p(default_random_engine & eng){
    uniform_int_distribution<int32_t> dist(0,WORLD_SIZE-1);
    return Point{dist(eng),dist(eng)};
}
void init_info(people & pps){
    default_random_engine eng(1);//uclock());
    for(size_t i = 0; i < NUM_PEOPLE; i++){
        Point home = rand_p(eng);
        uniform_int_distribution<int32_t> Xdist(max(0,home.X-int32_t(HOME_WORK_MAX_DIS)),min(int32_t(WORLD_SIZE-1),int32_t(home.X+HOME_WORK_MAX_DIS)));
        uniform_int_distribution<int32_t> Ydist(max(0,home.Y-int32_t(HOME_WORK_MAX_DIS)),min(int32_t(WORLD_SIZE-1),int32_t(home.Y+HOME_WORK_MAX_DIS)));
        Point work(Xdist(eng),Ydist(eng));
        pps.add_person(home,work);
    }
}
void blocks::update_block_info(){
    num_residents.assign(0);
    num_workers.assign(0);
    for(size_t pn : range(pps.size())){
        num_residents[pps.home[pn]]++;
        num_workers[pps.work[pn]]++;
    }
}
void blocks::update(){
    update_trans();
    update_block_info();
}
