#include "optimization/BreakGEP.hpp"
#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"

//oppositeJ一定在mergeJJ前面，肯定是先反转J，再合并，不然合并了怎么反转
Modify BreakGEP::runOnFunc(Function *function){
    breakGEP(getBasicBlocks(function));
    Modify ret{};
    ret.modify_instr=true;
    return ret;
}


::std::vector<BasicBlock*> BreakGEP::getBasicBlocks(Function *function){
    ::std::vector<BasicBlock*> BBs;
        if(!function->isDeclaration())
            for(auto basicblock: function->getBasicBlocks())
                BBs.push_back(basicblock);
    
    return BBs;
}


::std::list<Instruction *>& BreakGEP::getInstList(BasicBlock*bb){
    auto & inst_list = bb->getInstructions();
    return inst_list;
}


void BreakGEP::breakGEP(::std::vector<BasicBlock*> BBs){
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
