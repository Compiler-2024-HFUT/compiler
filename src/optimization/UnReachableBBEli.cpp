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
    erased.clear();
    std::set<BasicBlock*>reachable;
    std::list<BasicBlock*>work{func->getEntryBlock()};
    while(!work.empty()){
        auto b=work.front();
        work.pop_front();
        reachable.insert(b);
        for(auto s:b->getSuccBasicBlocks()){
            if(reachable.count(s))
                continue;
            work.push_back(s);
            reachable.insert(s);
        }
    }
    std::vector<Instruction*> to_del;
    auto toeraselist=func->getBasicBlocks();
    for(auto b:toeraselist){
        if(reachable.count(b))
            continue;
        func->removeBasicBlock(b);
        rmBBPhi(b);
        erased.insert(b);
        to_del.insert(to_del.end(),b->getInstructions().begin(),b->getInstructions().end());
    }
    for(Instruction* i:to_del){
        i->removeUseOfOps();
    }
    for(auto i:to_del){
        assert(i->getUseList().empty());
        delete i;
    }
    if(!erased.empty()){
        ret.modify_bb=true;
        ret.modify_instr=true;
    }
    for(auto b:erased){
        delete b;
    }
    return ret;
}
