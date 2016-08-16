#include "blocks.h"
#include "RangeIterator.h"
#include <ctime>
#include <random>
#include <iostream>
#include <queue>
#include <cmath>
#include "parrelell.h"
#include "test.h"

decltype(Point::X) dist(Point a,Point b){
    return abs(a.X-b.X) + abs(a.Y-b.Y);
}
static const move_cost_ty MAX_COST = 1LL<<28;
size_t invest_to_speed(size_t invest){
    return (invest + 1);
}
move_cost_ty invest_to_time(size_t invest){
    return move_cost_ty(1LL) / invest_to_speed(invest);
}
move_cost_ty time_dif_upgrade(move_cost_ty curtime,size_t cur_invest){
    return curtime - (curtime * invest_to_speed(cur_invest)) / invest_to_speed(cur_invest+1);
}
mcarr i_to_s_arr(count_ty & invests){
    mcarr res;
    for(size_t i : range(blocks::arrsize())){
        res.Arr[i] = invest_to_time(invests.Arr[i]);
    }
    return res;
}


struct PointVal{
    move_cost_ty val;
    Point p;
    bool operator < (const PointVal & other){
        return val > other.val;
    }
    bool operator > (const PointVal & other){
        return !(*this < other);
    }
    
};

bool operator == (const PointVal & one,const PointVal & other){
    return one.val == other.val;
}
std::ostream &operator<<(std::ostream &os, PointVal const &m) { 
    return os << m.val;
}

namespace std{
    template<>
    struct hash<PointVal>{
        size_t operator()(const PointVal & pv)const{
            uint64_t pint = *reinterpret_cast<const uint64_t *>(&pv.p);
            return pint;
        }
    };
}
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
bool djistra_test(){
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
    for(Point P : PIterContainter(0,0,test_size,test_size)){
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
board<move_cost_ty> get_upgrade_vals(board<move_cost_ty> & mcto,board<move_cost_ty> & mcfrom,move_cost_ty min_val,mcarr & movecosts,count_ty & investment,Point home,Point work){
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
void add_to(board<move_cost_ty> & to,const board<move_cost_ty> & add){
    for(size_t i = 0; i < to.size();i++){
        to.Arr[i] += add.Arr[i];
    }
}
void add_to_useage(count_ty & useage,const vector<Point> & path){
    for(Point p : path){
        useage[p]++;
    }
}
board<move_cost_ty> update_trans_usage(blocks & blks){
    mcarr mc_arr = i_to_s_arr(blks.trans_invest);
    vector<count_ty> trans_usages(num_threads());
    vector<board<move_cost_ty>> thread_upgrades(num_threads());
    par_for_tid(0,blks.pps.size(),[&](size_t pn,size_t tid){
        Point myhome = blks.pps.home[pn];
        Point mywork = blks.pps.work[pn];
        board<move_cost_ty> move_to_costs = djistras_algorithm(myhome,mywork,mc_arr);
        board<move_cost_ty> move_from_costs = djistras_algorithm(mywork,myhome,mc_arr);
        move_cost_ty min_cost = move_to_costs[mywork]+move_from_costs[mywork]-mc_arr[mywork];
        add_to(thread_upgrades[tid],get_upgrade_vals(move_to_costs,move_from_costs,min_cost,mc_arr,blks.trans_invest,myhome,mywork));
        add_to_useage(trans_usages[tid],make_path(move_to_costs,myhome,mywork));
    });
    blks.trans_usage.assign(0);
    for(count_ty & usage : trans_usages){
        for(Point p : iter_all()){
            blks.trans_usage[p] += usage[p];
        }
    }
    board<move_cost_ty> upgrade_vs(0);
    for(board<move_cost_ty> & up_vs : thread_upgrades){
        add_to(upgrade_vs,up_vs);
    }
    
    return upgrade_vs;
}
void update_trans_invest(blocks & blks,board<move_cost_ty> & upgrade_vs){
    vector<PointVal> movecosts(blocks::arrsize());
    for(Point P : iter_all()){
        movecosts[to_idx(P)] = PointVal{upgrade_vs[P],P};
    }
    std::sort(movecosts.begin(),movecosts.end(),[](PointVal one,PointVal other){
        return one.val > other.val;
    });
    //for any value of blks.inv_per_turn this corrupts the algoritm and makes
    //it produce unoptimal results, but it still does pretty well and also
    //goes that much faster
    for(size_t i = 0; i < blks.inv_per_turn;i++){
        blks.trans_invest[movecosts[i].p]++;
    }
}
void blocks::update_trans(){
    board<move_cost_ty> upgrade_vs = update_trans_usage(*this);
    for(size_t i = 0; i < arrsize(); i++){
        size_t_upgrade_vs.Arr[i] = upgrade_vs.Arr[i] * 1000;
    }
    update_trans_invest(*this,upgrade_vs);
}
