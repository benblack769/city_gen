#include "blocks.h"

blocks::blocks()
{
    for (size_t & r : num_residents.Arr){
        r = (256ULL*rand()) / RAND_MAX;
    }
}
