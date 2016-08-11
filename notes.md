# Notes about city_gen

## Tiling:

If we ever want to create a truely large interacting world with a lot of spacial reasoning, the only way to do this reasonably is through tiling.

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
           

Say the source is on a tile of level 2, trying to get somewhere outside of that tile. It will path to the 4 tiles of size 3 surrounding their current
tile of level 3, and then not try any more paths on tile level 2 until coming to a tile level 3 that borders the level 3 tiles that contains the destination.

This way, any pathing or spacial analysis can happen in log^2 time with the world size instead of quadratic.

NOTE: there will be stored accumulated stats for each tile size, and each size should be viewable. 

GRAPHICS NOTE: The smallest sizes might have have tiled graphics, as in a map, if it is too slow.

MAX SIZE: This will allow for world sizes around 10,000x10,000 on an ordinary machine.

## Traits/Attributes: 

Can be picked up in two ways, depending on the trait:

Inborn trait: Randomly produced at birth, possible by genetic aproximation from parents.

Learned traits: Picked up from people around the person.

#### Inborn traits examples:

* Physical weight (discrimination factor, especially for women)
* Heathiness (sickness rate)

#### Learned:

* Friendliness (affect how fast one picks up friends)
* Authoritarian leaning (affects how one deals with work environment)
* Violence (many negative/violent interactions)
* Religious fervor 
* Job skills

#### Mixed:

* Introversion (Friend max)

#### Numeric

* Income/wealth
* Religion marker (integer indication of a religion afflilation)

At the beginning, these can all be arbitary, independent "Discrimination Factors".

This is basically what it sounds like, people with more similar factors will have much easier time making friends, and working with others, etc. 
This should nicely simulate historical phenomenom like ghettos and deaf support communities.


Right now, these can be implemennted by an array of integers with enum indexes, 
allowing for both value comparisons and uniqueness comparsions, both ease and performance.

Will be updated with each tick, but will have wildly different rates of change, some will never change (true inborn traits).

Traits should be as independent and as few as possible. Try combining some traits to get what you want rather than creating new one.

## Transportation investment:

Investment on differnt tile sizes will be completely independent. 
You can think of this as different layers of transporations, a long distance one that goes high in 
the sky and short distance ones on the ground. 

This is not meant to look good, it is not meant to generate realistic-looking cities. It is meant
to allow very fast route calculations that will help simulate crude spacial relationship phenomena like ghettos, districts, etc.


### Speed calculation

In general, larger tiles should just about always have
faster transit, and all pathing algorithms will assume this.

### Investment calculation 

Use tiling and the altered djikstras algorithms to find optimal paths of the people's routes.
Then find the optimal cost/benefit calculation. Consider comparing cost of transit investment to other region wide costs.

Consider calculating a list of best tiles to improve to save computation time (as is currently done). Watch out for bad descisions here, though.

Under current plan, it is not possible to not allow people to live, work, etc on tiles with high investment to create more realistic looking cities, but
it can be part of a property cost evaluation (it takes up space, driving up prices). 

## Land prices

Prices will affect investment and firm and worker housing purchases. People will "buy" some land, when they move there, if their wealth attribute is high enough.

Roads will similarly increase in price as land prices increase.

## Routes:

People will have routes to go from home to work. Eventually, this should be explanded to include things like

* lunch joints
* community centers (churches, schools, charities)
* coffee shops
* friend's homes
* and more

People use the algorithm described above to find optimal paths between these places. 

## Interactions:

People, at home, work, other route destinations, and while commuting to those various destinations, will have some chance of interacting with the people around them.

The larger the tile they pass through, there will be a much lower the chance of interacting. 

Interactions with people might create friendships, or might affect the attributes of those involved.

Peoples interactions with 

## Relationships

Relationships are born and maintatined by interactions. Although attributes like discrimination factors will be involved, what really
will make relationships grow and thrive are phisical interactions. 

## Jobs
