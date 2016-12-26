# City Generator

### Inspiration

I was inspired by city simulation games, in particular SimCity 4, and their power to model the real world in a level of detail that allows you to create realistic feeling cities. However, I never particularly liked the fact that you had to dictatorially build all the city infrastructure by hand. In particular, transportation networks were tedious and difficult to build.

So, as a computer scientist, I was wondering: How might one build an optimal road network? If this is too difficult, then at least, how might one measure the efficiency of a road network?

### Model

I first reduced the complicated problem of building infrastructure in cities to the simplest case that could demonstrate an algorithm for automated network construction.

I considered a world where people randomly choose two locations on a grid to commute between. I used taxicab distance, assuming that the world is already in some sort of grid. Building and improving roads takes money, speeds up commute time, but improving roads gives decreasing returns. In particular, I set `time = 1/(investment+1)`.

The end goal is for the total sum of commute times to be minimal.

How might one measure this? This is clearly a shortest path problem.


###
