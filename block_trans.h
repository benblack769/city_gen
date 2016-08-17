/*
1. Essencial types and declarations used by block.
2. Aproximization functions to tweak and experiment with.
*/
#include <vector>
#include <cstdint>
#include <cassert>
#include <headerlib/point.hpp>

using namespace std;

using move_cost_ty = float;

struct Edge{
    Edge(size_t indest,float indis):
        dest(indest),
        dis(indis){
    }
    size_t dest;
    uint32_t invest = 0;
    float dis;
    move_cost_ty movecost = 0;//cached value caculated from investment
    move_cost_ty marg_benefit_invest = 0;
};
struct Node{
    Point src;
    vector<Edge> edges;
    void add_edge(size_t edge_dest,float dis){
        edges.emplace_back(edge_dest,dis);
    }
    Edge & get_edge(size_t edge_dest){
        return *find(edge_dest);
    }
protected:
    vector<Edge>::iterator find(size_t edge_dest){
        vector<Edge>::iterator res = find_if(edges.begin(),edges.end(),[&](Edge & e){return e.dest == edge_dest;});
        assert(res != edges.end());
        return res;
    }
};

vector<Node> make_graph();

