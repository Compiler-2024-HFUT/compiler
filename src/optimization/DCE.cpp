#include "optimization/DCE.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
//副作用
bool isSeIns(Instruction*ins){
    //全局store有副作用，不能删
    if(ins->isStoreOffset())    return true;
    if(ins->isStore()){
        auto ptr=((StoreInst*)ins)->getLVal();
        if(dynamic_cast<GlobalVariable*>(ptr))
            return true;
        return false;
        //有些函数没有副作用不过先不做了
    }else if(ins->isCall()){
        return true;
    }
    return false;
}
//先找部分有效指令
bool isValidIns(Instruction*ins){
    // if(side_effect_ins_.count(ins))
    //     return true;
    //局部store有效
    if(ins->isStore()|| ins->isStoreOffset())
        return true;
    //跳转和ret有效
    if(ins->isTerminator())
        return true;
    return false;
}
void DCE::dfsReachableInstr(BasicBlock*bb){
    if(visited_bb_.count(bb))
        return;
    visited_bb_.insert(bb);
    for(auto ins:bb->getInstructions()){
        if(isSeIns(ins)){
            // side_effect_ins_.insert(ins);
            valid_ins_.insert(ins);
            work_list_.push_back(ins);
        }else if(isValidIns(ins)){
            valid_ins_.insert(ins);
            // side_effect_ins_.insert(ins);
            work_list_.push_back(ins);
        }
    }

    for(auto succ:bb->getSuccBasicBlocks())
        dfsReachableInstr(succ);
}
void DCE::initInfo(Function*func){
    this->clear();
    dfsReachableInstr(func->getEntryBlock());
    //此时valid只有terminator和storecall指令
    while(!work_list_.empty()){
        Instruction* ins=work_list_.back();
        work_list_.pop_back();
        auto &ops=ins->getOperands();
        for(auto op:ops){
            if(auto op_ins=dynamic_cast<Instruction*>(op)){
                if(valid_ins_.count(op_ins)) continue;
                else{
                    work_list_.push_back(op_ins);
                    valid_ins_.insert(op_ins);
                }
            }
        }
    }
}
Modify DCE::runOnFunc(Function*func){
    Modify ret{};
    if(func->isDeclaration())
        return ret;
    this->initInfo(func);
    for(auto b:func->getBasicBlocks()){
        auto &instrs=b->getInstructions();
        for(auto _iter=instrs.begin();_iter!=instrs.end();){
            auto curiter=_iter++;
            if(!valid_ins_.count(*curiter)){
                ret.modify_instr=true;
                b->eraseInstr(curiter);
            }
        }
    }
    return ret;
}
