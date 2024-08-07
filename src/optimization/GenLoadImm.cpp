#include "optimization/GenLoadImm.hpp"
#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Instruction.hpp"

Modify GenLoadImm::runOnFunc(Function *function){
    if(function->isDeclaration())
        return{};
    Modify ret;
    for(auto b:function->getBasicBlocks()){
        auto ins_list=b->getInstructions();
        for(auto iter=ins_list.begin();iter!=ins_list.end();){
            auto cur_iter=iter;
            auto ins=*cur_iter;
            ++iter;
            if(ins->isCmp()||ins->isFCmp()){
                auto cons=dynamic_cast<Constant*>(ins->getOperand(1));
                if(cons){
                    auto imm=LoadImmInst::createLoadImm(cons->getType(),cons,b);
                    ins_list.pop_back();
                    ins_list.insert(cur_iter,imm);
                    ret.modify_instr=true;
                }
            }else if(ins->isAdd()){
                if(ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(1))){
                    if(cons->getValue()<2048&&cons->getValue()>=-2048){
                        continue;
                    }
                    auto imm=LoadImmInst::createLoadImm(cons->getType(),cons,b);
                    ins_list.pop_back();
                    ins_list.insert(cur_iter,imm);
                    ret.modify_instr=true;
                }
            }else if(ins->isMul()){
                if(ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(1))){
                    auto imm=LoadImmInst::createLoadImm(cons->getType(),cons,b);
                    ins_list.pop_back();
                    ins_list.insert(cur_iter,imm);
                    ret.modify_instr=true;
                }
            }else if(ins->isGep()){

                
            }
        }
    }
    return ret;
}