#include "optimization/UnReachableBBEli.hpp"
#include "optimization/util.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
void UnReachableBBEli::eraseBB(BasicBlock*bb){
    if(bb==bb->getParent()->getEntryBlock())
        return;
    if(!bb->getPreBasicBlocks().empty())
        return;
    if(bb->getTerminator()->isRet())
        exit(223);

    this->erased.insert(bb);
    std::list<BasicBlock*> succbbs=bb->getSuccBasicBlocks();
    bb->getParent()->removeBasicBlock(bb);
    rmBBPhi(bb);
    for(auto succ:succbbs){
        if(erased.count(succ))continue;
        if(succ->getPreBasicBlocks().empty())
            eraseBB(succ);
    }
}
Modify UnReachableBBEli::runOnFunc(Function*func){
    BasicBlock*entry=func->getEntryBlock();
    auto &bbs=func->getBasicBlocks();
    Modify ret{};
    for(auto iter=bbs.begin();iter!=bbs.end();){
        auto b=*iter;
        ++iter;
        if(b==entry)continue;
        if(b->getPreBasicBlocks().empty()){
            eraseBB(b);
            iter=++(bbs.begin());
        }
    }
    std::vector<Instruction*> to_del;
    for(auto b:erased){
        ret.modify_bb=true;
        ret.modify_instr=true;
        std::copy(b->getInstructions().begin(),b->getInstructions().end(),to_del.end());
    }
    for(Instruction* i:to_del){
        i->removeUseOfOps();
    }
    for(auto i:to_del){
        assert(i->getUseList().empty());
        delete i;
    }
    for(auto b:erased){
        delete b;
    }
    return ret;
}
