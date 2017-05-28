#pragma once
#include <iostream>
#include "people.h"
#include "infoholder.h"
#include "map.h"

inline Point wrap_point(Point p){
    return Point(p.X %WORLD_SIZE,p.Y% WORLD_SIZE);
}

class World{
public:
    People people;
    Map map;
    
    World(){
        init_world();
    }
    void addPersonToMap(infoID person,Point p){
        Point & ploc = people[person].location;
        ploc = wrap_point(p);
        map[ploc].residents.add(person);
    }
    void removePersonFromMap(infoID person){
        Point ploc = people[person].location;
        map[ploc].residents.del(person);
    }
    void movePerson(infoID person,Point dest){
        removePersonFromMap(person);
        addPersonToMap(person,dest);
    }
    void people_wander(){
        for(infoID person : people){
            
        }
    }
    void init_world(){
        for(int x : range(NUM_PEOPLE)){
            add_random_person();
        }
    }
    void add_random_person(){
        Person p;
        infoID per = people.add(p);
        addPersonToMap(per,random_p());
    }
    Point random_p(){
        return Point(rand()%WORLD_SIZE,rand()%WORLD_SIZE);
    }
    void update(){
        for(infoID pid : people){
            Point loc = people[pid].location;
            
            Point best_loc = loc;
            double best_val = -10e-500;
            for(Point p : iter_around(loc,1)){
                double val = map[p].food_content;
                if(best_val < val){
                    best_val = val;
                    best_loc = p;
                }
            }
            movePerson(pid,best_loc);
        }
        map.update_point_properties();
    }
};
