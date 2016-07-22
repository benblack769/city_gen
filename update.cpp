#include "update.h"
#include "RangeIterator.h"

void update_block_info(blocks & bks,firms & fms,people & pps){
    for(size_t pn : range(pps.size())){
        bks.num_residents[pps.home[pn]]++;
        bks.num_workers[pps.work[pn]]++;
    }
}

void update_info(blocks & bks,firms & fms,people & pps){
    
    update_block_info(bks,fms,pps);
}
