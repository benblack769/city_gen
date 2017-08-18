#pragma once
#include <iostream>

#include "people.h"
#include "infoholder.h"
#include "map.h"
#include "intelligence_option/intelligence.h"

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
        p.health = 1.0;
        p.energy = 1.0;
        infoID per = people.add(p);
        addPersonToMap(per,random_p());
    }
    Point random_p(){
        return Point(rand()%WORLD_SIZE,rand()%WORLD_SIZE);
    }
    void update(){
        
        clock_t start = clock();
        clock_t tot_other = 0;
        
        default_random_engine rand_gen(seed_gen());
        
        clock_t arg = clock();
        map.update_point_properties();
        tot_other += clock() - arg;
        for(infoID pid : people){
            Person & pinfo = people[pid];
            Point loc = people[pid].location;
            PointProperty & mylocinfo = map[loc];
            
            // uses intelligence to make choice
            PersonIntelligence intel;
            PointsAround points = map.get_points_around(loc);
            full_choice mychoice = intel.persons_choice(pinfo,points,map[loc]);
            
            
            // make world state respond to choice
            pinfo.energy *= 0.9;
            pinfo.health *= 0.9;
            switch(mychoice.base){
            case MOVE:
                movePerson(pid, loc + mychoice.move_dir);
                pinfo.energy *= 0.9;
                break;
            case EAT:
                pinfo.health += health_addition(mylocinfo);
                break;
            case REST:
                pinfo.energy += sleep_addition(mylocinfo);
                break;
            case SHELTER:
                mylocinfo.shelter_val += 0.1;
                break;
            default:
                assert(false && "no default case");
            }
        }
        clock_t tot_time = clock() - start;
        cout << "tot time = " << tot_time << endl;
        cout << "choice_time = " << tot_other << endl;
    }
};
