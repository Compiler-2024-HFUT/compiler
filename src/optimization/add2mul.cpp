
/**
 * 加法转乘法 <br>
 * 仅考虑32位整数运算情况 <br>
 * 分析每个寄存器产生时的加（减）表达式，构造更佳的计算顺序 <br>
 * 例如：c=a+a+a+a+3*a-a+b+b优化为6*a+2*b <br>
 */
 #include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Instruction.hpp"
#include "optimization/add2mul.hpp"
Modify Add2Mul::runOnFunc(Function *func) {
    Modify ret;
    for(auto b:func->getBasicBlocks()){
        auto &ins_list=b->getInstructions();
        for(auto iter=ins_list.begin();iter!=ins_list.end();){
            auto ins=*iter;
            auto cur_iter=iter++;
            if(ins->isAdd()){
                auto lhs=ins->getOperand(0),rhs=ins->getOperand(1);
                if(auto ins_lhs=dynamic_cast<Instruction*>(lhs)){
                    if(ins_lhs->isAdd()){
                        if(auto mul=dynamic_cast<Instruction*>(ins_lhs->getOperand(1));mul&&mul->isMul()&&ins_lhs->useOne()&&mul->useOne()){
                            if(mul->getOperand(0)==rhs){
                                if(ConstantInt* cr=dynamic_cast<ConstantInt*>(mul->getOperand(1))){
                                    mul->replaceOperand(1,ConstantInt::get(cr->getValue()+1));
                                    ins->removeUseOfOps();
                                    ins->replaceAllUseWith(ins_lhs);
                                    iter=ins_list.erase(cur_iter);
                                    delete ins;
                                }
                            }
                        }else if(ins_lhs->getOperand(1)==rhs&&ins_lhs->useOne()){
                        auto mul=BinaryInst::create(Instruction::OpID::mul,rhs,ConstantInt::get(2));
                            mul->setParent(ins_lhs->getParent());
                            ins_lhs->getParent()->insertInstr(ins_lhs->getParent()->findInstruction(ins_lhs),mul);
                            ins_lhs->replaceOperand(1,mul);
                            ins->removeUseOfOps();
                            ins->replaceAllUseWith(ins_lhs);
                            iter=ins_list.erase(cur_iter);
                            delete ins;
                        }else if(ins_lhs->getOperand(0)==rhs&&ins_lhs->useOne()){
                        auto mul=BinaryInst::create(Instruction::OpID::mul,rhs,ConstantInt::get(2));
                            mul->setParent(ins_lhs->getParent());
                            ins_lhs->getParent()->insertInstr(ins_lhs->getParent()->findInstruction(ins_lhs),mul);
                            ins_lhs->replaceOperand(0,mul);
                            ins->removeUseOfOps();
                            ins->replaceAllUseWith(ins_lhs);
                            iter=ins_list.erase(cur_iter);
                            delete ins;
                        }
                    }
                }

            }
        }
    }
    return ret;
}