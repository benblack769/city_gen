/*
1. Essencial types and declarations used by block.
2. Aproximization functions to tweak and experiment with.
*/
#include <vector>
#include <cstdint>
#include <point.hpp>

using namespace std;

using move_cost_ty = float;

struct Edge{
    Edge(size_t indest):
        dest(indest){
    }
    size_t dest;
    uint64_t invest = 0;
    move_cost_ty movecost = 0;//cached value caculated from investment
    move_cost_ty marg_benefit_invest = 0;
};
struct Node{
    Point src;
    vector<Edge> edges;
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

vector<Node> make_graph();

inline decltype(Point::X) dist(Point a,Point b){
    return abs(a.X-b.X) + abs(a.Y-b.Y);
}
static const move_cost_ty MAX_COST =1LL<<28;
inline size_t invest_to_speed(size_t invest){
    return (invest + 1);
}
inline move_cost_ty invest_to_time(size_t invest){
    return move_cost_ty(1.0) / invest_to_speed(invest);
}
inline move_cost_ty time_dif_upgrade(move_cost_ty curtime,size_t cur_invest){
    return curtime - (curtime * invest_to_speed(cur_invest)) / invest_to_speed(cur_invest+1);
}
