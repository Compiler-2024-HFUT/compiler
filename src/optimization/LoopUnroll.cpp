#include "optimization/LoopUnroll.hpp"
#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/SCEV.hpp"

void LoopUnroll::unrollCommonLoop(Loop *loop, LoopTrip trip) {
    if(trip.step % UNROLLING_TIME != 0) {
        LOG_WARNING("Unroll using a time that is not a multiple of step!!")
        return;
    }
    
    BB *header = loop->getHeader();
    vector<BB*> exits = loop->getExits();
    BB *latch = loop->getSingleLatch();
    LOG_ERROR("You should LoopSimplified Before Unrolling" , latch == nullptr)

    vector<BB*> entrys = { nullptr };
    vector<BB*> latchs = { latch };
    vector< vector<BB*> > exitings = { exits };

    for(int i = 1; i < UNROLLING_TIME; i++) {
        BB *newEntry = nullptr;
        BB *newLatch = nullptr;
        vector<BB*> newExiting = {};
        
        loop->copyBody(newEntry, newLatch, newExiting);
        entrys.push_back(newEntry);
        latchs.push_back(newLatch);
        exitings.push_back(newExiting);
    }

    for(int i = 0; i < UNROLLING_TIME-1; i++) {
        latchs[i]->removeSuccBasicBlock( header );
        latchs[i]->addSuccBasicBlock( entrys[i+1] );
        entrys[i+1]->removePreBasicBlock( header );
        entrys[i+1]->addPreBasicBlock( latchs[i] );
        
        latchs[i]->getTerminator()->replaceOperand(0, entrys[i+1]);
        
        // entry[i+1]中一定没有phi吗？
        LOG_ERROR("entry has phiInst!", entrys[i+1]->getInstructions().front()->isPhi())
    }
    
    // exits' phi
    

}

void LoopUnroll::unrollPartialLoop(Loop *loop) {
    
}

// 只合并单块(必为latch、唯一入口为header、terminator是无条件跳转)
// 且块内指令数(不包括br)小于DIRECT_UNROLLING_SIZE
void LoopUnroll::unrolEntirelLoop(Loop *loop, LoopTrip trip) {
    if(trip.step > DIRECT_UNROLLING_TIME)
        return;
    
}

void LoopUnroll::removeLoop(Loop *loop) {

}

void LoopUnroll::visitLoop(Loop *loop) {
    SCEV *scev = info_man_->getInfo<SCEV>();
    LoopTrip trip = loop->computeTrip(scev);
    LOG_WARNING(trip.print())

    if(trip.step < 0) {
        return;
    } else if(trip.step == 0) {
        removeLoop(loop);
        return;
    }

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

