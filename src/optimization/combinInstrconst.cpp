#include "optimization/combinInstrconst.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Instruction.hpp"
#include <algorithm>
bool CombinInstrConst::combinConstMulAdd(BinaryInst*instr){
    auto type=instr->getInstrType();
    bool modify=false;
    if(type!=Instruction::OpID::add&&type!=Instruction::OpID::fadd&&
        type!=Instruction::OpID::mul&&type!=Instruction::OpID::fmul)
            return modify;
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    auto cr=dynamic_cast<ConstantInt*>(rhs);
    auto cl=dynamic_cast<ConstantInt*>(lhs);
    auto fcr=dynamic_cast<ConstantFP*>(rhs);
    auto fcl=dynamic_cast<ConstantFP*>(lhs);
    if(cl&&cr){
        if(type==Instruction::OpID::add)
            instr->replaceAllUseWith(ConstantInt::get(cl->getValue()+cr->getValue()));
        else
            instr->replaceAllUseWith(ConstantInt::get(cl->getValue()*cr->getValue()));
        modify=true;
    }else if(fcl&&fcr){
        if(type==Instruction::OpID::fadd)
            instr->replaceAllUseWith(ConstantFP::get(fcl->getValue()+fcr->getValue()));
        else
            instr->replaceAllUseWith(ConstantFP::get(fcl->getValue()*fcr->getValue()));
        modify=true;
    }
    return modify;
}
bool CombinInstrConst::combinMullAdd(BinaryInst*instr){
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    if(!lhs->useOne()||!dynamic_cast<BinaryInst*>(lhs))return 0;
    auto l_ins=(BinaryInst*)(lhs);
    if(l_ins->getInstrType()!=instr->getInstrType()) return 0;
    auto type=instr->getInstrType();
    if(type!=Instruction::OpID::add&&type!=Instruction::OpID::fadd&&
        type!=Instruction::OpID::mul&&type!=Instruction::OpID::fmul)
            return 0;

    auto  modify=false;

    auto cr=dynamic_cast<ConstantInt*>(rhs);
    auto clr=dynamic_cast<ConstantInt*>(l_ins->getOperand(1));
    auto fcr=dynamic_cast<ConstantFP*>(rhs);
    auto fclr=dynamic_cast<ConstantFP*>(l_ins->getOperand(1));
    BasicBlock*insBB=instr->getParent();
    if(cr&&clr){
        instr->removeAllOperand();
        instr->addOperand(l_ins->getOperand(0));
        instr->addOperand(ConstantInt::get(cr->getValue()+clr->getValue()));
        modify=true;
    }else if(fcr&&fclr){
        instr->removeAllOperand();
        instr->addOperand(l_ins->getOperand(0));
        instr->addOperand(ConstantFP::get(fcr->getValue()+fclr->getValue()));
        modify=true;
    }
    if(modify){
        auto const iter=std::find(work_set_.begin(),work_set_.end(),l_ins);
        if(iter!=work_set_.end()){
            work_set_.erase(iter);
            l_ins->getParent()->deleteInstr(l_ins);
            delete  l_ins;
        }
        for(auto [v,i]:instr->getUseList()){
            work_set_.push_back(v);
        }
    }
    return modify;
}
void  CombinInstrConst::preProcess(Function*cur_func){
    for(auto b:cur_func->getBasicBlocks()){
        auto & instrs=b->getInstructions();
        for(auto instr_iter=instrs.begin();instr_iter!=instrs.end();){
            auto cur_iter=instr_iter++;
            auto instr=*cur_iter;
            auto bin=dynamic_cast<BinaryInst*>(instr);
            if(bin==nullptr)continue;
            auto left=bin->getOperand(0);
            auto right=bin->getOperand(1);
            if(bin->isSub()){
                if(auto cr=dynamic_cast<ConstantInt*>(right)){
                    auto new_add=BinaryInst::create(Instruction::OpID::add,left,ConstantInt::get(-cr->getValue()));
                    new_add->setParent(b);
                    instr_iter=++b->insertInstr(cur_iter,new_add);
                    instr->replaceAllUseWith(new_add);
                    b->eraseInstr(instr_iter++);
                }
            }else if(bin->isFSub()){
                if(auto cr=dynamic_cast<ConstantFP*>(right)){
                    auto new_add=BinaryInst::create(Instruction::OpID::fadd,left,ConstantFP::get(-cr->getValue()));
                    new_add->setParent(b);
                    instr->replaceAllUseWith(new_add);
                    instr_iter=++b->insertInstr(cur_iter,new_add);
                    b->eraseInstr(instr_iter++);
                }
            }else if(bin->isAdd()||bin->isMul()){//||bin->isFAdd()||bin->isFMul()){
                if(dynamic_cast<Constant*>(left)&&!dynamic_cast<Constant*>(right)){
                    bin->removeAllOperand();
                    bin->addOperand(right);
                    bin->addOperand(left);
                }
            }
        }
    }
}
Modify CombinInstrConst::runOnFunc(Function*func){
    preProcess(func);
    for(auto b:func->getBasicBlocks()){
        for(auto instr:b->getInstructions()){
            if(instr->isBinary())
                work_set_.push_back(instr);
        }
    }
    Modify ret{};
    while(!work_set_.empty()){
        auto val=work_set_.back();
        work_set_.pop_back();
        auto bin=dynamic_cast<BinaryInst*>(val);
        if(bin==nullptr) continue;
        if(combinConstMulAdd(bin)){
            bin->getParent()->deleteInstr(bin);
            delete bin;
            continue;
        }
        ret.modify_instr=combinMullAdd(bin);
        if(ret.modify_instr){
            work_set_.push_back(bin);
        }
    }
    return ret;
}