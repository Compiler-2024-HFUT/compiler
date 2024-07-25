#include "optimization/CombineJJ.hpp"
#include "midend/BasicBlock.hpp"

//oppositeJ一定在mergeJJ前面，肯定是先反转J，再合并，不然合并了怎么反转
Modify CombineJJ::runOnFunc(Function *function){
    oppositeJ(getBasicBlocks(function));
    mergeJJ(getBasicBlocks(function));
    return {};
}




::std::vector<BasicBlock*> CombineJJ::getBasicBlocks(Function *function){
    ::std::vector<BasicBlock*> BBs;
        if(!function->isDeclaration())
            for(auto basicblock: function->getBasicBlocks())
                BBs.push_back(basicblock);
    return BBs;
}


::std::list<Instruction *>& CombineJJ::getInstList(BasicBlock*bb){
    auto & inst_list = bb->getInstructions();
    return inst_list;
}

void CombineJJ::mergeJJ(::std::vector<BasicBlock*> BBs){
    for(auto bb :BBs)
        if(bb->getTerminator()->isBr()){
            auto inst_br = dynamic_cast<BranchInst *>(bb->getTerminator());
            if(inst_br->isCondBr()){
                if(dynamic_cast<Instruction *>(inst_br->getOperand(0)) && dynamic_cast<Instruction *>(inst_br->getOperand(0))->getUseList().size()<2){
                    if(dynamic_cast<Instruction *>(inst_br->getOperand(0))->isCmp())
                        mergeTJJ(inst_br, dynamic_cast<CmpInst *>(dynamic_cast<Instruction *>(inst_br->getOperand(0))), bb);
                    else if(dynamic_cast<Instruction *>(inst_br->getOperand(0))->isFCmp())
                        mergeTJJ(inst_br, dynamic_cast<FCmpInst *>(dynamic_cast<Instruction *>(inst_br->getOperand(0))), bb);
                    else std::cerr<<"既不是cmp也不是fcmp"<<std::endl;
                }
            }
            
        }
            
            
} 

template <class T>
void CombineJJ::mergeTJJ(BranchInst* inst_br, T* inst_I_or_F_cmp, BasicBlock* bb){
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



void CombineJJ::oppositeJ(::std::vector<BasicBlock*> BBs){
    for(auto bb:BBs){
        auto& inst_list = getInstList(bb);
        for(auto iter = inst_list.begin(); iter!=inst_list.end(); iter++)
            if((*iter)->isCmp())    oppositeTJ(inst_list, dynamic_cast<CmpInst*>(*iter), iter, bb);
            else if((*iter)->isFCmp())  oppositeTJ(inst_list, dynamic_cast<FCmpInst*>(*iter), iter, bb);
    }
}
template <class T>
void CombineJJ::oppositeTJ(::std::list<Instruction*>& inst_list, T* inst_I_or_F_cmp, ::std::list<Instruction*>::iterator &inst_pos, BasicBlock*bb){
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
