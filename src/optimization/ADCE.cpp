#include "optimization/ADCE.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"

void ADCE::run(){
    for (auto func : moudle_->getFunctions()){
        auto &bb_list=func->getBasicBlocks();
        if(bb_list.empty())continue;
        cur_func_=func;
        for(auto bb:func->getBasicBlocks()){
            auto &instr_list=bb->getInstructions();
            for(auto instr_iter=instr_list.begin();instr_iter!=instr_list.end();){
                auto cur_iter=instr_iter++;
                auto instr=*cur_iter;
                if(instr->isCall()||instr->isTerminator()){
                    alive_instr_.insert(instr);
                    work_list_.push_back(instr);
                }else if(instr->getUseList().empty()&&!instr->isWriteMem()){
                    bb->eraseInstr(cur_iter);
                }
            }
        }
        do{
            auto instr=work_list_.back();
            work_list_.pop_back();
            if(instr->isAlloca()||dynamic_cast<GlobalVariable*>(instr)){
                for(auto use:instr->getUseList()){
                    if(auto store=static_cast<StoreInst*>(use.val_)){
                    if(alive_instr_.count(store)) continue;
                        work_list_.push_back(store);
                        alive_instr_.insert(store);
                    }
                }
                continue;
            }
            for(auto val:instr->getOperands()){
                if(auto ins=dynamic_cast<Instruction*>(val)){
                    if(alive_instr_.count(ins)) continue;
                    work_list_.push_back(ins);
                    alive_instr_.insert(ins);
                }
            }
        }while(!work_list_.empty()); 
        for(auto bb:func->getBasicBlocks()){
            auto &instr_list=bb->getInstructions();
            for(auto instr_iter=instr_list.begin();instr_iter!=instr_list.end();){
                auto cur_iter=instr_iter++;
                auto instr=*cur_iter;
                if(alive_instr_.find(instr)==alive_instr_.end()){
                    bb->eraseInstr(cur_iter);
                }
            }
        }
    }
}
