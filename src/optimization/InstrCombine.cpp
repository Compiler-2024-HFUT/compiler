#include "optimization/InstrCombine.hpp"
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include <sys/cdefs.h>
int ispow2(int num){
    if(num<=0)
        return 0;
    int ret=0;
    int curnum=1;
    while(curnum<num){
        curnum*=2;
        ++ret;
    }
    if(curnum!=num)
        return 0;
    return ret;
}
void  InstrCombine::preProcess(Function*cur_func){
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
            }else if(bin->isAdd()||bin->isMul()||bin->isFAdd()||bin->isFMul()){
                if(dynamic_cast<Constant*>(left)&&!dynamic_cast<Constant*>(right)){
                    bin->removeOperands(0,1);
                    bin->addOperand(right);
                    bin->addOperand(left);
                }
            }
        }
    }
}
Instruction* InstrCombine::replaceInstUsesWith(Instruction*old_instr,Value* new_val){
    for(auto [v,i]:old_instr->getUseList()){
        if(auto ins=dynamic_cast<Instruction*>(v))
            work_set_.push_back(ins);
    }
    old_instr->replaceAllUseWith(new_val);
    return old_instr;
}
//mul和add
Instruction* InstrCombine::combineConst(BinaryInst*instr){
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    if(!lhs->useOne())return nullptr;
    if(!dynamic_cast<BinaryInst*>(lhs)) return nullptr;
    auto l_ins=static_cast<BinaryInst*>(lhs);
    if(l_ins->getInstrType()!=instr->getInstrType()) return nullptr;
    auto cr=dynamic_cast<ConstantInt*>(rhs);
    auto clr=dynamic_cast<ConstantInt*>(l_ins->getOperand(1));
    if(cr&&clr){
        instr->removeOperands(0,1);
        auto ret=BinaryInst::create(instr->getInstrType(),l_ins->getOperand(0),ConstantInt::get(cr->getValue()+clr->getValue()));
        ret->setParent(instr->getParent());
        l_ins->getParent()->deleteInstr(l_ins);
        work_set_.remove(l_ins);
        return ret;
    }
    return nullptr;
}
Instruction* InstrCombine::combineMul(Instruction*instr){
    Instruction* ret=nullptr;
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    {
        ConstantInt* cr=dynamic_cast<ConstantInt*>(rhs);
        if(cr){
            if(cr->getValue()==0)
                ret=replaceInstUsesWith(instr,cr);
            else if(cr->getValue()==1){
                ret= replaceInstUsesWith(instr,lhs);
            }else{
                // int log2_cr=log2(cr->getValue());
                // if(pow(2,log2_cr)!=cr->getValue()) return ret;
                // ConstantInt* new_cr=ConstantInt::get(log2_cr);
                // if(new_cr->getValue()>0)
                //     ret=BinaryInst::create(Instruction::OpID::shl,lhs,new_cr);
                // ret->setParent(instr->getParent());
            }
        }
    }
    return ret;
}
Instruction* InstrCombine::combineAdd(Instruction*instr){
    Instruction* ret=nullptr;
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    auto blhs=dynamic_cast<BinaryInst*>(lhs),brhs=dynamic_cast<BinaryInst*>(rhs);
    ret=combineConst(static_cast<BinaryInst*>(instr));
    if(ret)
        return ret;
    if(auto cons=dynamic_cast<ConstantInt*>(rhs)){
        if(cons->getValue()==0)
            ret=replaceInstUsesWith(instr,lhs);
        else if(auto consl=dynamic_cast<ConstantInt*>(lhs)){
            ret=replaceInstUsesWith(instr,ConstantInt::get(consl->getValue()+cons->getValue()));
        }
    }
    else if(blhs){
        if(blhs->isNeg())
            ret=BinaryInst::create(Instruction::OpID::sub,rhs,blhs->getOperand(1));
    }else if(brhs){
        if(brhs->isNeg())
            ret=BinaryInst::create(Instruction::OpID::sub,lhs,brhs->getOperand(1));
    }
    // auto lhs=dynamic_cast<BinaryInst*>(instr->getOperand(0));
    // if(lhs&&lhs->getInstrType()==instr->getInstrType()&&lhs->useOne()){
    //     auto lhs_rhs=lhs->getOperand(1);
    //     // (dynamic_cast<ConstantInt*>(lhs_rhs)&&dynamic_cast<ConstantInt*>(rhs))
    //     {auto clr=dynamic_cast<ConstantInt*>(lhs_rhs),cr=dynamic_cast<ConstantInt*>(rhs);
    //     if((clr&&cr)||lhs->getOperand(1)==instr->getOperand(1)){
    //         BinaryInst*tmp_lhs =lhs;
    //         auto tmp=instr->getOperand(1);
    //         instr->setOperand(1,lhs->getOperand(0));
    //         lhs->setOperand(0,tmp);
    //     }}
    //     auto tmp=instr;
    //     instr=lhs;
    //     lhs=dynamic_cast<BinaryInst*>(tmp->getOperand(0));
    // }
    return ret;
}
Instruction* InstrCombine::combineDiv(Instruction*instr){
    Instruction* ret=nullptr;
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    auto cl=dynamic_cast<ConstantInt*>(lhs),cr=dynamic_cast<ConstantInt*>(rhs);
    if(cr!=nullptr){
        if(cr->getValue()==1){
            ret=replaceInstUsesWith(instr,cl);
        }else if(cl){
            ret=replaceInstUsesWith(instr,ConstantInt::get(cl->getValue()/cr->getValue()));
        }else{
            // int log2_cr=log2(cr->getValue());
            // if(pow(2,log2_cr)!=cr->getValue()) return ret;
            // ConstantInt* new_cr=ConstantInt::get(log2_cr);
            // if(new_cr->getValue()>0)
            //     ret=BinaryInst::create(Instruction::OpID::asr,lhs,new_cr);
            // ret->setParent(instr->getParent());
        }
    }if(cl){
        if(cl->getValue()==0)
            ret=replaceInstUsesWith(instr,cl);
    }
    return ret;
}
Instruction* InstrCombine::combine(Instruction*instr){
    auto pair=combine_map_.find(instr->getInstrType());
    if(pair==combine_map_.end())return nullptr;
    return(pair->second)(instr);

}
Modify InstrCombine::runOnFunc(Function*func){

    if(func->getBasicBlocks().empty())return {};
    cur_func_=func;
    work_set_.clear();
    preProcess(func);
    for(auto b:func->getBasicBlocks()){
        work_set_.insert(work_set_.end(),b->getInstructions().begin(),b->getInstructions().end());
    }

    while (!work_set_.empty()) {
        auto instr=work_set_.back();
        work_set_.pop_back();
        auto new_instr=combine(instr);
        //什么都没做
        if(new_instr==nullptr) continue;
        //插入了新的指令替换此指令
        if(new_instr!=instr){
            auto bb=instr->getParent();
            auto iter=bb->insertInstr(instr->getParent()->findInstruction(instr),new_instr);
            instr->replaceAllUseWith(new_instr);
            new_instr->setParent(instr->getParent());
            bb->eraseInstr(++iter);
            work_set_.remove(instr);
            delete instr;
            work_set_.push_back(new_instr);
            for(auto [v ,i]:new_instr->getUseList()){
                if(dynamic_cast<Instruction*>(v))
                    work_set_.push_back((Instruction*)v);
            }
        }else if(instr->useEmpty()&&!instr->isCall()&&!instr->isWriteMem()){
            work_set_.remove(instr);
            instr->getParent()->eraseInstr(instr->getParent()->findInstruction(instr));
            delete instr;
        }
    }

    work_set_.clear();
    return{};
}

InstrCombine::InstrCombine(Module *m,InfoManager*im):FunctionPass(m,im),combine_map_{
    {Instruction::OpID::add,[this](Instruction* instr)->Instruction* { return combineAdd(instr); }},
    {Instruction::OpID::mul,[this](Instruction* instr)->Instruction* { return combineMul(instr); }},
    {Instruction::OpID::sdiv,[this](Instruction* instr)->Instruction* { return combineDiv(instr); }},
}{

}