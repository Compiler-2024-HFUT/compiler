#include "optimization/Mem2Reg.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include <iostream>
#include <memory>

void Mem2Reg::run(){
    for (auto func : moudle_->getFunctions()){
        auto &bb_list=func->getBasicBlocks();
        if(bb_list.empty())continue;

        ::std::unique_ptr<Dominators> dom=std::make_unique<Dominators>(func);
        cur_fun_dom=func_dom_.insert({func,std::move(dom)}).first;

        removeSL();
        // for(auto bb:func->getBasicBlocks()){
        //     removeOneBBLoad(bb);
        
        // };
        generatePhi();
    
    }
}
bool Mem2Reg::isLocalVarOp(Instruction *instr){
    if (instr->isStore()){
        StoreInst *store_instr = (StoreInst *)instr;
        auto l_value = store_instr->getLVal();
        auto alloc = dynamic_cast<AllocaInst *>(l_value);
        auto array_element_ptr = dynamic_cast<GetElementPtrInst *>(l_value);
        return alloc!=nullptr && array_element_ptr==nullptr;
    }else if (instr->isLoad()){
        LoadInst *load_instr = (LoadInst *)instr;
        auto l_value = load_instr->getLVal();
        auto alloc = dynamic_cast<AllocaInst *>(l_value);
        auto array_element_ptr = dynamic_cast<GetElementPtrInst *>(l_value);
        return alloc!=nullptr && array_element_ptr==nullptr;
    }else
        return false;
}
bool Mem2Reg::isAllocVar(Instruction *instr){
    if(instr->isAlloca()){
        AllocaInst *alloc=static_cast<AllocaInst*>(instr);
        return alloc->getType()->isFloatType()||alloc->getType()->isIntegerType();
    }else
        return false;

}
void Mem2Reg::removeSL(){
    ::std::map<Value *, Instruction *> define_list;
    ::std::map<Value *, Value *> new_value;
    for(auto bb:cur_fun_dom->first->getBasicBlocks())
        removeOne(bb,define_list,new_value);
}
void Mem2Reg::removeOne(BasicBlock*cur_bb,map<Value *, Instruction *> define_list,map<Value *, Value *> new_value){
    ::std::map<Instruction *, Value *> use_list;
    for(Instruction* instr : cur_bb->getInstructions()){
        if(!isLocalVarOp(instr)) continue;
        else
        if(instr->isStore()){
            auto store_ins=(StoreInst*)instr;
            auto l_val=store_ins->getLVal();
            auto r_val=store_ins->getRVal();
            auto r_val_load_instr = dynamic_cast<Instruction*>(r_val);
            if(r_val_load_instr!=nullptr)
                if(auto it=use_list.find(r_val_load_instr); it!= use_list.end())
                    r_val = it->second;
            
            new_value[l_val]=r_val;

            //remove when redefine in one block
            if(auto it_pair=define_list.find(l_val);it_pair != define_list.end()){
                cur_bb->deleteInstr(it_pair->second);
                it_pair->second = store_ins;
            }else{
                define_list.insert({l_val, store_ins});
            }

        }else if(instr->isLoad()) {
            LoadInst* load_ins=static_cast<LoadInst *>(instr);
            Value* l_val = static_cast<LoadInst *>(instr)->getLVal();
            Value* r_val = dynamic_cast<Value *>(instr);
            if(define_list.find(l_val) == define_list.end())
                continue;
            
            Value* value = new_value.find(l_val)->second;
            use_list.insert({load_ins, value});
        }

    }
    
    for(auto [load_instr ,value]: use_list){
        for(auto use: load_instr->getUseList()){
            Instruction * use_inst = dynamic_cast<Instruction *>(use.val_);
            use_inst->setOperand(use.arg_no_, value);
        }
        cur_bb->deleteInstr(load_instr);
        // use_list.erase(load_instr);
    }

    auto is_df=[this](BasicBlock*bb){
        for(auto b:cur_fun_dom->first->getBasicBlocks()){
            auto dfs=b->getDomFrontier();
            for(auto df:dfs)
                if(df==bb)return true; 
        }
        return false;
    };

    for (BasicBlock* succ_bb : cur_bb->getSuccBasicBlocks() ){
        if(!is_df(succ_bb))
            removeOne(succ_bb,define_list,new_value);
    }
}

void Mem2Reg::generatePhi(){
}