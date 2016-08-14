#include "blocks.h"
#include "RangeIterator.h"
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

using movecosts = unordered_map<size_t,move_cost_ty>;
using nodeset = unordered_set<size_t>;
using nodecost = pair<size_t,move_cost_ty>;

unordered_map<size_t,move_cost_ty> djistras_algorithm(movecosts source,nodeset dests,vector<Node> & graph);

constexpr size_t UNDERLINGS_Ts[NUM_TIERS] = {0,TRANS_TIER_1_UNDERLINGS,TRANS_TIER_2_UNDERLINGS};
constexpr size_t NUM_Ts[NUM_TIERS] = {WORLD_SIZE ,WORLD_SIZE / TRANS_TIER_1_UNDERLINGS,TRANS_TIER_1_UNDERLINGS / TRANS_TIER_2_UNDERLINGS};
constexpr size_t SIZE_Ts[NUM_TIERS] = {1,TRANS_TIER_1_UNDERLINGS,TRANS_TIER_1_UNDERLINGS * TRANS_TIER_2_UNDERLINGS};

template<int8_t tier>
size_t tier_size_accum(){
    //count of nodes up to and including the current tier
    return tier_size_accum<tier-1>() + sqr(NUM_Ts[tier]);
}
template<>
size_t tier_size_accum<-1>(){
    return 0;
}

template<int8_t tier>
size_t tieridx(Point P){
    return tier_size_accum<tier-1>() + P.Y * NUM_Ts[tier] + P.X;
}
template<int8_t tier>
Point underling_cen(Point tspot){
    int32_t add_to = (UNDERLINGS_Ts[tier] + 1) / 2;
    Point tblock = tspot * UNDERLINGS_Ts[tier];
    return tblock + Point{add_to,add_to};
}
template<int8_t tier>
size_t underling_cen_idx(Point tspot){
    return tieridx<tier-1>(underling_cen<tier>(tspot));
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
    return ((tspot + Point{1,1}) * SIZE_Ts[tier]) / 2;
}

template<int8_t tier,typename fnty>
void iter_around8(Point cen_p,fnty itfn){
    for(Point P : iter_around<NUM_Ts[tier]>(cen_p,1)){
        if(P != cen_p){
            itfn(P);
        }
    }
}
template<int8_t tier>
bool is_in_bordering_tier_rep(Point a,Point b){
    Point a_rep = tier_rep<tier>(a);
    Point b_rep = tier_rep<tier>(b);
    return abs(a_rep.X - b_rep.X) + abs(a_rep.Y - b_rep.Y) <= 2;
}

template<int8_t tier>
Node make_node(Point src){
    Node nn{src,vector<Edge>()};
    nn.edges.reserve(8);
    for(Point dest : iter_around<NUM_Ts[tier]>(src,1)){
        if(dest != src){
            nn.add_edge(tieridx<tier>(dest));
        }
    }
    return nn;
}
template<int8_t tier>
void make_nodes(vector<Node> & graph){
    for(Point P : iter_all<tier>()){
        graph[tieridx<tier>(P)] = make_node<tier>(P);
    }
}
vector<Node> make_graph(){
    size_t size = tier_size_accum<NUM_TIERS-1>();
    vector<Node> graph(size);

    make_nodes<0>(graph);
    make_nodes<1>(graph);
    make_nodes<2>(graph);
    return graph;
}
template<int8_t tier>
movecosts upwards_moving_dists(Point tspot,movecosts tm1_starts,movecosts & accumvals,vector<Node> & graph){
    nodeset dests;
    iter_around8<tier>(tspot,[&](Point P){
        dests.insert(underling_cen_idx<tier>(P));
    });

    movecosts tm1_dists = djistras_algorithm(tm1_starts,dests,graph);
    accumvals.insert(tm1_dists.begin(),tm1_dists.end());

    movecosts tdists;
    iter_around8<tier>(tspot,[&](Point P){
        tdists[tieridx<tier>(P)] = tm1_dists[underling_cen_idx<tier>(P)];
    });
    return tdists;
}

template<int8_t tier>
void set_move_speed(Point tspot,vector<Node> & graph){
    /*movecosts start;
    start[underling_cen_idx<tier>(tspot)] = 0;
    for(pair<size_t,move_cost_ty> pointval : upwards_moving_dists<tier>(tspot,start)){
        graph[tieridx<tier>(tspot)].get_edge(pointval.first).movecost = pointval.second;
    }*/
    for(Edge & e : graph[tieridx<tier>(tspot)].edges){
        e.movecost = invest_to_time(e.invest);
    }
}

template<int8_t tier>
movecosts downwards_moving_dists(movecosts tsrcs,Point tdest,movecosts & accumvals,vector<Node> & graph){
    nodeset dests;
    iter_around8<tier>(tdest,[&](Point P){
        dests.insert(tieridx<tier>(P));
    });
    movecosts tdists = djistras_algorithm(tsrcs,dests,graph);
    accumvals.insert(tdists.begin(),tdists.end());

    movecosts tbelow_dists;
    iter_around8<tier>(tdest,[&](Point P){
        tbelow_dists[underling_cen_idx<tier>(P)] = tdists[tieridx<tier>(P)];
    });
    return tbelow_dists;
}

template<int8_t tier>
void all_down_moving_dists(Point dest,movecosts srccosts,movecosts & accumvals,vector<Node> & graph){
    all_down_moving_dists<tier-1>(dest,
        downwards_moving_dists<tier>(srccosts,tier_rep<tier>(dest),accumvals,graph),
        accumvals,graph);
}
template<>
void all_down_moving_dists<0>(Point dest,movecosts srccosts,movecosts & accumvals,vector<Node> & graph){
    nodeset dests({tieridx<0>(dest)});
    movecosts mv_to_dest_costs = djistras_algorithm(srccosts,dests,graph);
    accumvals.insert(mv_to_dest_costs.begin(),mv_to_dest_costs.end());
}

template<int8_t tier>
void all_up_moving_dists(Point src,Point dest,movecosts srccosts,movecosts & accumvals,vector<Node> & graph){
    if(tier >= NUM_TIERS-1 || is_in_bordering_tier_rep<tier+1>(src,dest)){
        all_down_moving_dists<tier>(dest,srccosts,accumvals,graph);
    }
    else{
        movecosts tiercosts = upwards_moving_dists<tier>(tier_rep<tier>(src),srccosts,accumvals,graph);
        all_up_moving_dists<tier+1>(src,dest,tiercosts,accumvals,graph);
    }
}
template<>
void all_up_moving_dists<NUM_TIERS>(Point ,Point ,movecosts ,movecosts & ,vector<Node> & ){
    return;
}

movecosts move_costs(Point src,Point dest,vector<Node> & graph){
    movecosts mcs;

    movecosts start;
    start[tieridx<0>(src)] = 0;
    all_up_moving_dists<0>(src,dest,start,mcs,graph);
    return mcs;
}
void add_marginal_benefit(Point src,Point dest,vector<Node> & graph,vector<Node> & revgraph){
    //revgraph is identical to graph, but the edge movecosts are swapped between the paired edges.
    movecosts forward_mcs = move_costs(src,dest,graph);
    movecosts backwards_mcs = move_costs(src,dest,revgraph);
    
    move_cost_ty min_cost = forward_mcs[tieridx<0>(dest)];
    
    for(nodecost nc : forward_mcs){
        for(Edge & e : graph[nc.first].edges){
            if(backwards_mcs.count(nc.first)){
                move_cost_ty mv_thr_cst_wo_edge = nc.second + backwards_mcs[e.dest];
                
                assert(mv_thr_cst_wo_edge + e.movecost < min_cost && "min cost not smallest cost!");
                
                move_cost_ty upgraded_cost = time_dif_upgrade(e.movecost,e.invest);
                move_cost_ty new_cost = upgraded_cost + mv_thr_cst_wo_edge;
                move_cost_ty gained_time = min_cost - new_cost;
                if(gained_time > 0){
                    e.marg_benefit_invest += gained_time;
                }
            }
        }
    }
}




struct NodeVal{
    move_cost_ty val;
    size_t n;
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
unordered_map<size_t,move_cost_ty> djistras_algorithm(movecosts sources,nodeset dests,vector<Node> & graph){
    auto compare = [](const NodeVal & one,const NodeVal & other){
        return (one.val > other.val);
    };
    priority_queue<NodeVal,vector<NodeVal>,decltype(compare)> minheap(compare);
    nodeset done;
    movecosts move_to_val;

    auto add_point = [&](Edge & e,NodeVal prev){
        if(!done.count(e.dest)){
            move_cost_ty tot_val = prev.val+e.movecost;

            move_to_val[e.dest] = tot_val;
            minheap.push(NodeVal{tot_val,e.dest});

            done.insert(e.dest);
        }
    };
    //sets start values
    for(auto source : sources){
        minheap.push(NodeVal{source.second,source.first});
        done.insert(source.first);
    }
    //main algorithm
    for(size_t max_iter = blocks::arrsize(); max_iter > 0 && minheap.size() > 0; max_iter--){
        NodeVal mintime = minheap.top();
        minheap.pop();
        for(Edge & e : graph[mintime.n].edges){
            add_point(e,mintime);
            if(dests.count(e.dest)){
                dests.erase(e.dest);
                if(dests.size()){
                    //stops computation soon after dests are found
                    max_iter = min(max_iter,DJISTA_ITERS_AFTER_DEST_FOUND);
                }
            }
        }
    }
    return move_to_val;
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
void set_move_speeds(vector<Node> & graph){
    for(Point P : iter_all<NUM_Ts[tier]>()){
        set_move_speed<tier>(P,graph);
    }
}

void set_move_costs(vector<Node> & graph){
    set_move_speeds<0>(graph);
    set_move_speeds<1>(graph);
    set_move_speeds<2>(graph);
}
vector<Node> reverse_graph(vector<Node> graph){
    vector<Node> rev = graph;
    for(size_t nn : range(graph.size())){
        for(Edge & e : graph[nn].edges){
            rev[e.dest].get_edge(nn).movecost = e.movecost;
        }
    }
    return rev;
}

void update_trans_invest(blocks & blks){
    
    set_move_costs(blks.graph);
    
    vector<Edge *> all_edges;
    for(Node & n : blks.graph){
        for(Edge & e : n.edges){
            all_edges.push_back(&e);
        }
    }
    for(Edge * e : all_edges){
        e->marg_benefit_invest = 0;
    }
    vector<Node> revgraph = reverse_graph(blks.graph);
    for(size_t pn : range(NUM_PEOPLE)){
        add_marginal_benefit(blks.pps.home[pn],blks.pps.work[pn],blks.graph,revgraph);
    }
    std::sort(all_edges.begin(),all_edges.end(),[](Edge * one,Edge * other){
        return one->marg_benefit_invest > other->marg_benefit_invest;
    });
    //for any value of blks.inv_per_turn > 1 this corrupts the algoritm and makes
    //it produce unoptimal results, but it still does pretty well and also
    //goes that much faster
    for(size_t i = 0; i < blks.inv_per_turn;i++){
        all_edges[i]->invest++;
    }
}
template<int8_t tier,typename fnty>
void add_view_tier(count_ty & view,vector<Node> & graph,fnty add_fn){
    for(Node & n : graph){
        for(Edge & e : n.edges){
            Point start = base_cen<tier>(n.src);
            Point end = base_cen<tier>(graph[e.dest].src);
            Point inc = (end - start) / SIZE_Ts[tier];
            for(Point P = start;P != end; P += inc){
                view[P] = add_fn(e);
            }
        }
    }
}
template<typename fnty>
void set_view(count_ty & view,vector<Node> & graph,fnty add_fn){
    view.assign(0);
    add_view_tier<0>(view,graph,add_fn);
    add_view_tier<1>(view,graph,add_fn);
    add_view_tier<2>(view,graph,add_fn);    
}

void blocks::update_trans(){
    update_trans_invest(*this);
    
    set_view(trans_invest_view,graph,[](Edge & e){return e.invest;});
    set_view(upgrade_vs_view,graph,[](Edge & e){return e.marg_benefit_invest * 1000;});
}
