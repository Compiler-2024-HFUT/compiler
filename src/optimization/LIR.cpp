#include "optimization/LIR.hpp"
#include "midend/BasicBlock.hpp"

//oppositeJ一定在mergeJJ前面，肯定是先反转J，再合并，不然合并了怎么反转
void LIR::runOnFunc(Function *function){

    breakGEP(getBasicBlocks(function));
    oppositeJ(getBasicBlocks(function));
    makeOffset(getBasicBlocks(function));
    mergeJJ(getBasicBlocks(function));
}




::std::vector<BasicBlock*> LIR::getBasicBlocks(Function *function){
    ::std::vector<BasicBlock*> BBs;
        if(!function->isDeclaration())
            for(auto basicblock: function->getBasicBlocks())
                BBs.push_back(basicblock);
    
    return BBs;
}


::std::list<Instruction *>& LIR::getInstList(BasicBlock*bb){
    auto & inst_list = bb->getInstructions();
    return inst_list;
}
void LIR::mergeJJ(::std::vector<BasicBlock*> BBs){
    for(auto bb :BBs)
        if(bb->getTerminator()->isBr()){
            auto inst_br = dynamic_cast<BranchInst *>(bb->getTerminator());
            if(inst_br->isCondBr())
                if(dynamic_cast<Instruction *>(inst_br->getOperand(0))){
                    if(dynamic_cast<Instruction *>(inst_br->getOperand(0))->isCmp())
                        mergeTJJ(inst_br, dynamic_cast<CmpInst *>(dynamic_cast<Instruction *>(inst_br->getOperand(0))), bb);
                    else if(dynamic_cast<Instruction *>(inst_br->getOperand(0))->isFCmp())
                        mergeTJJ(inst_br, dynamic_cast<FCmpInst *>(dynamic_cast<Instruction *>(inst_br->getOperand(0))), bb);
                  //  else std::cerr<<"既不是cmp也不是fcmp！！！"<<std::endl;
                }
                
            
        }
            
            
} 

template <class T>
void LIR::mergeTJJ(BranchInst* inst_br, T* inst_I_or_F_cmp, BasicBlock* bb){
    auto true_bb = dynamic_cast<BasicBlock*>(inst_br->getOperands()[1]);
    auto false_bb = dynamic_cast<BasicBlock*>(inst_br->getOperands()[2]);
    auto cmp_operator = inst_I_or_F_cmp->getCmpOp();
    auto cmp_operands = inst_I_or_F_cmp->getOperands();
    bb->removeSuccBasicBlock(true_bb);
    bb->removeSuccBasicBlock(false_bb);
    true_bb->removePreBasicBlock(bb);
    false_bb->removePreBasicBlock(bb);
    if constexpr (std::is_same_v<T, CmpInst>) CmpBrInst::createCmpBr(cmp_operator, cmp_operands[0], cmp_operands[1], true_bb, false_bb, bb);
    else  FCmpBrInst::createFCmpBr(cmp_operator, cmp_operands[0], cmp_operands[1], true_bb, false_bb, bb);
   
    bb->deleteInstr(inst_I_or_F_cmp);
    bb->deleteInstr(inst_br);

}



void LIR::oppositeJ(::std::vector<BasicBlock*> BBs){
    for(auto bb:BBs){
        auto& inst_list = getInstList(bb);

        for(auto iter = inst_list.begin(); iter!=inst_list.end(); iter++)
     
            if((*iter)->isCmp())    oppositeTJ(inst_list, dynamic_cast<CmpInst*>(*iter), iter, bb);
            else if((*iter)->isFCmp())  oppositeTJ(inst_list, dynamic_cast<FCmpInst*>(*iter), iter, bb);


    }
}
template <class T>
void LIR::oppositeTJ(::std::list<Instruction*>& inst_list, T* inst_I_or_F_cmp, ::std::list<Instruction*>::iterator &inst_pos, BasicBlock*bb){
    auto cmp_operator = inst_I_or_F_cmp->getCmpOp();
    if constexpr (std::is_same_v<T, CmpInst>){
        switch(cmp_operator){
            case CmpOp::GT:{
                auto inst_oppo_cmp = CmpInst::createCmp(CmpOp::LT, inst_I_or_F_cmp->getOperand(1), inst_I_or_F_cmp->getOperand(0), bb);
                inst_list.pop_back();   //注意！！！删除由于createCmp函数额外插入的语句
                bb->addInstruction(inst_pos, inst_oppo_cmp); 
                inst_pos--;
                inst_I_or_F_cmp->replaceAllUseWith(inst_oppo_cmp);
                bb->deleteInstr(inst_I_or_F_cmp);
                break;
            }
            case CmpOp::LE:{
                auto inst_oppo_cmp = CmpInst::createCmp(CmpOp::GE, inst_I_or_F_cmp->getOperand(1), inst_I_or_F_cmp->getOperand(0), bb);
                inst_list.pop_back();   //注意！！！删除由于createCmp函数额外插入的语句
                bb->addInstruction(inst_pos, inst_oppo_cmp); 
                inst_pos--;
                inst_I_or_F_cmp->replaceAllUseWith(inst_oppo_cmp);
                bb->deleteInstr(inst_I_or_F_cmp);
                break;
            }
        }
    }
    else{
        switch(cmp_operator){
            case CmpOp::GT:{
                auto inst_oppo_fcmp = FCmpInst::createFCmp(CmpOp::LT, inst_I_or_F_cmp->getOperand(1), inst_I_or_F_cmp->getOperand(0), bb);
                inst_list.pop_back();   //注意！！！删除由于createCmp函数额外插入的语句
                bb->addInstruction(inst_pos, inst_oppo_fcmp); 
                inst_pos--;
                inst_I_or_F_cmp->replaceAllUseWith(inst_oppo_fcmp);
                bb->deleteInstr(inst_I_or_F_cmp);
                break;
            }
            case CmpOp::LE:{
                auto inst_oppo_fcmp = FCmpInst::createFCmp(CmpOp::GE, inst_I_or_F_cmp->getOperand(1), inst_I_or_F_cmp->getOperand(0), bb);
                inst_list.pop_back();   //注意！！！删除由于createCmp函数额外插入的语句
                bb->addInstruction(inst_pos, inst_oppo_fcmp); 
                inst_pos--;
                inst_I_or_F_cmp->replaceAllUseWith(inst_oppo_fcmp);
                bb->deleteInstr(inst_I_or_F_cmp);
                break;
            }
        }

    }
}



void LIR::breakGEP(::std::vector<BasicBlock*> BBs){
    for(auto bb : BBs){
        auto & inst_list = getInstList(bb);
        for(auto iter=inst_list.begin(); iter!=inst_list.end(); iter++){
            auto inst_gep = *iter;
            if(inst_gep->isGep() &&(inst_gep->getOperand(inst_gep->getNumOperands()-1)!=ConstantInt::get(0))){//后一个条件的目的是：后一个参数不为0的gep才行，避免对初始化做修改
                auto size = ConstantInt::get(inst_gep->getType()->getPointerElementType()->getSize());
                //gep指令的格式不是固定的
                int offset_op = inst_gep->getNumOperands()-1;
                auto offset = inst_gep->getOperand(offset_op);   //取偏移量
                inst_gep->removeOperands(offset_op,offset_op);   //删除偏移量
                inst_gep->addOperand(ConstantInt::get(0));   //追加 
                auto inst_mul_offset = BinaryInst::createMul(offset, size, bb); //计算偏移量的指令（offset*element_size）
                bb->addInstruction(++iter, inst_list.back());
                inst_list.pop_back();   //消除createMul函数的副作用
               
                
                auto inst_first_address_array_add = BinaryInst::createAdd(inst_gep, inst_mul_offset, bb);
                bb->addInstruction(iter--,  inst_list.back());
                inst_list.pop_back();
               
             inst_gep->removeUse(inst_first_address_array_add);
             inst_gep->replaceAllUseWith(inst_first_address_array_add);
                inst_gep->getUseList().clear();
                inst_gep->addUse(inst_first_address_array_add);



            }
        }
    }

}

void LIR::makeOffset(::std::vector<BasicBlock*> BBs){
    for(auto bb:BBs){
        auto &inst_list = getInstList(bb);
        for(auto iter=inst_list.begin(); iter!=inst_list.end(); iter++)
            if((*iter)->isLoad())   makeOffsetT(dynamic_cast<LoadInst*>(*iter),inst_list, iter, bb);
            else if((*iter)->isStore()) makeOffsetT(dynamic_cast<StoreInst*>(*iter), inst_list, iter, bb);

        
    }
}

template <class T>
void LIR::makeOffsetT(T*inst_load_or_store, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock*bb){
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

void LIR::handleGEP(GetElementPtrInst* inst_gep, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb, bool flag){
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
void LIR::handleAdd(BinaryInst* inst_ptr, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb, bool flag){
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
       inst_pos--;
    
       bb->deleteInstr(inst_add);
       auto inst_mul = *inst_pos;
       inst_pos--;
    
       bb->deleteInstr(inst_mul);
    }
    else {
        auto inst_offset = StoreOffsetInst::createStoreOffset(dynamic_cast<StoreInst*>(inst)->getOperand(0), base, offset, bb);
        inst_list.pop_back();
        bb->addInstruction(inst_pos--, inst_offset);
        inst->replaceAllUseWith(inst_offset);
        bb->deleteInstr(inst);


 
    }

}