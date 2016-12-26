# City Generator

## Installation

Requires Qt 5.

Can be built using standard QT methods.

## Overview

### Inspiration

I was inspired by city simulation games, in particular SimCity 4, and their power to model the real world in a level of detail that allows you to create realistic feeling cities. However, I never particularly liked the fact that you had to dictatorially build all the city infrastructure by hand. In particular, transportation networks were tedious and difficult to build.

So, as a computer scientist, I was wondering: How might one build an optimal road network? If this is too difficult, then at least, how might one measure the efficiency of a road network?

### Model

I first reduced the complicated problem of building infrastructure in cities to the simplest case that could demonstrate an algorithm for automated network construction.

I considered a world where people randomly choose two locations on a grid to commute between. Roads are built connecting grid points in a directional way, simulating the cost of building two way streets instead of one way streets. Building and improving roads takes money, speeds up commute time, but improving roads gives decreasing returns. In particular, I set `time = 1/(investment+1)`.

The end goal is for the total sum of commute times to be minimal.

How might one measure this? This is clearly a shortest path problem. So I just used Dijkstra's Algorithm.

Here is some output from an earlier version that shows the paths the people are taking from their source (blue points) to their destination (gray points).

![path_image](https://github.com/weepingwillowben/city_gen/tree/master/screenshots/path_image.PNG)

Note that this version uses taxicab distance and is not directional, so it looks different than the others. This version is stored in the simple_tiles branch.

### Update Algorithm

Code for the upgrade algorithm is all in block_trans.cpp

What I eventually figured out is that I could exactly calculate the best section to invest in next, supposing that this would be the last section I will ever invest in. To me, this was a very nice result, as it is exactly the greedy city planner, who does not look into the future at all, but only the next time step.

So how do you go about doing this? (Explanation assumes knowledge of graph terminology).

Intuitively, I calculated the single source costs from both the source and the destination for all vertexes, and for all edges looked at the benefit investing more in that edge will bring, vs the optimal path. Note that only edges which are nearly as fast the optimal path will have any benefit at all.

Then I combined these benefits from all the commute routes in the grid, and simply invested in the one with the most benefit. You can view the total upgrade benefit in the application itself. Here are some screenshots as the upgrade progresses.

![path_image](https://github.com/weepingwillowben/city_gen/tree/master/screenshots/upgrade1.PNG)

![path_image](https://github.com/weepingwillowben/city_gen/tree/master/screenshots/upgrade2.PNG)

![path_image](https://github.com/weepingwillowben/city_gen/tree/master/screenshots/upgrade3.PNG)

## Other dreams

Notes for these ideas and some much larger ideas are in `nodes.md`. These are not really meant to be read by people other than myself, but maybe they will be helpful.
