#include "optimization/GenLoadImm.hpp"
#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Instruction.hpp"

Modify GenLoadImm::runOnFunc(Function *function){
    if(function->isDeclaration())
        return{};
    Modify ret;
//     auto islog2=[](int num){
//     if(num<=0){
//         return 0;
//     }
//     int ret=0;
//     int curnum=1;
//     while(curnum<num){
//         curnum=curnum<<1;
//         ++ret;
//     }
//     if(curnum!=num)
//         return 0;
//     return ret;
// };
    for(auto b:function->getBasicBlocks()){
        auto &ins_list=b->getInstructions();
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
                    ins->replaceOperand(1,imm);
                }
                else if(dynamic_cast<Constant*>(ins->getOperand(0))){
                    cons=dynamic_cast<Constant*>(ins->getOperand(0));
                    auto imm=LoadImmInst::createLoadImm(cons->getType(),cons,b);
                    ins_list.pop_back();
                    ins_list.insert(cur_iter,imm);
                    ret.modify_instr=true;
                    ins->replaceOperand(0,imm);
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
                    ins->replaceOperand(1,imm);
                }
            }else if(ins->isMul()){
                if(ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(1))){
                    auto imm=LoadImmInst::createLoadImm(cons->getType(),cons,b);
                    ins_list.pop_back();
                    ins_list.insert(cur_iter,imm);
                    ret.modify_instr=true;
                    ins->replaceOperand(1,imm);
                }
            }else if(ins->isGep()){
                if(ins->getNumOperands()==2){
                    ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(1));
                    if(cons==0)
                        continue;
                    if(cons->getValue()>511||cons->getValue()<-512){
                        auto imm=LoadImmInst::createLoadImm(cons->getType(),cons,b);
                        ins_list.pop_back();
                        ins_list.insert(cur_iter,imm);
                        ret.modify_instr=true;
                        ins->replaceOperand(1,imm);
                    }
                }else if(ins->getNumOperands()==3){
                    ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(2));
                    if(cons==0)
                        continue;
                    if(cons->getValue()>511||cons->getValue()<-512){
                        auto imm=LoadImmInst::createLoadImm(cons->getType(),cons,b);
                        ins_list.pop_back();
                        ins_list.insert(cur_iter,imm);
                        ret.modify_instr=true;
                        ins->replaceOperand(2,imm);
                    }
                }
            }else if(ins->isFAdd()||ins->isFMul()){
                auto cons=dynamic_cast<Constant*>(ins->getOperand(1));
                if(cons){
                    auto imm=LoadImmInst::createLoadImm(cons->getType(),cons,b);
                    ins_list.pop_back();
                    ins_list.insert(cur_iter,imm);
                    ret.modify_instr=true;
                    ins->replaceOperand(1,imm);
                }
            }else if(ins->isDiv()||ins->isFDiv()){
                if(dynamic_cast<Constant*>(ins->getOperand(0))){
                    auto imm=LoadImmInst::createLoadImm(ins->getOperand(0)->getType(),ins->getOperand(0),b);
                    ins_list.pop_back();
                    ins_list.insert(cur_iter,imm);
                    ret.modify_instr=true;
                    ins->replaceOperand(0,imm);
                }else if(dynamic_cast<Constant*>(ins->getOperand(1))){
                    auto imm=LoadImmInst::createLoadImm(ins->getOperand(1)->getType(),ins->getOperand(1),b);
                    ins_list.pop_back();
                    ins_list.insert(cur_iter,imm);
                    ret.modify_instr=true;
                    ins->replaceOperand(1,imm);
                }
            }
        }
    }
    return ret;
}