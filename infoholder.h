#pragma once
#include <vector>
#include <cassert>

using namespace std;

constexpr int null_node_val = 0xcccccccc;

struct infoID{
    infoID(int loc=null_node_val){
        vecloc = loc;
    }
    int vecloc;
    bool is_null(){
        return vecloc == null_node_val;
    }
};
template<class int_iter_ty>
class InfoIter{
public:
    InfoIter(int_iter_ty iter_begin){
        iter = iter_begin;
    }
    infoID operator *(){
        return infoID{iter};
    }
    void operator ++(){
        iter++;
    }
    bool operator !=(InfoIter other){
        return iter != other.iter;
    }
protected:
    int_iter_ty iter;
};

template<class info_ty>
class InfoHolder{
private:
    vector<info_ty> info_list;
public:
    using iterator = InfoIter<int>;
    size_t size(){
        return info_list.size();
    }
    info_ty & get(infoID loc){
        assert(loc.vecloc >= 0 && loc.vecloc < size());
        return info_list[loc.vecloc];
    }
    infoID add(info_ty info){
        int new_nodeloc = info_list.size();
        infoID new_node = infoID{new_nodeloc};
        info_list.emplace_back(info);
        return new_node;
    }
    info_ty & operator [](infoID loc){
        return get(loc);
    }
    void delete_val(infoID loc){
        info_list.erase(info_list.begin()+loc.vecloc);
    }
    iterator begin(){
        return iterator(0);
    }
    iterator end(){
        return iterator(size());
    }
};
