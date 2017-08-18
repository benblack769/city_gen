#pragma once

#include <headerlib/point.hpp>

#include "intelligence_option/choices.h"
#include "people.h"
#include "map.h"

/*
Strategy goes more or less like this:

for each square that you can move to 
    1. Calculate an overal value for moving onto that square.
Calculate the value of sleeping
Estimate value of building shelter

Convert those values into probablilties, sample the actual action from those probabilities.
*/

inline double turns_survive(double food){
    return log(food+1.0) * 3; 
}
inline double turns_force_sleep(double exaustion){
    return tanh(exaustion) * 3;
}

inline double survive_val(double turns_alive){
    return 50.0 / (turns_alive + 0.0001);
}
inline double sleep_val(double turns_force_sleep){
    return 3.0 / (turns_force_sleep + 0.0001);
}


inline double value_of_sleeping(Person info, PointProperty pp){
    return sleep_val(turns_force_sleep(info.energy + sleep_addition(pp))) 
            - sleep_val(turns_force_sleep(info.energy));
}
inline double value_of_shelter(Person info, PointProperty pp){
    return pp.shelter_val / 3;
}
inline double value_of_eating(Person info, PointProperty pp){
    return survive_val(turns_survive(info.health + health_addition(pp))) 
            - survive_val(turns_survive(info.health));
}
inline double value_of_moving(Person info, PointProperty pp){
    return value_of_sleeping(info,pp) + 
            value_of_shelter(info,pp) + 
            value_of_eating(info,pp);
}

inline vector<double> persons_choice(Person info, const vector<PointProperty> & properties, const PointProperty & cur_point){
   vector<double> values(num_choices);
   iter_around1_idx(info.location, [&](Point pp, int idx){
      	full_choice move_choice(MOVE, pp - info.location); 
      values[choice_idx(move_choice)] = value_of_moving(info,properties[idx]);
   });
    full_choice sleep_choice(REST,Point());
    full_choice shelter_choice(SHELTER,Point());
    full_choice eat_choice(EAT,Point());
    
    values[choice_idx(sleep_choice)] = value_of_sleeping(info,cur_point);
    values[choice_idx(shelter_choice)] = value_of_shelter(info,cur_point);
    values[choice_idx(eat_choice)] = value_of_eating(info,cur_point);
	return values;
}
inline full_choice make_choice(vector<double> choice_vals){
   discrete_distribution<int> dist;
   int outchoice = dist(seed_gen);
   return get_choice(outchoice);
}

class PersonIntelligence{
protected:
public:
    full_choice persons_choice(Person info, const vector<PointProperty> & properties, const PointProperty & cur_point){
        return make_choice(::persons_choice(info,properties,cur_point));
    }
};
