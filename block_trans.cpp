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

struct Edge{
    Edge(size_t indest):
        dest(indest),
        movecost(0){
    }
    size_t dest;
    move_cost_ty movecost;//cached value caculated from investment
};
struct Node{
    uint64_t tile_tier;
    Point src;
    vector<Edge> edges;
    //vector<bool> edge_is_attached;//allows for constant time operations
    void delete_edge(size_t edge_dest){
        edges.erase(find(edge_dest));
    }
    void add_edge(size_t edge_dest){
        edges.push_back(edge_dest);
    }
    Edge & get_edge(size_t edge_dest){
        return *find(edge_dest);
    }
protected:
    vector<Edge>::iterator find(size_t edge_dest){
        return find_if(edges.begin(),edges.end(),[&](Edge & e){return e.dest == edge_dest;});
    }
};
unordered_map<size_t,move_cost_ty> djistras_algorithm(unordered_map<size_t,move_cost_ty> source,unordered_set<size_t> dests,vector<Node> & graph);
move_cost_ty core_trav_time(Point src,Point dest_1_away,mcarr & mcs){
    return (mcs[src] + mcs[dest_1_away]) / 2;
}
constexpr size_t UNDERLINGS_Ts[NUM_TIERS] = {0,TRANS_TIER_1_UNDERLINGS,TRANS_TIER_2_UNDERLINGS};
constexpr size_t NUM_Ts[NUM_TIERS] = {WORLD_SIZE ,WORLD_SIZE / TRANS_TIER_1_UNDERLINGS,TRANS_TIER_1_UNDERLINGS / TRANS_TIER_2_UNDERLINGS};
constexpr size_t SIZE_Ts[NUM_TIERS] = {1,TRANS_TIER_1_UNDERLINGS,TRANS_TIER_1_UNDERLINGS * TRANS_TIER_2_UNDERLINGS};

template<int64_t tier>
size_t tier_size_accum(){
    //count of nodes up to and including the current tier
    return tier_size_accum<tier-1>() + sqr(NUM_Ts[tier]);
}
template<>
size_t tier_size_accum<-1>(){
    return 0;
}

template<int64_t tier>
size_t tieridx(Point P){
    return tier_size_accum<tier-1>() + P.Y * NUM_Ts[tier] + P.X;
}
template<int64_t tier>
Point underling_cen(Point tspot){
    int32_t add_to = (UNDERLINGS_Ts[tier] + 1) / 2;
    Point tblock = tspot * UNDERLINGS_Ts[tier];
    return tblock + Point{add_to,add_to};
}
template<int64_t tier>
Point overlord(Point tspot){
    return tspot / UNDERLINGS_Ts[tier];
}
template<int64_t tier>
Point tier_rep(Point boardspot){
    return boardspot / SIZE_Ts[tier];
}

template<size_t tier,typename fnty>
void iter_around8(Point cen_p,fnty itfn){
    for(Point P : iter_around<NUM_Ts[tier]>(cen_p,1)){
        if(P != cen_p){
            itfn(P);
        }
    }
}
template<int64_t tier>
bool is_in_bordering_tier_rep(Point a,Point b){
    Point a_rep = tier_rep<tier>(a);
    Point b_rep = tier_rep<tier>(b);
    return abs(a_rep.X - b_rep.X) + abs(a_rep.Y - b_rep.Y) <= 2;
}

template<size_t tier>
Node make_node(Point src){
    Node nn{0,src,vector<Edge>()};
    nn.edges.reserve(8);
    for(Point dest : iter_around<NUM_Ts[tier]>(src,1)){
        if(dest != src){
            nn.edges.emplace_back(dest,src);
        }
    }
}

struct tiered_graph{
    vector<Node> graph;
    
    mcarr * mcs;//non-owning
    
    tiered_graph(mcarr & inmcs):
        mcs(&inmcs)
    {
        make_nodes<0>();
        make_nodes<1>();
        make_nodes<2>();
    }
    
    
    template<size_t tier>
    void make_nodes(){
        for(Point P : iter_all<tier>()){
            graph[tieridx<tier>(P)] = make_node<tier>(P);
        }
    }
    void add_edge(size_t start,size_t end){
        graph[start].add_edge(end);
        graph[end].add_edge(start);
    }
    void delete_edge(size_t start,size_t end){
        graph[start].delete_edge(end);
        graph[end].delete_edge(start);
    }
    
    template<size_t tier,typename fnty>
    void do_to_tier_below(Point tspot,Point attdir,size_t distance,fnty do_fn){
        assert(((abs(attdir.X) | abs(attdir.Y)) == 1) && "attdir must be a cardinal direction");
        
        size_t tidx = tieridx<tier>(tspot);
        
        auto if_neg1_then_0 = [](int32_t num){return num == -1 ? 0 : num;};
        Point start_spot = tspot + Point{if_neg1_then_0(attdir.X),if_neg1_then_0(attdir.Y)};
        
        Point start_coord = start_spot * UNDERLINGS_Ts[tier];
        if(attdir.X == attdir.Y){
            //if diagonal direction, attach the single point that connects big thing diagonally.
            Point addcoord = start_spot + attdir;
            do_fn(tidx,tieridx<tier-1>(addcoord));
        }
        else{
            //else connect all the points along the edge.
            Point iter_dir = {abs(attdir.Y),abs(attdir.X)};
            Point curspot = start_spot;
            for(size_t i : range(distance)){
                do_fn(tidx,tieridx<tier-1>(curspot));
                curspot += iter_dir;
            }
        }
    }
    template<size_t tier,typename fnty>
    void do_to_tier_below_around(Point tspot,size_t distance,fnty do_fn){
        for(Point dir = {-1,-1}; dir.Y <= 1; dir.Y++){
            for(dir.X = -1; dir.X <= 1 ;dir.X++){
                if(dir != Point{0,0}){
                    do_to_tier_below(tspot,dir,do_fn);
                }
            }
        }
    }
    
    template<size_t tier>
    unordered_map<size_t,move_cost_ty> get_around_dists(Point tm1_spot,unordered_map<size_t,move_cost_ty> tm1_starts){
        Point tspot = overlord<tier-1>(tm1_spot);
        
        unordered_set<size_t> dests;
        iter_around8<tier>(tspot,[&](Point P){
            dests.insert(tieridx<tier-1>(underling_cen<tier>(P)));
        });
        
        unordered_map<size_t,move_cost_ty> tm1_dists = djistras_algorithm(tm1_starts,dests,graph);
        
        unordered_map<size_t,move_cost_ty> tdists;
        iter_around8<tier>(tspot,[&](Point P){
            tdists[tieridx<tier>(P)] = tm1_dists[tieridx<tier-1>(underling_cen<tier>(P))];
        });
        return tdists;
    }

    template<size_t tier>
    void set_move_speed(Point tspot){
        Point tblockcen = underling_cen<tier>(tspot);
        unordered_map<size_t,move_cost_ty> start;
        start[tieridx<tier-1>(tblockcen)] = 0;
        for(pair<size_t,move_cost_ty> pointval : get_around_dists<tier>(tblockcen,start)){
            graph[tieridx<tier>(tspot)].get_edge(pointval.first).movecost = pointval.second;
        }
    }
    unordered_map<size_t,move_cost_ty> move_up_vals(Point src,Point dest){
        unordered_map<size_t,move_cost_ty> start;
        start[tieridx<0>(src)] = 0;
        if(is_in_bordering_tier_rep<1>(src,dest)){
            return 
        }
        else{
            unordered_map<size_t,move_cost_ty> next_t0 = get_around_dists<0>(src,start);
            unordered_map<size_t,move_cost_ty> next_t1 = get_around_dists<1>(src,next_t0);
            if(is_in_bordering_tier_rep<2>(src,dest)){
                
            }
            else{
                unordered_map<size_t,move_cost_ty> next_t2 = get_around_dists<2>(src,next_t1);
                
            }
        }
        
    }
    
    
    /*void detach(size_t nidx){
        //disconnects all edges that point towards it (assumes all nodes that point towards it can be reached by edges).
        for(Edge & e : graph[nidx].edges){
            graph[e.dest].delete_edge(e.dest);
        }
    }
    void attach(size_t nidx){
        //adds edges in other direction to own edges. (assumes that it has been completely detached)
        for(Edge & e : graph[nidx].edges){
            graph[e.dest].add_edge(nidx);
        }
    }*/
    
};





struct NodeVal{
    move_cost_ty val;
    size_t n;
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
unordered_map<size_t,move_cost_ty> djistras_algorithm(unordered_map<size_t,move_cost_ty> sources,unordered_set<size_t> dests,vector<Node> & graph){
    auto compare = [](const NodeVal & one,const NodeVal & other){
        return (one.val > other.val);
    };
    priority_queue<NodeVal,vector<NodeVal>,decltype(compare)> minheap(compare);
    unordered_set<size_t> done;
    unordered_map<size_t,move_cost_ty> move_to_val;
	
    auto add_point = [&](Edge & n,NodeVal prev){
        if(!done.count(n.dest)){
            move_cost_ty tot_val = prev.val+n.movecost;
            
            move_to_val[n.dest] = tot_val;
            minheap.push(NodeVal{tot_val,n.dest});
            
            done.insert(n.dest);
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
        for(Edge & n : graph[mintime.n].edges){
            add_point(n,mintime);
            if(dests.count(n.dest)){
                dests.erase(n.dest);
                if(dests.size()){
                    //stops computation soon after dests are found
                    max_iter = min(max_iter,DJISTA_ITERS_AFTER_DEST_FOUND);
                }
            }
        }
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
