#include "optimization/DeadStoreEli.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include <unordered_map>

Modify DeadStoreEli::runOnFunc(Function*func){
    auto &bb_list=func->getBasicBlocks();
    if(bb_list.empty())return {};
    cur_func_=func;
    rmStore();
    return {};
}
bool DeadStoreEli::isAllocVar(Instruction *instr){
    if(instr->isAlloca()){
        AllocaInst *alloc=static_cast<AllocaInst*>(instr);
        return alloc->getAllocaType()->isFloatType()||alloc->getAllocaType()->isIntegerType();
    }else
        return false;

}
void DeadStoreEli::rmStore(){
    std::unordered_map<Value*,std::list<Instruction *>::iterator>dead_allocs;

    for(auto bb:cur_func_->getBasicBlocks())
        for(auto instr_iter=bb->getInstructions().begin();instr_iter!=bb->getInstructions().end();){
        auto cur_iter=instr_iter;
        auto instr=*instr_iter;instr_iter++;
        if(isAllocVar(instr)){
            bool all_store=true;
            for(auto use:instr->getUseList()){
                auto use_ins=dynamic_cast<Instruction*>(use.val_);
                assert(use_ins&&"alloca use isnot instruction");
                if(!use_ins->isStore())
                    all_store=false;
            }
            if(all_store)
                dead_allocs.insert({instr,cur_iter});
        }else if(instr->isStore()){
            if(auto alloc_iter=dead_allocs.find(static_cast<StoreInst*>(instr)->getLVal());alloc_iter != dead_allocs.end()){
                static_cast<StoreInst*>(instr)->getRVal()->removeUse(instr);
                bb->getInstructions().erase(cur_iter);
                delete (instr);
            }
        }
    }
    for(auto alloc_iter:dead_allocs){
        auto alloc=static_cast<AllocaInst*>(alloc_iter.first);
        alloc->getParent()->getInstructions().erase(alloc_iter.second);
        delete (alloc);
    }
}