#include "blocks.h"
#include "RangeIterator.h"
#include <ctime>
#include <random>
#include <iostream>
#include <queue>
#include <cmath>
#include "parrelell.h"
#include "test.h"

void init_info(people & pps);

blocks::blocks()
{
    init_info(pps);
    trans_invest.assign(0);
}
decltype(Point::X) dist(Point a,Point b){
    return abs(a.X-b.X) + abs(a.Y-b.Y);
}
Point rand_p(default_random_engine & eng){
    uniform_int_distribution<int32_t> dist(0,WORLD_SIZE-1);
    return Point{dist(eng),dist(eng)};
}

void init_info(people & pps){
    default_random_engine eng(uclock());
    for(int i = 0; i < NUM_PEOPLE; i++){
        Point home = rand_p(eng);
        uniform_int_distribution<int32_t> Xdist(max(0,home.X-int32_t(HOME_WORK_MAX_DIS)),min(int32_t(WORLD_SIZE-1),int32_t(home.X+HOME_WORK_MAX_DIS)));
        uniform_int_distribution<int32_t> Ydist(max(0,home.Y-int32_t(HOME_WORK_MAX_DIS)),min(int32_t(WORLD_SIZE-1),int32_t(home.Y+HOME_WORK_MAX_DIS)));
        Point work(Xdist(eng),Ydist(eng));
        pps.add_person(home,work);
    }
}
using move_cost_ty = int32_t;
static const move_cost_ty MAX_COST = 1LL<<28;
using mcarr = board<move_cost_ty>;
size_t invest_to_speed(size_t invest){
    return (invest + 1);
}
move_cost_ty invest_to_time(size_t invest){
    return move_cost_ty(1LL <<16) / invest_to_speed(invest);
}
move_cost_ty time_dif_upgrade(move_cost_ty curtime,size_t cur_invest){
    return curtime - (curtime * invest_to_speed(cur_invest)) / invest_to_speed(cur_invest+1);
}
mcarr i_to_s_arr(blocks::count_ty & invests){
    mcarr res;
    for(size_t i : range(blocks::arrsize())){
        res.Arr[i] = invest_to_time(invests.Arr[i]);
    }
    return res;
}
struct PointVal{
    move_cost_ty val;
    Point p;
    bool operator < (PointVal other)const{
        return this->val > other.val;
    }
};
Point point_before(Point cenp,board<move_cost_ty> & move_costs){
    Point minp;
    move_cost_ty minv = MAX_COST*2;
    for(Point P : iter_around_1(cenp)){
        move_cost_ty curv = move_costs[P];
        if(curv < minv){
            minv = curv;
            minp = P;
        }
    }
    return minp;
}

vector<Point> make_path(board<move_cost_ty> & move_costs,Point start,Point end){
    vector<Point> res;
    Point p = end;
    res.push_back(p);
    while(p != start){
        p = point_before(p,move_costs);
        res.push_back(p);
    }
    return vector<Point>(res.rbegin(),res.rend());
}
board<move_cost_ty> djistras_algorithm(Point source,Point dest,mcarr & mcs){
    auto compare = [](const PointVal & one,const PointVal & other){
        return (one.val > other.val);
    };
    priority_queue<PointVal,vector<PointVal>,decltype(compare)> minheap(compare);
    board<uint8_t> done(false);
    board<move_cost_ty> move_to_val(MAX_COST);
	
    auto add_point = [&](Point p,PointVal prev){
        if(!done[p]){
            move_cost_ty tot_val = prev.val+mcs[p];
            
            move_to_val[p] = tot_val;
            minheap.push(PointVal{tot_val,p});
            
            done[p] = true;
        }
    };
    
    add_point(source,PointVal{0,source});
    for(size_t max_iter = blocks::arrsize(); max_iter > 0 && minheap.size() > 0; max_iter--){
        PointVal mintime = minheap.top();
        minheap.pop();
        iter_around1(mintime.p,[&](Point p){
            add_point(p,mintime);
            if(p == dest){
                //stops computation soon after dest is found
                max_iter = min(max_iter,DJISTA_ITERS_AFTER_DEST_FOUND);
            }
        });
    }
    return move_to_val;
}
TEST(djistra_test){
    constexpr size_t test_size = 8; 
    Point source(1,1);
    Point dest(5,5);
    vector<Point> correct_path({Point(1,1),Point(2,1),Point(3,1),Point(4,1),Point(4,2),Point(4,3),Point(5,3),Point(5,4),Point(5,5)});
    move_cost_ty arr[test_size][test_size] = 
    {
        {5,5,5,5,5,5,5,5},
        {5,5,2,2,2,2,2,2},
        {5,5,7,7,3,5,5,2},
        {5,5,5,2,2,2,5,2},
        {5,5,5,2,5,2,5,2},
        {5,5,5,2,5,1,5,2},
        {5,5,5,2,5,5,5,2},
        {5,5,5,2,2,2,2,2}
    };
    mcarr in_data(10000000);
    for(Point P : point_iter_cont(0,0,test_size,test_size)){
        in_data[P] = arr[P.Y][P.X];
    }
    board<move_cost_ty> dikstra_vals = djistras_algorithm(source,dest,in_data);
    vector<Point> min_path = make_path(dikstra_vals,source,dest);
    /*for(Point p : min_path){
        cout << p.X << "\t" << p.Y << endl;
    }*/
    return min_path.size() == correct_path.size() &&
            equal(min_path.begin(),min_path.end(),correct_path.begin());    
}
board<move_cost_ty> get_upgrade_vals(board<move_cost_ty> & mcto,board<move_cost_ty> & mcfrom,move_cost_ty min_val,mcarr & movecosts,blocks::count_ty & investment,Point home,Point work){
    board<move_cost_ty> upgrade_vals(0);
    for(Point P : iter_all()){
        if(mcto[P] != MAX_COST && mcfrom[P] != MAX_COST){
            move_cost_ty move_through_val = mcto[P] + mcfrom[P] - movecosts[P];
            move_cost_ty shorted_time = time_dif_upgrade(movecosts[P],investment[P]);
            move_cost_ty upgraded_mtv = move_through_val - shorted_time;
            move_cost_ty gained_time = min_val - upgraded_mtv;
            upgrade_vals[P] = max(gained_time,move_cost_ty(0));
        }
    }
    return upgrade_vals;
}
void add_to(board<move_cost_ty> & to,board<move_cost_ty> & add){
    for(size_t i = 0; i < to.size();i++){
        to.Arr[i] += add.Arr[i];
    }
}

board<move_cost_ty> update_trans_usage(blocks & blks){
    blks.trans_usage.assign(0);
    mcarr mc_arr = i_to_s_arr(blks.trans_invest);
    static vector<vector<Point>> all_paths(NUM_PEOPLE);
    static vector<board<move_cost_ty>> all_upgrades(NUM_PEOPLE);
    par_for(0,blks.pps.size(),[&](size_t pn){
        Point myhome = blks.pps.home[pn];
        Point mywork = blks.pps.work[pn];
        board<move_cost_ty> move_to_costs = djistras_algorithm(myhome,mywork,mc_arr);
        board<move_cost_ty> move_from_costs = djistras_algorithm(mywork,myhome,mc_arr);
        move_cost_ty min_cost = move_to_costs[mywork]+move_from_costs[mywork]-mc_arr[mywork];
        all_upgrades[pn] = get_upgrade_vals(move_to_costs,move_from_costs,min_cost,mc_arr,blks.trans_invest,myhome,mywork);
        all_paths[pn] = make_path(move_to_costs,myhome,mywork);
    });    
    for(vector<Point> & path : all_paths){
        for(Point & p : path){
            blks.trans_usage[p]++;
        }
    }
    board<move_cost_ty> upgrade_vs(0);
    for(board<move_cost_ty> & up_vs : all_upgrades){
        add_to(upgrade_vs,up_vs);
    }
    
    return upgrade_vs;
}
void update_trans_invest(blocks & blks,board<move_cost_ty> & upgrade_vs){
    for(size_t inv: range(blks.inv_per_turn)){
        move_cost_ty max_benefit = 0;
        Point maxp;
        for(Point P : iter_all()){
            move_cost_ty cur_ben = upgrade_vs[P];
            if(max_benefit < cur_ben){
                max_benefit = cur_ben;
                maxp = P;
            }
        }
        if(max_benefit != 0){
            blks.trans_invest[maxp]++;
        }
        else{
            cout << "no benefit!"<<  endl;
        }
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
    board<move_cost_ty> upgrade_vs = update_trans_usage(*this);
    for(size_t i = 0; i < arrsize(); i++){
        size_t_upgrade_vs.Arr[i] = upgrade_vs.Arr[i] * 1000;
    }
    update_trans_invest(*this,upgrade_vs);
    update_block_info();
}
