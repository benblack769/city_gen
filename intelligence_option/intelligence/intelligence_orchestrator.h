#pragma once
#include "choice_maker.h"
#include "choices.h"
#include "intelligence_inputs.h"

double cost_fn(Person person){
    assert(person.health > 0 && person.energy >= 0);
    return - 1.0 / sqr(person.health) - 1.0 / person.energy;
}

class PersonIntelligence{
protected:
    ChoiceMaker chooser;
public:
    
};
