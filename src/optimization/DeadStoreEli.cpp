#include "optimization/DeadStoreEli.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>

void DeadStoreEli::run(){
    for (auto func : moudle_->getFunctions()){
        auto &bb_list=func->getBasicBlocks();
        if(bb_list.empty())continue;
        cur_func_=func;
        rmStore();

    }
}
bool DeadStoreEli::isAllocVar(Instruction *instr){
    if(instr->isAlloca()){
        AllocaInst *alloc=static_cast<AllocaInst*>(instr);
        return alloc->getAllocaType()->isFloatType()||alloc->getAllocaType()->isIntegerType();
    }else
        return false;

}
void DeadStoreEli::rmStore(){
    std::list<std::list<Instruction *>::iterator >dead;
    std::unordered_map<Value*,std::list<Instruction *>::iterator>dead_allocs;

    for(auto bb:cur_func_->getBasicBlocks())
        for(auto instr_iter=bb->getInstructions().begin();instr_iter!=bb->getInstructions().end();){
        auto cur_iter=instr_iter;
        auto instr=*instr_iter;instr_iter++;
        if(isAllocVar(instr)&&instr->getUseList().size()<2){
            dead_allocs.insert({instr,cur_iter});
        }else if(instr->isStore()){
            if(auto alloc_iter=dead_allocs.find(static_cast<StoreInst*>(instr)->getLVal());alloc_iter != dead_allocs.end()){
                static_cast<StoreInst*>(instr)->getRVal()->removeUse(instr);
                dead.push_back(cur_iter);
                bb->getInstructions().erase(cur_iter);
                delete (instr);
                
                dead_allocs.erase(alloc_iter);
                auto alloc=static_cast<AllocaInst*>(alloc_iter->first);
                alloc->getParent()->getInstructions().erase(alloc_iter->second);
                delete (alloc);
            }
        }
    }
}