#pragma once
#include <unordered_set>
#include "constants.h"
#include "infoholder.h"

class PeopleSet{
private:
    unordered_set<int> people;
public:
    using set_iter = unordered_set<int>::iterator;
    using iterator = InfoIter<set_iter>;
    void add(infoID person){
        people.insert(person.vecloc);
    }
    void del(infoID person){
        people.erase(person.vecloc);
    }
    bool in(infoID person){
        return people.count(person.vecloc);
    }
    size_t size(){
        return people.size();
    }
    iterator begin(){
        return iterator(people.begin());
    }
    iterator end(){
        return iterator(people.end());
    }
};
struct PointProperty{
    double travel_ease;
    double shelter_val;
    double food_content;
    PeopleSet residents;
};

inline double health_addition(const PointProperty & pp){
    return pp.food_content / 2;
}
inline double sleep_addition(const PointProperty & pp){
    return 1 + pp.shelter_val * 2;
}

class Map{
private:
    board<PointProperty> g_props;
public:
    Map(){
        default_random_engine gen(seed_gen());
        for(PointProperty & pp : g_props){
            pp.food_content = urand(gen);
        }
    }
    PointProperty & operator[](Point p){
        return g_props[p];
    }
    void update_point_properties(){
        for(PointProperty & pp : g_props){
            pp.food_content += 0.001;
            pp.food_content /= (1+pp.residents.size());
            
            pp.shelter_val *= 0.997;
            
            pp.travel_ease *= 0.997;
        }
    }
    void get_points_around(vector<PointProperty> & out_pinputs,Point location){
        assert(out_pinputs.size() == 9);
        int i = 0; 
        iter_around1(location,[&](Point x){
            out_pinputs[i] = g_props[x];
            i++;
        });
    }
};
