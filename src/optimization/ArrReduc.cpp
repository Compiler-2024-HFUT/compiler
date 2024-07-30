#include "optimization/ArrReduc.hpp"
#include "analysis/Info.hpp"
#include "midend/Instruction.hpp"
Modify ArrReduc::runOnFunc(Function*func){
    if(func->isDeclaration())
        return {};
    visited.clear();
    return runOnBB(func->getEntryBlock(),{});
}
Modify ArrReduc::runOnBB(BasicBlock*bb,std::map<GetElementPtrInst*,LoadInst*> gep_load){
    Modify ret;
    visited.insert(bb);
    auto &ins_list=bb->getInstructions();
    for(auto __iter=ins_list.begin();__iter!=ins_list.end();){
        auto cur_iter=__iter++;
        auto instr=*cur_iter;
        if(instr->isLoad()){
            auto load=(LoadInst*)instr;
            auto lval=load->getLVal();
            if(auto gep=dynamic_cast<GetElementPtrInst*>(lval)){
                auto gep_iter=gep_load.find(gep);
                if(gep_iter!=gep_load.end()){
                    load->replaceAllUseWith(gep_iter->second);
                    __iter=ins_list.erase(cur_iter);
                    ret.modify_instr=true;
                }else{
                    gep_load.insert({gep,load});
                }
            }
        }else if(instr->isStore()){
            auto store=(StoreInst*)instr;
            auto lval=store->getLVal();
            if(auto gep=dynamic_cast<GetElementPtrInst*>(lval)){
                auto gep_iter=gep_load.find(gep);
                if(gep_iter!=gep_load.end()){
                    gep_load.erase(gep_iter);
                }
            }
        }
    }
    for(auto succ_bb:bb->getSuccBasicBlocks()){
        if(visited.count(succ_bb))
            continue;
        if(succ_bb->getPreBasicBlocks().size()==1&&succ_bb->getPreBasicBlocks().front()==bb){
            ret=ret|runOnBB(succ_bb,gep_load);
        }else{
            ret=ret|runOnBB(succ_bb,{});
        }
    }
    return ret;
}
