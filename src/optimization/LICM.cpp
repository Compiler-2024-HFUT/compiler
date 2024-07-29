#include "optimization/LICM.hpp"

#include <list>
using std::list;

void LICM::visitLoop(Loop *loop) {
    BB *preheader = loop->getPreheader();
    LOG_ERROR("You should run LoopSimplified before LICM!", !preheader)

    LiveVar *lv = info_man_->getInfo<LiveVar>();
    Dominators *dom = info_man_->getInfo<Dominators>();
    const pair< set<Value*>, set<Value*> > &liveOutOfPreHeader = lv->getLiveVarOut(preheader);

    vector<Instruction*> &invariantsVec = info_man_->getInfo<LoopInvariant>()->getInvariants(loop);
    list<Instruction*> invariantsList = list<Instruction*>(invariantsVec.begin(), invariantsVec.end());
    uset<Instruction*> invariantsUset = uset<Instruction*>(invariantsVec.begin(), invariantsVec.end());

    while(!invariantsList.empty()) {
        Instruction *inv = invariantsList.front();
        BB *parentBB = inv->getParent();
        
        bool whileContinue = false;

        // 检查其所有在invariants里面的op都已经移动了，保证inv支配其所有op，否则移至最后
        for(Value *op : inv->getOperands()) {
            Instruction *opV = dynamic_cast<Instruction*>(op);
            if(invariantsUset.count(opV)) {
                whileContinue = true;
                invariantsList.pop_front();
                invariantsList.push_back(inv);
                break;
            }
        }

        if(whileContinue) {
            continue;
        } else {
            invariantsList.pop_front();
            invariantsUset.erase(inv);
        }

        // inv dominates all its uses, or equivalently, inv is not live-out of its pre-header.
        bool isInvDomAllUse = true;
        if(inv->getType()->isFloatType() && liveOutOfPreHeader.second.count(inv)
        || liveOutOfPreHeader.first.count(inv)) {
            isInvDomAllUse = false;
        }

        // inv's block dominates all  ( loop exits where inv is live-out )
        bool isInvBBDomAllExits = true;
        uset<BB*> invDoms = dom->getDomSet(inv->getParent());
        for(BB *e : loop->getExits()) {
            const pair< set<Value*>, set<Value*> > &liveOutOfExit = lv->getLiveVarOut(e);
            if(inv->getType()->isFloatType() && liveOutOfExit.second.count(inv)
            || inv->getType()->isIntegerType() && liveOutOfExit.first.count(inv)) {
                if(!invDoms.count(e)) {
                    isInvBBDomAllExits = false;
                    break;
                }
            }
        }
        
        if(isInvDomAllUse && isInvBBDomAllExits) {
            parentBB->getInstructions().remove(inv);
            preheader->addInstrBeforeTerminator(inv);
        }
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
