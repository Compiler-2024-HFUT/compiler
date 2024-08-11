#include "analysis/Info.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include "optimization/GepCombine.hpp"
#include <vector>
//after vn breakgep;
GetElementPtrInst* __isBeGepUseEq(Instruction*ins,Value*ptr){
    for(auto [u,i]:ins->getUseList()){
        if(auto gep=dynamic_cast<GetElementPtrInst*>(u)){
            if(gep->getNumOperands()!=2)
                continue;
            if(gep->getOperand(0)==ptr)
                return gep;
        }
    }
    return 0;
}
Modify GepCombine::runOnFunc(Function*func){
    if(func->isDeclaration())
        return{};
    for(auto b:func->getBasicBlocks()){
        for(auto ins:b->getInstructions()){
            if(ins->isGep()&&ins->getNumOperands()==2){
                work_set_.push_back((GetElementPtrInst*)ins);
            }
        }
    }
    while(!work_set_.empty()){
        auto gep=work_set_.back();
        work_set_.pop_back();
        Value* offset=gep->getOperands().back();
        if(auto add=dynamic_cast<Instruction*>(offset);add&&add->isAdd()){
            if(dynamic_cast<ConstantInt*>(add->getOperand(1))){
                if(auto lhs=dynamic_cast<Instruction*>(add->getOperand(0))){
                    auto gep_has_offset=__isBeGepUseEq(lhs,gep->getOperand(0));
                    if(gep_has_offset==0)
                        continue;
                    auto bb=gep->getParent();
                    auto new_gep=GetElementPtrInst::createGep(gep_has_offset,{add->getOperand(1)},bb);
                    gep->replaceAllUseWith(new_gep);
                    bb->getInstructions().pop_back();
                    bb->insertInstr(bb->findInstruction(gep),new_gep);
                }
                
            }
        }

    }
    return {};
}