#include "blocks.h"
#include <headerlib/RangeIterator.h>
#include <headerlib/range_array.hpp>
#include <ctime>
#include <random>
#include <iostream>
#include <queue>
#include <cmath>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include "parrelell.h"
#include "test.h"

struct pointcost{
    Point P;
    move_cost_ty cost;
};

using nodeset = vector<Point>;
using nodecost = PointInfo<move_cost_ty>;
using movecosts = RangeArray<move_cost_ty>;
using tiered_movecosts = vector<movecosts>;
using startcosts = vector<pointcost>;

constexpr int POINT_ADJAC_FACTOR = WORLD_SIZE*WORLD_SIZE;//if the point is a in an ajacent tile a tier above, then it looks a max distancce of this*UNDERLINGS_Ts[tier+1]
constexpr int POINT_TIER_CHANGE = 2;//if moving a tier above or below, then it looks a max distancce of this*UNDERLINGS_Ts[tier(+/0)1]

void djistras_algorithm(movecosts & output, startcosts & sources, nodeset & dests, tier_ty & graph, int8_t tier);

constexpr size_t UNDERLINGS_Ts[] = {1,TRANS_TIER_1_UNDERLINGS,TRANS_TIER_2_UNDERLINGS};
constexpr size_t SIZE_Ts[] = {1,TRANS_TIER_1_UNDERLINGS,TRANS_TIER_1_UNDERLINGS * TRANS_TIER_2_UNDERLINGS};
constexpr size_t NUM_Ts[] = {WORLD_SIZE / SIZE_Ts[0],WORLD_SIZE / SIZE_Ts[1],WORLD_SIZE / SIZE_Ts[2]};

inline float distance(Point a,Point b){
    return sqrt(sqr(a.X-b.X) + sqr(a.Y-b.Y));
}
static const move_cost_ty MAX_COST =1LL<<28;
inline uint32_t invest_to_speed(uint32_t invest){
    return (invest*1 + 1);//change this, change speed_to_inv!!!!!!!!
}
inline float speed_to_invest(float speed){
    return (speed - 1)/1;//change this, change invest_to_speed!!!!!!!!
}
inline move_cost_ty invest_to_time(uint32_t invest,float dis){
    return move_cost_ty(1.0)*dis / invest_to_speed(invest);
}
inline move_cost_ty time_dif_upgrade(move_cost_ty curtime,float dis){
    float aprox_speed = curtime/dis;
    uint32_t cur_invest = speed_to_invest(aprox_speed);
    return (curtime - (curtime * invest_to_speed(cur_invest)) / invest_to_speed(cur_invest+1))/dis;
}
Point underling_cen(Point tspot,int8_t tier){
    int32_t add_to = UNDERLINGS_Ts[tier] / 2;
    Point tblock = tspot * UNDERLINGS_Ts[tier];
    return tblock + Point{add_to,add_to};
}
Point overlord(Point tspot,int8_t tier){
    return tspot / UNDERLINGS_Ts[tier];
}
Point tier_rep(Point boardspot,int8_t tier){
    return boardspot / SIZE_Ts[tier];
}
Point base_cen(Point tspot,int8_t tier){
    int32_t add_to = SIZE_Ts[tier] / 2;
    Point tblock = tspot * SIZE_Ts[tier];
    return tblock + Point{add_to,add_to};
}

template<typename fnty>
void iter_around8(Point cen_p,int8_t tier,fnty itfn){
    for(Point P : iter_around(cen_p,1,NUM_Ts[tier])){
        if(P != cen_p){
            itfn(P);
        }
    }
}
void print_p(Point p){
    cout << p.X  << "\t" << p.Y<< endl;
}
bool is_in_bordering_tier_rep(Point a,Point b,int8_t tier){
    Point a_rep = tier_rep(a,tier);
    Point b_rep = tier_rep(b,tier);
    return abs(a_rep.X - b_rep.X) + abs(a_rep.Y - b_rep.Y) <= 2;
}

template<typename data_ty>
RangeArray<data_ty> make_ra_vec(Point start,Point pastend,int8_t tier){
    Point corner(max(start.X,0),max(start.Y,0));
    Point end(min(pastend.X,int32_t(NUM_Ts[tier])),min(pastend.Y,int32_t(NUM_Ts[tier])));
    Point size = end - corner;
    return RangeArray<data_ty>(corner,size.X,size.Y,vector<data_ty>(size.X*size.Y,data_ty()));
}
template<typename data_ty>
RangeArray<data_ty> make_ra_vec(Point cen,int range,int8_t tier){
    Point rangep = Point(range,range);
    return make_ra_vec<data_ty>(cen-rangep,cen+rangep+Point(1,1),tier);
}
template<typename data_ty>
SqrData<data_ty> make_ra_arr(Point start,Point pastend,int8_t tier){
    Point corner(max(start.X,0),max(start.Y,0));
    Point end(min(pastend.X,int32_t(NUM_Ts[tier])),min(pastend.Y,int32_t(NUM_Ts[tier])));
    Point size = end - corner;
    return SqrData<data_ty>(corner,size.X,size.Y,array<data_ty,9>());
}
template<typename data_ty>
SqrData<data_ty> make_ra_arr(Point cen,int8_t tier){
    Point rangep = Point(1,1);
    return make_ra_arr<data_ty>(cen-rangep,cen+rangep+Point(1,1),tier);
}
template<typename data_ty,typename cont_ty>
PIterContainter iter_scope(const RangeArray<data_ty,cont_ty> & ra){
    Point st = ra.Corner;
    Point end = ra.Corner+Point(ra.XSize,ra.YSize);
    return PIterContainter(st.X,st.Y,end.X,end.Y);
}
template<typename edge_ty,typename fn_ty>
SqrData<edge_ty> make_node(Point src,int8_t tier,fn_ty init_fn){
    SqrData<edge_ty> res = make_ra_arr<edge_ty>(src,tier);
    iter_around8(src,tier,[&](Point dest){
        res[dest] = init_fn(dest);
    });
    return res;
}
template<typename edge_ty,typename fn_ty>
tier_data<edge_ty> make_tier(int8_t tier,fn_ty init_fn){
    size_t tier_size = NUM_Ts[tier];
    tier_data<edge_ty> res(tier_size,tier_size);
    for(Point src : iter_all(tier_size)){
        res[src] = make_node<edge_ty>(src,tier,[&](Point dest){return init_fn(src,dest);});
    }
    return res;
}
board_inv_ty init_trans_inv(){
    return make_tier<uint32_t>(0,[](Point ,Point ){return 0;});
}
tier_ty board_costs(board_inv_ty & invest){
    return make_tier<move_cost_ty>(0,[&](Point src,Point dest){
        return invest_to_time(invest[src][dest],distance(src,dest));
    });
}

startcosts upwards_moving_dists(movecosts & outcosts,Point tspot,startcosts & tm1_starts,tier_ty & graph,int8_t tier){
    nodeset dests;
    iter_around8(tspot,tier,[&](Point P){
        dests.push_back(underling_cen(P,tier));
    });
    movecosts & tm1_dists = outcosts;
    djistras_algorithm(tm1_dists,tm1_starts,dests,graph,tier-1);

    startcosts tdists;
    iter_around8(tspot,tier,[&](Point P){
        tdists.push_back(pointcost{P,tm1_dists.at(underling_cen(P,tier))});
    });
    return tdists;
}
startcosts downwards_moving_dists(movecosts & outcosts,startcosts & tsrcs,Point tdest,tier_ty & graph,int8_t tier){
    nodeset dests;
    iter_around8(tdest,tier,[&](Point P){
        dests.push_back(P);
    });
    
    movecosts & tcosts = outcosts;
    djistras_algorithm(tcosts,tsrcs,dests,graph,tier);

    startcosts tm1_costs;
    iter_around8(tdest,tier,[&](Point P){
        tm1_costs.push_back(pointcost{underling_cen(P,tier),tcosts.at(P)});
    });
    return tm1_costs;
}

void all_down_moving_dists(Point dest,startcosts & srccosts,tiered_movecosts & accumvals,graph_ty & graph,int8_t tier){
    if(tier == 0){
        nodeset dests({dest});
        djistras_algorithm(accumvals.back(),srccosts,dests,graph[tier],tier);
    }else{
        startcosts downcosts = downwards_moving_dists(accumvals.back(),srccosts,tier_rep(dest,tier),graph[tier],tier);
        accumvals.push_back(make_ra_vec<move_cost_ty>(tier_rep(dest,tier-1),UNDERLINGS_Ts[tier]*POINT_TIER_CHANGE,tier-1));
        all_down_moving_dists(dest,downcosts,accumvals,graph,tier-1);
    }
}
void all_up_moving_dists(Point src,Point dest,startcosts & srccosts,tiered_movecosts & accumvals,graph_ty & graph,int8_t tier){
    if(tier == NUM_TIERS-1){
        accumvals.push_back(make_ra_vec<move_cost_ty>(Point(0,0),Point(1,1)*NUM_Ts[tier],tier));
        all_down_moving_dists(dest,srccosts,accumvals,graph,tier);
    }
    else if(is_in_bordering_tier_rep(src,dest,tier+1)){
        accumvals.push_back(make_ra_vec<move_cost_ty>(tier_rep(src,tier),POINT_ADJAC_FACTOR*UNDERLINGS_Ts[tier+1],tier));
        all_down_moving_dists(dest,srccosts,accumvals,graph,tier);
    }
    else{
        accumvals.push_back(make_ra_vec<move_cost_ty>(tier_rep(src,tier),UNDERLINGS_Ts[tier+1]*POINT_TIER_CHANGE,tier));
        startcosts tiercosts = upwards_moving_dists(accumvals.back(),tier_rep(src,tier+1),srccosts,graph[tier],tier+1);
        all_up_moving_dists(src,dest,tiercosts,accumvals,graph,tier+1);
    }
}
tiered_movecosts move_costs(Point src,Point dest,graph_ty & graph){
    tiered_movecosts mcs;

    startcosts start({pointcost{src,0}});
    all_up_moving_dists(src,dest,start,mcs,graph,0);
    return mcs;
}
movecosts move_out_costs(Point tsrc,tier_ty & prev_tier,int8_t tier){
    startcosts tm1srcs{{pointcost{underling_cen(tsrc,tier),0}}};
    nodeset tm1dests;
    iter_around8(tsrc,tier,[&](Point P){
        tm1dests.push_back(underling_cen(P,tier));
    });
    movecosts tm1costs = make_ra_vec<move_cost_ty>(underling_cen(tsrc,tier),UNDERLINGS_Ts[tier]*POINT_TIER_CHANGE,tier-1);
    djistras_algorithm(tm1costs,tm1srcs,tm1dests,prev_tier,tier-1);
    return tm1costs;
}
Node tier_costs(Point tsrc,tier_ty & prev_tier,int8_t tier){
    movecosts tm1costs = move_out_costs(tsrc,prev_tier,tier);
    return make_node<move_cost_ty>(tsrc,tier,[&](Point dest){return tm1costs[underling_cen(dest,tier)];});
}
tier_ty make_tier_costs(tier_ty & prev_tier,int8_t tier){
    size_t tier_s = NUM_Ts[tier];
    tier_ty res(tier_s,tier_s);
    for(Point src : iter_all(tier_s)){
        res[src] = tier_costs(src,prev_tier,tier);
    }
    return res;
}
graph_ty graph_costs(board_inv_ty & invest){
    graph_ty graph;
    graph[0] = board_costs(invest);
    for(int8_t tier : range(1,NUM_TIERS)){
        graph[tier] = make_tier_costs(graph[tier-1],tier);
    }
    return graph;
}
template<typename fn_ty>
void iterate_benefits(movecosts & fcosts,movecosts & bcosts,move_cost_ty min_cost,tier_ty & graph,int8_t tier,fn_ty mb_inv_fn){
    for(Point src : iter_scope(fcosts)){
        iter_around8(src,tier,[&](Point dest){
            if(!bcosts.IsInScope(dest)){
                return;//continue
            }
            move_cost_ty fval = fcosts[src];
            move_cost_ty bval = bcosts[dest];
            if(fval != MAX_COST && bval != MAX_COST){
                move_cost_ty edge_mc = graph[src][dest];
                
                move_cost_ty mv_thr_cst_wo_edge = fval + bval;
                move_cost_ty bef_upg_cost = mv_thr_cst_wo_edge + edge_mc;
                if(min_cost * 0.99 > bef_upg_cost){
                //    cout << "  mincost_error! ";
                }
                assert(bef_upg_cost >= min_cost*0.99 && "min cost not smallest cost!");
                
                move_cost_ty upgraded_cost = time_dif_upgrade(edge_mc,distance(base_cen(src,tier),base_cen(dest,tier)));
                move_cost_ty new_cost = upgraded_cost + mv_thr_cst_wo_edge;
                move_cost_ty gained_time = min_cost - new_cost;
                if(gained_time > 0 && min_cost  * 0.99 <= bef_upg_cost){
                    mb_inv_fn(src,dest,gained_time);
                }
            }
        });
    }
}

void move_mcs_down(Point tsrc,Point tdest,tier_ty & prevgraph,tier_ty & prevrevgraph,tier_ty & prevmarg_ben,tier_ty & tmarg_ben,int8_t tier){
    
    movecosts fcosts = move_out_costs(tsrc,prevgraph,tier);
    movecosts bcosts = move_out_costs(tdest,prevrevgraph,tier);
    
    Point tm1src = underling_cen(tsrc,tier);
    Point tm1dest = underling_cen(tdest,tier);
    assert(fcosts.IsInScope(tm1dest));
    assert(bcosts.IsInScope(tm1src));
    
    move_cost_ty min_fcost = fcosts[tm1dest];
    move_cost_ty min_bcost = bcosts[tm1src];
    assert(abs(min_fcost - min_bcost) < 0.1);//todo: make this more stable for higher numbers!!!!

    Point maxsrc,maxdest;
    move_cost_ty max_ben = 0;
    iterate_benefits(fcosts,bcosts,min_fcost,prevgraph,tier-1,[&](Point margsrc,Point margdest,move_cost_ty marg_ben){
        if(marg_ben > max_ben){
            max_ben = marg_ben;
            maxsrc = margsrc;
            maxdest = margdest;
        }
    });
    if(max_ben != 0){
        prevmarg_ben[maxsrc][maxdest] += tmarg_ben[tsrc][tdest];
    }
}
void move_mcs_down(graph_ty & graph,graph_ty & revgraph,graph_ty & marg_ben){
    for(int8_t tier = NUM_TIERS-1; tier >= 1; tier--){
        for(Point src : iter_all(NUM_Ts[tier])){
            iter_around8(src,tier,[&](Point dest){
                move_mcs_down(src,dest,graph[tier-1],revgraph[tier-1],marg_ben[tier-1],marg_ben[tier],tier);
            });
        }
    }
}

void add_marginal_benefit(Point src,Point dest,graph_ty & graph,graph_ty & revgraph,graph_ty & marg_ben_g,graph_data<uint32_t> & popcount){
    //revgraph is identical to graph, but the edge movecosts are swapped between the paired edges.
    tiered_movecosts forward_mcs = move_costs(src,dest,graph);
    tiered_movecosts backwards_mcs = move_costs(dest,src,revgraph);
    
    move_cost_ty min_cost = forward_mcs.back().at(dest);
    move_cost_ty min_cost2 = backwards_mcs.back().at(src);
    assert(abs(min_cost - min_cost2) < 0.1 && "going forward is not equal to going reverse backwards.");
    
    assert(forward_mcs.size() == backwards_mcs.size());
    const size_t move_size = forward_mcs.size();
    for(size_t mi = 0; mi < move_size; mi++){
        size_t bmi = move_size-1-mi;
        int8_t tier = int64_t(NUM_TIERS) - 1 - (abs(int64_t(mi)-int64_t(bmi)))/2 - (NUM_TIERS*2 - 1 - move_size)/2;
        
        movecosts & f_mcs = forward_mcs[mi];
        movecosts & b_mcs = backwards_mcs[move_size-1-mi];
        assert(SameScope(f_mcs,b_mcs));
        
        iterate_benefits(f_mcs,b_mcs,min_cost,graph[tier],tier,[&](Point src,Point dest,move_cost_ty marg_ben){
            marg_ben_g[tier][src][dest] += marg_ben;
            popcount[tier][src][dest]++;
        });
    }
}

struct NodeVal{
    move_cost_ty val;
    Point p;
};
/*
Point point_before(Point cenp,board<move_cost_ty> & move_costs){
    Point minp;
    move_cost_ty minv = MAX_COST*2;
    for(Point P : iter_around(cenp,1)){
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
}*/
int64_t djiks_t;
void djistras_algorithm(movecosts & output,startcosts & sources,nodeset & dests,tier_ty & graph,int8_t tier){
    int64_t time = uclock();
    //output is already constructed, here it just assigns default value (the "empty" marker)
    output.assign(MAX_COST);
    
    auto compare = [](const NodeVal & one,const NodeVal & other){
        return (one.val > other.val);
    };
    priority_queue<NodeVal,vector<NodeVal>,decltype(compare)> minheap(compare);
    RangeArray<bool> done(output.Corner,output.XSize,output.YSize,vector<bool>(output.size(),false));

    auto add_point = [&](Point dest,move_cost_ty mc,NodeVal prev){
        if(done.IsInScope(dest) && !done[dest]){
            move_cost_ty tot_val = prev.val + mc;
            minheap.push(NodeVal{tot_val,dest});
        }
    };
    //checks to see if dest is in scope
    for(Point dest : dests){
        assert(done.IsInScope(dest));
    }
    //sets start values
    for(auto source : sources){
        assert(output.IsInScope(source.P));
        minheap.push(NodeVal{source.cost,source.P});
    }
    //main algorithm
    for(size_t max_iter = done.size()*8; max_iter > 0 && minheap.size() > 0; max_iter--){
        NodeVal mintime = minheap.top();
        minheap.pop();
        
        if(!done[mintime.p]){//guarenteed to be in scope, because check happens when added to heap
            done[mintime.p] = true;
            output[mintime.p] = mintime.val;
            
            auto dest_iter = find(dests.begin(),dests.end(),mintime.p);
            if(dest_iter != dests.end()){
                dests.erase(dest_iter);
                if(dests.size() == 0){
                    //stops computation soon after dests are found
                    max_iter = min(max_iter,DJISTA_ITERS_AFTER_DEST_FOUND);
                }
            }
            Node & curn = graph[mintime.p];
            iter_around8(mintime.p,tier,[&](Point ep){
                add_point(ep,curn[ep],mintime);
            });
        }
    }
    djiks_t += uclock() - time;
    assert(dests.size() == 0 && "ran out of places to look for dest");
}
/*
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
    for(Point P : point_iter_cont(0,0,test_size,test_size)){
        in_data[P] = arr[P.Y][P.X];
    }
    board<move_cost_ty> dikstra_vals = djistras_algorithm(source,dest,in_data);
    vector<Point> min_path = make_path(dikstra_vals,source,dest);
    //for(Point p : min_path){
    //    cout << p.X << "\t" << p.Y << endl;
    //}
    return min_path.size() == correct_path.size() &&
            equal(min_path.begin(),min_path.end(),correct_path.begin());
}*/
void reverse_tier(tier_ty & rev_t,tier_ty & orig_t,int8_t tier){
    for(Point src : iter_all(NUM_Ts[tier])){
        iter_around8(src,tier,[&](Point dest){
            rev_t[dest][src] = orig_t[src][dest];
        });
    }
}

graph_ty reverse_graph(graph_ty & graph){
    graph_ty rev = graph;
    for(int8_t tier : range(NUM_TIERS)){
        reverse_tier(rev[tier],graph[tier],tier);        
    }
    return rev;
}
template<typename data_ty>
graph_data<data_ty> zero_graph(){
    graph_data<data_ty> graph;
    for(int8_t tier : range(NUM_TIERS)){
        graph[tier] = make_tier<data_ty>(tier,[](Point ,Point ){return 0;});
    }
    return graph;
}

void update_trans_invest(blocks & blks,graph_ty & marg_ben){
    
    graph_ty graph = graph_costs(blks.trans_invest);
    graph_ty revgraph = reverse_graph(graph);
    
    graph_data<uint32_t> popcount = zero_graph<uint32_t>();
    for(size_t pn : range(NUM_PEOPLE)){
        add_marginal_benefit(blks.pps.home[pn],blks.pps.work[pn],graph,revgraph,marg_ben,popcount);
    }
    move_mcs_down(graph,revgraph,marg_ben,popcount);
    
    struct edge_cost{
        Point src;
        Point dest;
        move_cost_ty mb;
    };

    int64_t start = uclock();
    vector<edge_cost> all_edges;
    all_edges.reserve(8*(sqr(NUM_Ts[0])));
    for(Point src : iter_all()){
        iter_around8(src,0,[&](Point dest){
            all_edges.push_back(edge_cost{src,dest,marg_ben[0][src][dest]});
        });
    }
    std::sort(all_edges.begin(),all_edges.end(),[](edge_cost one,edge_cost other){
        return one.mb > other.mb;
    });
    int64_t end = uclock() - start;
    cout << "sort: \t\t" << end << endl;
    //for any value of blks.inv_per_turn > 1 this corrupts the algoritm and makes
    //it produce unoptimal results, but it still does pretty well and also
    //goes that much faster
    for(size_t i = 0; i < blks.inv_per_turn;i++){
        edge_cost ec = all_edges[i];
        blks.trans_invest[ec.src][ec.dest]++;
    }
}
template<typename data_ty>
void set_view(count_ty & view,tier_data<data_ty> & data){
    for(Point src : iter_all()){
        iter_around8(src,0,[&](Point dest){
            view[src] += data[src][dest]*1000;
        });
    }
}

void blocks::update_trans(){
    djiks_t = 0;
    int64_t start = uclock();
    graph_ty marg_ben = zero_graph<move_cost_ty>();
    update_trans_invest(*this,marg_ben);
    int64_t end = uclock() - start;
    cout << "dijik: \t\t" << djiks_t << endl;
    cout << "tot: \t\t" << end << endl;
    
    set_view(trans_invest_view,this->trans_invest);
    set_view(upgrade_vs_view,marg_ben[0]);
}
