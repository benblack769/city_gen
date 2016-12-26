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

constexpr size_t UNDERLINGS_Ts[NUM_TIERS] = {1,TRANS_TIER_1_UNDERLINGS,TRANS_TIER_2_UNDERLINGS};
constexpr size_t SIZE_Ts[NUM_TIERS] = {1,TRANS_TIER_1_UNDERLINGS,TRANS_TIER_1_UNDERLINGS * TRANS_TIER_2_UNDERLINGS};
constexpr size_t NUM_Ts[NUM_TIERS] = {WORLD_SIZE / SIZE_Ts[0],WORLD_SIZE / SIZE_Ts[1],WORLD_SIZE / SIZE_Ts[2]};

inline float distance(Point a,Point b){
    return sqrt(sqr(a.X-b.X) + sqr(a.Y-b.Y));
}
static const move_cost_ty MAX_COST =1LL<<28;
inline move_cost_ty invest_to_speed(size_t invest){
    return (invest + 1);
}
inline move_cost_ty invest_to_time(size_t invest,float dis){
    return move_cost_ty(1.0)*dis / invest_to_speed(invest);
}
inline move_cost_ty time_dif_upgrade(move_cost_ty curtime,size_t cur_invest,float dis){
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
RangeArray<data_ty,array<data_ty,9>> make_ra_arr(Point start,Point pastend,int8_t tier){
    Point corner(max(start.X,0),max(start.Y,0));
    Point end(min(pastend.X,int32_t(NUM_Ts[tier])),min(pastend.Y,int32_t(NUM_Ts[tier])));
    Point size = end - corner;
    return RangeArray<data_ty,array<data_ty,9>>(corner,size.X,size.Y,array<data_ty,9>());
}
template<typename data_ty>
RangeArray<data_ty,array<data_ty,9>> make_ra_arr(Point cen,int8_t tier){
    Point rangep = Point(1,1);
    return make_ra_arr<data_ty>(cen-rangep,cen+rangep+Point(1,1),tier);
}
template<typename data_ty,typename cont_ty>
PIterContainter iter_scope(const RangeArray<data_ty,cont_ty> & ra){
    Point st = ra.Corner;
    Point end = ra.Corner+Point(ra.XSize,ra.YSize);
    return PIterContainter(st.X,st.Y,end.X,end.Y);
}
Node make_node(Point src,int8_t tier){
    Node node = make_ra_arr<Edge>(src,tier);
    iter_around8(src,tier,[&](Point dest){
        node[dest].dis = distance(base_cen(src,tier),base_cen(dest,tier));
    });
    return node;
}
tier_ty make_nodes(int8_t tier){
    size_t num_sqrs = NUM_Ts[tier];
    tier_ty tier_g(num_sqrs,num_sqrs);
    for(Point P : iter_all(num_sqrs)){
        tier_g[P] = make_node(P,tier);
    }
    return tier_g;
}
graph_ty make_graph(){
    return graph_ty{{make_nodes(0),make_nodes(1),make_nodes(2)}};
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
void set_move_speed(Point tspot,tier_ty & tier_g,int8_t tier){
    /*movecosts start;
    start[underling_cen_idx<tier>(tspot)] = 0;
    for(pair<size_t,move_cost_ty> pointval : upwards_moving_dists<tier>(tspot,start)){
        graph[tieridx<tier>(tspot)].get_edge(pointval.first).movecost = pointval.second;
    }*/
    iter_around8(tspot,tier,[&](Point dest){
        Edge & e = tier_g[tspot][dest];
        e.movecost = invest_to_time(e.invest,e.dis);
    });
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
void add_marginal_benefit(Point src,Point dest,graph_ty & graph,graph_ty & revgraph){
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
        int8_t tier = int64_t(NUM_TIERS) - 1 - (abs(int64_t(mi)-int64_t(bmi)))/2 - (5 - move_size)/2;
        
        movecosts f_mcs = forward_mcs[mi];
        movecosts b_mcs = backwards_mcs[move_size-1-mi];
        assert(SameScope(f_mcs,b_mcs));
        for(Point src : iter_scope(f_mcs)){
            Node & srcnode = graph[tier][src];
            iter_around8(src,tier,[&](Point dest){
                if(!b_mcs.IsInScope(dest)){
                    return;//break
                }
                move_cost_ty fval = f_mcs[src];
                move_cost_ty bval = b_mcs[dest];
                if(fval != MAX_COST && bval != MAX_COST){
                    Edge & e = srcnode[dest];
                    
                    move_cost_ty mv_thr_cst_wo_edge = fval + bval;
                    move_cost_ty bef_upg_cost = mv_thr_cst_wo_edge + e.movecost;
                    if(min_cost * 0.999 > bef_upg_cost){
                        cout << "  mincost_error! ";
                    }
                    //assert(mv_thr_cst_wo_edge + e.movecost >= min_cost && "min cost not smallest cost!");
                    
                    move_cost_ty upgraded_cost = time_dif_upgrade(e.movecost,e.invest,e.dis);
                    move_cost_ty new_cost = upgraded_cost + mv_thr_cst_wo_edge;
                    move_cost_ty gained_time = min_cost - new_cost;
                    if(gained_time > 0 && min_cost  * 0.999 <= bef_upg_cost){
                        e.marg_benefit_invest += gained_time;
                    }
                }
            });
        }
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

    auto add_point = [&](Point dest,Edge e,NodeVal prev){
        if(done.IsInScope(dest) && !done[dest]){
            move_cost_ty tot_val = prev.val+e.movecost;
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
    for(Point P : PIterContainter(0,0,test_size,test_size)){
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
void set_move_speeds(tier_ty & graph,int8_t tier){
    for(Point P : iter_all(NUM_Ts[tier])){
        set_move_speed(P,graph,tier);
    }
}

void set_move_costs(graph_ty & graph){
    for(int8_t tier : range(NUM_TIERS)){
        set_move_speeds(graph[tier],tier);
    }
}
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

void update_trans_invest(blocks & blks){
    
    set_move_costs(blks.graph);
    
    for(tier_ty & t : blks.graph){
        for(Node & n : t.Arr){
            for(Edge & e : n.Arr){
                e.marg_benefit_invest = 0;
            }
        }
    }
    
    graph_ty revgraph = reverse_graph(blks.graph);
    for(size_t pn : range(NUM_PEOPLE)){
        add_marginal_benefit(blks.pps.home[pn],blks.pps.work[pn],blks.graph,revgraph);
    }
    
    int64_t start = uclock();
    vector<Edge *> all_edges;
    all_edges.reserve(8*(sqr(NUM_Ts[0])+sqr(NUM_Ts[1])+sqr(NUM_Ts[2])));
    for(int8_t tier : range(blks.graph.size())){
        for(Node & n : blks.graph[tier].Arr){
            iter_around8(n.Corner+Point(1,1),tier,[&](Point dest){
                Edge & e = n[dest];
                all_edges.push_back(&e);
            });
        }
    }
    /*for(size_t tn : range(NUM_TIERS)){
        tier_ty & t = blks.graph[tn];
        for(Point src : PIterContainter(0,0,NUM_Ts[tn],NUM_Ts[tn])){
            Node & n = t[src];
        }
    }*/
    std::sort(all_edges.begin(),all_edges.end(),[](Edge * one,Edge * other){
        return one->marg_benefit_invest > other->marg_benefit_invest;
    });
    int64_t end = uclock() - start;
    cout << "sort: \t\t" << end << endl;
    //for any value of blks.inv_per_turn > 1 this corrupts the algoritm and makes
    //it produce unoptimal results, but it still does pretty well and also
    //goes that much faster
    for(size_t i = 0; i < blks.inv_per_turn;i++){
        all_edges[i]->invest++;
    }
}
template<typename fnty>
void add_view_tier(count_ty & view,tier_ty & graph,int8_t tier,fnty add_fn){
    for(Point src:iter_all(NUM_Ts[tier])){
        iter_around8(src,tier,[&](Point dest){
            Edge e = graph[src][dest];
            
            Point start = base_cen(src,tier);
            Point end = base_cen(dest,tier);
            Point inc = (end - start) / int32_t(SIZE_Ts[tier]);
            for(Point P = start;P != end; P += inc){
                view[P] += add_fn(e);
            }
        });
    }
}
template<typename fnty>
void set_view(count_ty & view,graph_ty & graph,fnty add_fn){
    view.assign(0);
    for(int8_t tier : range(NUM_TIERS)){
        add_view_tier(view,graph[tier],tier,add_fn);
    }  
}

void blocks::update_trans(){
    djiks_t = 0;
    int64_t start = uclock();
    update_trans_invest(*this);
    int64_t end = uclock() - start;
    cout << "dijik: \t\t" << djiks_t << endl;
    cout << "tot: \t\t" << end << endl;
    
    set_view(trans_invest_view,graph,[](Edge e){return e.invest;});
    set_view(upgrade_vs_view,graph,[](Edge e){return e.marg_benefit_invest * 1000;});
}
