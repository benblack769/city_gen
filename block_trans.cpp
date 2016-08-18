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

constexpr int POINT_ADJAC_FACTOR = WORLD_SIZE;//if the point is a in an ajacent tile a tier above, then it looks a max distancce of this*UNDERLINGS_Ts[tier+1]
constexpr int POINT_TIER_CHANGE = 2;//if moving a tier above or below, then it looks a max distancce of this*UNDERLINGS_Ts[tier(+/0)1]

void djistras_algorithm(movecosts & output,startcosts & sources,nodeset & dests,tier_ty & graph);

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

template<int8_t tier>
Point underling_cen(Point tspot){
    int32_t add_to = UNDERLINGS_Ts[tier] / 2;
    Point tblock = tspot * UNDERLINGS_Ts[tier];
    return tblock + Point{add_to,add_to};
}
template<int8_t tier>
Point overlord(Point tspot){
    return tspot / UNDERLINGS_Ts[tier];
}
template<int8_t tier>
Point tier_rep(Point boardspot){
    return boardspot / SIZE_Ts[tier];
}
template<int8_t tier>
Point base_cen(Point tspot){
    int32_t add_to = SIZE_Ts[tier] / 2;
    Point tblock = tspot * SIZE_Ts[tier];
    return tblock + Point{add_to,add_to};
}

template<int8_t tier,typename fnty>
void iter_around8(Point cen_p,fnty itfn){
    for(Point P : iter_around<NUM_Ts[tier]>(cen_p,1)){
        if(P != cen_p){
            itfn(P);
        }
    }
}
void print_p(Point p){
    cout << p.X  << "\t" << p.Y<< endl;
}

template<int8_t tier>
bool is_in_bordering_tier_rep(Point a,Point b){
    Point a_rep = tier_rep<tier>(a);
    Point b_rep = tier_rep<tier>(b);
    return abs(a_rep.X - b_rep.X) + abs(a_rep.Y - b_rep.Y) <= 2;
}

template<int8_t tier,typename data_ty>
RangeArray<data_ty> make_ra(Point start,Point pastend){
    Point corner(max(start.X,0),max(start.Y,0));
    Point end(min(pastend.X,int32_t(NUM_Ts[tier])),min(pastend.Y,int32_t(NUM_Ts[tier])));
    Point size = end - corner;
    return RangeArray<data_ty>(corner,size.X,size.Y);
}
template<int8_t tier,typename data_ty>
RangeArray<data_ty> make_ra(Point cen,int range){
    Point rangep = Point(range,range);
    return make_ra<tier,data_ty>(cen-rangep,cen+rangep+Point(1,1));
}
template<typename data_ty>
PIterContainter iter_scope(const RangeArray<data_ty> & ra){
    Point st = ra.Corner;
    Point end = ra.Corner+Point(ra.XSize,ra.YSize);
    return PIterContainter(st.X,st.Y,end.X,end.Y);
}
template<int8_t tier>
Node make_node(Point src){
    Node node = make_ra<tier,Edge>(src,1);
    iter_around8<tier>(src,[&](Point dest){
        node[dest].dis = distance(base_cen<tier>(src),base_cen<tier>(dest));
    });
    return node;
}
template<int8_t tier>
tier_ty make_nodes(){
    constexpr size_t num_sqrs = NUM_Ts[tier];
    tier_ty tier_g(num_sqrs,num_sqrs);
    for(Point P : iter_all<num_sqrs>()){
        tier_g[P] = make_node<tier>(P);
    }
    return tier_g;
}
graph_ty make_graph(){
    return graph_ty{{make_nodes<0>(),make_nodes<1>(),make_nodes<2>()}};
}
template<int8_t tier>
startcosts upwards_moving_dists(movecosts & outcosts,Point tspot,startcosts & tm1_starts,tier_ty & graph){
    nodeset dests;
    iter_around8<tier>(tspot,[&](Point P){
        dests.push_back(underling_cen<tier>(P));
    });
    movecosts & tm1_dists = outcosts;
    djistras_algorithm(tm1_dists,tm1_starts,dests,graph);

    startcosts tdists;
    iter_around8<tier>(tspot,[&](Point P){
        tdists.push_back(pointcost{P,tm1_dists.at(underling_cen<tier>(P))});
    });
    return tdists;
}

template<int8_t tier>
void set_move_speed(Point tspot,tier_ty & tier_g){
    /*movecosts start;
    start[underling_cen_idx<tier>(tspot)] = 0;
    for(pair<size_t,move_cost_ty> pointval : upwards_moving_dists<tier>(tspot,start)){
        graph[tieridx<tier>(tspot)].get_edge(pointval.first).movecost = pointval.second;
    }*/
    iter_around8<tier>(tspot,[&](Point dest){
        Edge & e = tier_g[tspot][dest];
        e.movecost = invest_to_time(e.invest,e.dis);
    });
}

template<int8_t tier>
startcosts downwards_moving_dists(movecosts & outcosts,startcosts & tsrcs,Point tdest,tier_ty & graph){
    nodeset dests;
    iter_around8<tier>(tdest,[&](Point P){
        dests.push_back(P);
    });
    
    movecosts & tcosts = outcosts;
    djistras_algorithm(tcosts,tsrcs,dests,graph);

    startcosts tm1_costs;
    iter_around8<tier>(tdest,[&](Point P){
        tm1_costs.push_back(pointcost{underling_cen<tier>(P),tcosts.at(P)});
    });
    return tm1_costs;
}

template<int8_t tier>
void all_down_moving_dists(Point dest,startcosts & srccosts,tiered_movecosts & accumvals,graph_ty & graph){
    startcosts downcosts = downwards_moving_dists<tier>(accumvals.back(),srccosts,tier_rep<tier>(dest),graph[tier]);
    accumvals.push_back(make_ra<tier-1,move_cost_ty>(tier_rep<tier-1>(dest),UNDERLINGS_Ts[tier]*POINT_TIER_CHANGE));
    all_down_moving_dists<tier-1>(dest,downcosts,accumvals,graph);
}
template<>
void all_down_moving_dists<0>(Point dest,startcosts & srccosts,tiered_movecosts & accumvals,graph_ty & graph){
    nodeset dests({dest});
    djistras_algorithm(accumvals.back(),srccosts,dests,graph[0]);
}

template<int8_t tier>
void all_up_moving_dists(Point src,Point dest,startcosts & srccosts,tiered_movecosts & accumvals,graph_ty & graph){
    if(is_in_bordering_tier_rep<tier+1>(src,dest)){
        accumvals.push_back(make_ra<tier,move_cost_ty>(tier_rep<tier>(src),POINT_ADJAC_FACTOR*UNDERLINGS_Ts[tier+1]));
        all_down_moving_dists<tier>(dest,srccosts,accumvals,graph);
    }
    else{
        accumvals.push_back(make_ra<tier,move_cost_ty>(tier_rep<tier>(src),UNDERLINGS_Ts[tier+1]*POINT_TIER_CHANGE));
        startcosts tiercosts = upwards_moving_dists<tier+1>(accumvals.back(),tier_rep<tier+1>(src),srccosts,graph[tier]);
        all_up_moving_dists<tier+1>(src,dest,tiercosts,accumvals,graph);
    }
}
template<>
void all_up_moving_dists<NUM_TIERS-1>(Point ,Point dest,startcosts & srccosts,tiered_movecosts & accumvals,graph_ty & graph){
    constexpr int8_t tier = NUM_TIERS-1;
    accumvals.push_back(make_ra<tier,move_cost_ty>(Point(0,0),Point(1,1)*NUM_Ts[tier]));
    all_down_moving_dists<tier>(dest,srccosts,accumvals,graph);
}

tiered_movecosts move_costs(Point src,Point dest,graph_ty & graph){
    tiered_movecosts mcs;

    startcosts start({pointcost{src,0}});
    all_up_moving_dists<0>(src,dest,start,mcs,graph);
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
        int64_t tier = int64_t(NUM_TIERS) - 1 - (abs(int64_t(mi)-int64_t(bmi)))/2 - (5 - move_size)/2;
        
        movecosts f_mcs = forward_mcs[mi];
        movecosts b_mcs = backwards_mcs[move_size-1-mi];
        assert(SameScope(f_mcs,b_mcs));
        for(Point src : iter_scope(f_mcs)){
            Node & srcnode = graph[tier][src];
            for(Point dest : iter_scope(srcnode)){
                if(dest == src || !b_mcs.IsInScope(dest)){
                    continue;
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
            }
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
void djistras_algorithm(movecosts & output,startcosts & sources,nodeset & dests,tier_ty & graph){
    //output is already constructed, here it just assigns default value (the "empty" marker)
    output.assign(MAX_COST);
    
    auto compare = [](const NodeVal & one,const NodeVal & other){
        return (one.val > other.val);
    };
    priority_queue<NodeVal,vector<NodeVal>,decltype(compare)> minheap(compare);
    RangeArray<bool> done(output.Corner,output.XSize,output.YSize);

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
            for(Point ep : iter_scope(graph[mintime.p])){
                if(ep != mintime.p){
                    add_point(ep,graph[mintime.p][ep],mintime);
                }
            }
        }
    }
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
template<int8_t tier>
void set_move_speeds(tier_ty & graph){
    for(Point P : iter_all<NUM_Ts[tier]>()){
        set_move_speed<tier>(P,graph);
    }
}

void set_move_costs(graph_ty & graph){
    set_move_speeds<0>(graph[0]);
    set_move_speeds<1>(graph[1]);
    set_move_speeds<2>(graph[2]);
}
template<int8_t tier>
void reverse_tier(tier_ty & rev_t,tier_ty & orig_t){
    for(Point src : iter_all<NUM_Ts[tier]>()){
        iter_around8<tier>(src,[&](Point dest){
            rev_t[dest][src] = orig_t[src][dest];
        });
    }
}

graph_ty reverse_graph(graph_ty & graph){
    graph_ty rev = graph;
    reverse_tier<0>(rev[0],graph[0]);
    reverse_tier<1>(rev[1],graph[1]);
    reverse_tier<2>(rev[2],graph[2]);
    return rev;
}

void update_trans_invest(blocks & blks){
    
    set_move_costs(blks.graph);
    
    struct edge_data{
        Point src;
        Point dest;
        Edge * e;
    };
    vector<edge_data> all_edges;
    for(size_t tn : range(NUM_TIERS)){
        tier_ty & t = blks.graph[tn];
        for(Point src : PIterContainter(0,0,NUM_Ts[tn],NUM_Ts[tn])){
            Node & n = t[src];
            for(Point dest : iter_scope(n)){
                if(dest != src){
                    Edge & e = n[dest];
                    all_edges.push_back(edge_data{src,dest,&e});
                    e.marg_benefit_invest = 0;
                }
            }
        }
    }
    graph_ty revgraph = reverse_graph(blks.graph);
    for(size_t pn : range(NUM_PEOPLE)){
        add_marginal_benefit(blks.pps.home[pn],blks.pps.work[pn],blks.graph,revgraph);
    }
    std::sort(all_edges.begin(),all_edges.end(),[](edge_data & one,edge_data & other){
        return one.e->marg_benefit_invest > other.e->marg_benefit_invest;
    });
    //for any value of blks.inv_per_turn > 1 this corrupts the algoritm and makes
    //it produce unoptimal results, but it still does pretty well and also
    //goes that much faster
    for(size_t i = 0; i < blks.inv_per_turn;i++){
        all_edges[i].e->invest++;
    }
}
template<int8_t tier,typename fnty>
void add_view_tier(count_ty & view,tier_ty & graph,fnty add_fn){
    for(Point src:iter_all<NUM_Ts[tier]>()){
        iter_around8<tier>(src,[&](Point dest){
            Edge e = graph[src][dest];
            
            Point start = base_cen<tier>(src);
            Point end = base_cen<tier>(dest);
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
    add_view_tier<0>(view,graph[0],add_fn);
    add_view_tier<1>(view,graph[1],add_fn);
    add_view_tier<2>(view,graph[2],add_fn);    
}

void blocks::update_trans(){
    update_trans_invest(*this);
    
    set_view(trans_invest_view,graph,[](Edge e){return e.invest;});
    set_view(upgrade_vs_view,graph,[](Edge e){return e.marg_benefit_invest * 1000;});
}
