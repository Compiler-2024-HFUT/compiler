#include "optimization/MemInstOffset.hpp"
#include "midend/BasicBlock.hpp"

//oppositeJ一定在mergeJJ前面，肯定是先反转J，再合并，不然合并了怎么反转
Modify MemInstOffset::runOnFunc(Function *function){
    makeOffset(getBasicBlocks(function));
    Modify ret{};
    ret.modify_instr=true;
    return ret;
}




::std::vector<BasicBlock*> MemInstOffset::getBasicBlocks(Function *function){
    ::std::vector<BasicBlock*> BBs;
        if(!function->isDeclaration())
            for(auto basicblock: function->getBasicBlocks())
                BBs.push_back(basicblock);
    
    return BBs;
}


::std::list<Instruction *>& MemInstOffset::getInstList(BasicBlock*bb){
    auto & inst_list = bb->getInstructions();
    return inst_list;
}

void MemInstOffset::makeOffset(::std::vector<BasicBlock*> BBs){
    for(auto bb:BBs){
        auto &inst_list = getInstList(bb);
        for(auto iter=inst_list.begin(); iter!=inst_list.end(); iter++)
            if((*iter)->isLoad())   makeOffsetT(dynamic_cast<LoadInst*>(*iter),inst_list, iter, bb);
            else if((*iter)->isStore()) makeOffsetT(dynamic_cast<StoreInst*>(*iter), inst_list, iter, bb);

        
    }
}

template <class T>
void MemInstOffset::makeOffsetT(T*inst_load_or_store, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock*bb){
    if constexpr (std::is_same_v<T, LoadInst>){
        auto lval = inst_load_or_store->getLVal();
        if(dynamic_cast<GetElementPtrInst*>(lval))  handleGEP(dynamic_cast<GetElementPtrInst*>(lval), inst_list, inst_pos, bb, 0);
        else if(dynamic_cast<BinaryInst*>(lval))   handleAdd(dynamic_cast<BinaryInst*>(lval), inst_list, inst_pos, bb, 0);
    }
    else{
        auto lval = inst_load_or_store->getLVal();
        if(dynamic_cast<GetElementPtrInst*>(lval))  handleGEP(dynamic_cast<GetElementPtrInst*>(lval), inst_list, inst_pos, bb, 1);
        else if(dynamic_cast<BinaryInst*>(lval))   handleAdd(dynamic_cast<BinaryInst*>(lval), inst_list, inst_pos, bb, 1);
    }
}

void MemInstOffset::handleGEP(GetElementPtrInst* inst_gep, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb, bool flag){
    Value *base, *offset;
    auto inst = *inst_pos;
    base = inst_gep;
    offset = inst_gep->getOperand(inst_gep->getNumOperands()-1);
    if(!flag) {
        auto inst_offset =  LoadOffsetInst::createLoadOffset(base->getType()->getPointerElementType(), base, offset, bb);
        inst_list.pop_back();
        bb->addInstruction(inst_pos--, inst_offset);
        inst->replaceAllUseWith(inst_offset);
        bb->deleteInstr(inst);

    }
    else {
        auto inst_offset = StoreOffsetInst::createStoreOffset(dynamic_cast<StoreInst*>(inst)->getRVal(), base, offset, bb);
        inst_list.pop_back();
        bb->addInstruction(inst_pos--, inst_offset);
        inst->replaceAllUseWith(inst_offset);
        bb->deleteInstr(inst);

    }
     
   
    
}
void MemInstOffset::handleAdd(BinaryInst* inst_ptr, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb, bool flag){
    Value *base, *offset;
    auto inst = *inst_pos;
    if(inst_ptr->isAdd()){
        base = inst_ptr->getOperand(0);
        auto oper = inst_ptr->getOperand(1);
        if(dynamic_cast<Instruction*>(oper)->isMul() || dynamic_cast<Instruction*>(oper)->isLsl())
            offset = dynamic_cast<Instruction*>(oper)->getOperand(0);
        
    }

    if(!flag) {
        auto inst_offset =  LoadOffsetInst::createLoadOffset(base->getType()->getPointerElementType(), base, offset, bb);
        inst_list.pop_back();
        bb->addInstruction(inst_pos--, inst_offset);
        inst->replaceAllUseWith(inst_offset);
        bb->deleteInstr(inst);
       auto inst_add = *inst_pos;

    }
    else {
        auto inst_offset = StoreOffsetInst::createStoreOffset(dynamic_cast<StoreInst*>(inst)->getOperand(0), base, offset, bb);
        inst_list.pop_back();
        bb->addInstruction(inst_pos--, inst_offset);
        inst->replaceAllUseWith(inst_offset);
        bb->deleteInstr(inst);


 
    }

}