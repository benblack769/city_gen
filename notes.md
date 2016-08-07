# Notes about city_gen

## Tiling:

If we ever want to create a truely large interacting world, the only way to do this reasonably is through tiling.

Basically there are points, at the very smallest tile size. 

If there is an interaction within a certain distance of that tile, then it goes and computes things in terms of those smallest points.

If there is an interaction outside of that distance, then it starts working with the next largest tile size, then the next largest, etc.

So ones path to work may look like:

    - (smallest tile)
    x (second smallest tile)
    ^ (Third smallest tile)
    
    ---
      -xx
       xx
       xxxx^^^^
       xxxx^^^^
           ^^^^xx-
           ^^^^xx
           
This way, any pathing or spacial analysis can happen in log^2 time with the world size instead of quadratic.

NOTE: there will be stored accumulated stats for each tile size, and each size should be viewable. 

GRAPHICS NOTE: The smallest sizes might have have tiled graphics, as in a map, if it is too slow.

MAX SIZE: This will allow for world sizes around 10,000x10,000 on an ordinary machine.



## Traits: 

Can be picked up in two ways, depending on the trait:

Inborn trait: Randomly produced at birth, possible by genetic aproximation from parents.

Learned traits: Picked up from people around the person.

#### Inborn traits examples:

* Physical weight (discrimination factor, especially for women)
* Heathiness (sickness rate)

#### Learned:

* Friendliness (affect how fast one picks up friends)
* Authoritarian leaning (affects how one deals with work environment)

#### Mixed:

* Introversion (Friend max)


At the beginning, these can all be arbitary, independent "Discrimination Factors".

This is basically what it sounds like, people with more similar factors will have much easier time making friends, and working with others, etc. 
This should nicely simulate historical phenomenom like ghettos and deaf support communities.


Right now, these can be implemennted by an array of integers with enum indexes, 
allowing for both value comparisons and uniqueness comparsions, both ease and performance.

Will be updated with each tick, but will have wildly different rates of change, some will never change (true inborn traits).



## Transportation investment:

Investment on differnt tile sizes will be completely independent. 
You can think of this as different layers of transporations, a long distance one that goes high in 
the sky and short distance ones on the ground. 

### Speed calculation



In general, larger tiles should just about always have
faster transit, and all pathing algorithms will assume this.

### Investment calculation 

Use tiling and the djikstras algorithms to find optimal paths of the people's routes.
Then find the optimal cost/benefit calculation.


One thing to watch out here for is 


## Routes:

Interactions:

People while commuting to various destinations and in their home tile of all sizes, will have
