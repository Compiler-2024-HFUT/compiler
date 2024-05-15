#include "backend/LIR.hpp"

void LIR::run(){

    downGEP(getBasicBlocks(moudle_));
    oppositeJ(getBasicBlocks(moudle_));
    offset(getBasicBlocks(moudle_));
    mergeJJ(getBasicBlocks(moudle_));
}




::std::vector<BasicBlock*> LIR::getBasicBlocks(Module* m){
    ::std::vector<BasicBlock*> BBs;
    for(auto function: m->getFunctions()){
        if(!function->isDeclaration())
            for(auto basicblock: function->getBasicBlocks())
                BBs.push_back(basicblock);
    }
    return BBs;
}



void LIR::mergeJJ(::std::vector<BasicBlock*> BBs){
    for(auto bb :BBs)
        if(bb->getTerminator()->isBr()){
            auto inst_br = dynamic_cast<BranchInst *>(bb->getTerminator());
            if(inst_br->isCondBr())
                if(dynamic_cast<Instruction *>(inst_br->getOperand(0))){
                    if(dynamic_cast<Instruction *>(inst_br->getOperand(0))->isCmp())
                        mergeIJJ(inst_br, dynamic_cast<CmpInst *>(dynamic_cast<Instruction *>(inst_br->getOperand(0))), bb);
                    else if(dynamic_cast<Instruction *>(inst_br->getOperand(0))->isFCmp())
                        mergeFJJ(inst_br, dynamic_cast<FCmpInst *>(dynamic_cast<Instruction *>(inst_br->getOperand(0))), bb);
                    else std::cerr<<"既不是cmp也不是fcmp！！！"<<std::endl;
                }
                
            
        }
            
            
} 

void LIR::mergeIJJ(BranchInst* inst_br, CmpInst* inst_cmp, BasicBlock* bb){}

void LIR::mergeFJJ(BranchInst* inst_br, FCmpInst* inst_fcmp, BasicBlock* bb){}

void LIR::oppositeJ(::std::vector<BasicBlock*> BBs){}

void LIR::downGEP(::std::vector<BasicBlock*> BBs){


}

void LIR::offset(::std::vector<BasicBlock*> BBs){}