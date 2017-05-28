#pragma once
#include "constants.h"
#include "infoholder.h"
#include <unordered_set>

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
    //double travel_speed;
    //double shelter_val;
    double food_content;
    PeopleSet residents;
};

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
        }
    }
};
