#include "optimization/LoopUnroll.hpp"
#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/SCEV.hpp"

void LoopUnroll::unrollCommonLoop(Loop *loop, LoopTrip trip) {
    if(trip.step % time != 0)
        return;

    
    for(int i=0; i<time; i++) {
        for(BB *bb : loop->getBlocks()) {
            if(bb != loop->getHeader()) {

            }
        }
    }
             
    
}

void LoopUnroll::unrollPartialLoop(Loop *loop) {

}

void LoopUnroll::unrolEntirelLoop(Loop *loop) {

}

void LoopUnroll::removeLoop(Loop *loop) {

}

void LoopUnroll::visitLoop(Loop *loop) {
    SCEV *scev = info_man_->getInfo<SCEV>();
    LoopTrip trip = loop->computeTrip(scev);
    
    if(trip.step < 0) {
        return;
    } else if(trip.step == 0) {
        removeLoop(loop);
        return;
    }

    // ...
    time = 5;
    unrollCommonLoop(loop, trip);

    for(Loop *inner : loop->getInners()) {
        visitLoop(inner);
    }
}

void LoopUnroll::runOnFunc(Function* func) {
    vector<Loop*> loops = info_man_->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops) {
        visitLoop(loop);
    }
}

