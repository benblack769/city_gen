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

![path_image](https://raw.githubusercontent.com/benblack769/city_gen/master/screenshots/path_image.PNG)

Note that this version uses taxicab distance and is not directional, so it looks different than the others. This version is stored in the simple_tiles branch.

### Update Algorithm

Code for the upgrade algorithm is all in block_trans.cpp

What I eventually figured out is that I could exactly calculate the best section to invest in next, supposing that this would be the last section I will ever invest in. To me, this was a very nice result, as it is exactly the greedy city planner, who does not look into the future at all, but only the next time step.

So how do you go about doing this? (Explanation assumes knowledge of graph terminology).

Intuitively, I calculated the single source costs from both the source and the destination for all vertexes, and for all edges looked at the benefit investing more in that edge will bring, vs the optimal path. Note that only edges which are nearly as fast the optimal path will have any benefit at all.

Then I combined these benefits from all the commute routes in the grid, and simply invested in the one with the most benefit. You can view the total upgrade benefit in the application itself. Here are some screenshots as the upgrade progresses.

![upgrade1](https://raw.githubusercontent.com/benblack769/city_gen/master/screenshots/upgrade1.PNG)

![upgrade2](https://raw.githubusercontent.com/benblack769/city_gen/master/screenshots/upgrade2.PNG)

![upgrade3](https://raw.githubusercontent.com/benblack769/city_gen/master/screenshots/upgrade3.PNG)

### Algorithm Problems

This algorithm is a greedy algorithm, and as such is quite bad in certain scenarios. The most intractable problem is that if there is a region with no one's source or destination inside it, but many people going around it, and there are sufficiently upgraded roads built around it, then the algorithm will never even consider building inside it, for example, the region with the red x here:

![redx](https://raw.githubusercontent.com/benblack769/city_gen/master/screenshots/redx.PNG)

This is because it will always be more advantageous to invest in a road surrounding the region then in the middle of the region in this greedy algorithm, even though intuitively, we may eventually want it to build across the region because of decreasing returns of investment.

A more perfect version of this algorithm will look further into the future when making these decisions, but it is already kind of slow, so a traditional method like a random forest will probably be impractical.

### Performance

The code prints out timings for various parts of the algorithm onto the console. The algorithmic complexity is `O(p * n^2 * log(n))`, where `n` is the size of the grid, and `p` is the number of routes. So even though my code is parallel across different commute routes, and is reasonably efficient, this becomes really slow for larger grids, which I was interested in.

In order to address this some, I realized that there was a lot of overlap in the algorithm for different people taking long, but similar routes. So I created tiles which spanned many grid points which people would path across instead of the natural tiles. Then, after upgrade benefits are determined, I would figure out which natural tiles is the best to upgrade. While this does not have the same theoretical benefit of being a perfect greedy algorithm, it does work reasonably well, and also is theoretically `O(max(p*log(n)^2,n*log(n)))`, and is actually about 10 times faster for large grids (1000x1000).

Here is an example of how it builds roads. Yellow squares show road construction densities. Note that the roads are primality built according to the shapes of these larger (25x25) tiles.

![tiled_network](https://raw.githubusercontent.com/benblack769/city_gen/master/screenshots/tiled_network.PNG)

This can be enabled by editing `NUM_TIERS` in `constants.h` at line 13. Valid values are 1,2 and 3.

A more complete description can be found in `notes.md` tiling section.

## Other dreams

Notes for these ideas and some much larger ideas are in `notes.md`. These are not really meant to be read by people other than myself, but maybe they will be helpful.
