#include "optimization/LICM.hpp"

void LICM::visitLoop(Loop *loop) {
    BB *preheader = loop->getPreheader();
    LOG_ERROR("You should run LoopSimplified before LICM!", !preheader)

    vector<Instruction*> invariants = info_man_->getInfo<LoopInvariant>()->getInvariants(loop);
    for(auto iter = invariants.rbegin(); iter != invariants.rend(); iter++) {
        Instruction *inv = *iter;
        BB *parentBB = inv->getParent();
        parentBB->getInstructions().remove(inv);
        preheader->addInstrBeforeTerminator(inv);
    }

    // 暂不考虑子循环
    // for(Loop *inner : loop->getInners()) {
    //     visitLoop(inner);
    // }
}

void LICM::runOnFunc(Function* func) {
    vector<Loop*> loops = info_man_->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops) {
        visitLoop(loop);
    }
}
