#include "optimization/InstrCombine.hpp"
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include <algorithm>
#include <cassert>
int pow2(int num){
    return num!=0?2<<num:1;
}
//要考虑负吗
int ispow2(int num){
    bool isneg=false;
    if(num<=0){
        isneg=true;
        num=-num;
    }
        
    int ret=0;
    int curnum=1;
    while(curnum<num){
        curnum=curnum<<1;
        ++ret;
    }
    if(curnum!=num)
        return 0;
    return isneg?-ret:ret;
}
// 这个指令是不是 0-vlue;
BinaryInst*__is__neg(Value*v){
    if(auto bin=dynamic_cast<BinaryInst*>(v)){
        if(bin->isSub()){
            if(auto lc=dynamic_cast<ConstantInt*>((bin->getOperand(0)))){    
                return lc->getValue()==0?bin:nullptr;
            }
        }else if(bin->isFSub()){
            if(auto lc=dynamic_cast<ConstantFP*>((bin->getOperand(0)))){    
                return lc->getValue()==.0f?bin:nullptr;
            }
        }
    }
    return nullptr;
}
__attribute__((always_inline)) bool canSwitchOper(Instruction::OpID op){
        if(op==Instruction::OpID::add||op==Instruction::OpID::mul||
            op==Instruction::OpID::fadd||op==Instruction::OpID::fmul||
            op==Instruction::OpID::lxor||op==Instruction::OpID::lor||op==Instruction::OpID::land)
                return true;
    return false;
}
//add (add v1 c1) c2 ===> add v1(c1+c2)
Instruction* _simplify_bin(BinaryInst*bin_ins){
    auto op=bin_ins->getInstrType();
    if(canSwitchOper(op)==false)
        return 0;
    auto lhs=bin_ins->getOperand(0),rhs=bin_ins->getOperand(1);
    auto const_rhs=dynamic_cast<Constant*>(rhs);
    if(auto bin_lhs=dynamic_cast<BinaryInst*>(lhs)){
        if(bin_lhs->getInstrType()==op&&dynamic_cast<Constant*>(bin_lhs->getOperand(1))&&const_rhs){
            Constant* const_fold=Constant::get((Constant*)(bin_lhs->getOperand(1)),op,const_rhs);
            assert(const_fold!=0);
            bin_ins->removeAllOperand();
            bin_ins->addOperand(bin_lhs->getOperand(0));
            bin_ins->addOperand(const_fold);
            //有副作用
            // if(bin_lhs->useEmpty()){
            //     removeworkset(bin_lhs);
            //     bin_lhs->getParent()->deleteInstr(bin_lhs);
            //     delete bin_lhs;
            // }
            return bin_ins;
        }
    }
    return 0;
}
//尽量将const放到右边
//add c1 v1 ===> add v1 c1
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
            }else if(canSwitchOper(bin->getInstrType())){
                if(dynamic_cast<Constant*>(left)!=0&&dynamic_cast<Constant*>(right)==0){
                    bin->removeOperands(0,1);
                    bin->addOperand(right);
                    bin->addOperand(left);
                }
            }
        }
    }
}
__attribute__((always_inline))Instruction* replaceInstUsesWith(Instruction*old_instr,Value* new_val){
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
        // removeworkset(l_ins);
        return ret;
    }
    return nullptr;
}
Instruction* InstrCombine::combineMul(Instruction*instr){
    Instruction* ret=_simplify_bin((BinaryInst*)instr);
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    ConstantInt* cr=dynamic_cast<ConstantInt*>(rhs);
    auto bin_lhs=dynamic_cast<BinaryInst*>(lhs);
    if(cr){
        int cr_val=cr->getValue();
        if(cr_val==0)
            ret=replaceInstUsesWith(instr,cr);
        else if(bin_lhs){
            if(bin_lhs->isLsl()&&dynamic_cast<ConstantInt*>(bin_lhs->getOperand(1))){
                auto lhs_op1=(ConstantInt*)(bin_lhs);
                return BinaryInst::create(Instruction::OpID::mul,bin_lhs,
                    ConstantInt::getFromBin(cr,Instruction::OpID::shl,lhs_op1));
            }
        }   else if(cr_val==1){
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
    return ret;
}
Instruction* InstrCombine::combineSub(Instruction*instr){
    Instruction* ret=nullptr;
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    auto l_const=dynamic_cast<ConstantInt*>(lhs),r_const=dynamic_cast<ConstantInt*>(rhs);
    
    if(lhs==rhs)
        return replaceInstUsesWith(instr,ConstantInt::get(0));
    if(__is__neg(rhs)){
        return BinaryInst::create(Instruction::OpID::add,lhs,rhs);

    }
    return ret;
}
Instruction* InstrCombine::combineAdd(Instruction*instr){
    Instruction* ret=_simplify_bin((BinaryInst*)instr);
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    auto blhs=dynamic_cast<BinaryInst*>(lhs),brhs=dynamic_cast<BinaryInst*>(rhs);
    // ret=combineConst(static_cast<BinaryInst*>(instr));
    // if(ret)
    //     return ret;
    if(auto cons_r=dynamic_cast<ConstantInt*>(rhs)){
        if(cons_r->getValue()==0)
            ret=replaceInstUsesWith(instr,lhs);
        else if(auto consl=dynamic_cast<ConstantInt*>(lhs)){
            ret=replaceInstUsesWith(instr,ConstantInt::get(consl->getValue()+cons_r->getValue()));
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
Instruction* InstrCombine::combineFAdd(Instruction*instr){
    Instruction*ret=_simplify_bin((BinaryInst*)instr);
    return ret;
}
Instruction* InstrCombine::combineFMul(Instruction*instr){
    Instruction*ret=_simplify_bin((BinaryInst*)instr);
    return ret;
}

Instruction* InstrCombine::combineOr(Instruction*instr){
    Instruction*ret=_simplify_bin((BinaryInst*)instr);
    return ret;
}
Instruction* InstrCombine::combineXor(Instruction*instr){
    Instruction*ret=_simplify_bin((BinaryInst*)instr);
    return ret;
}
Instruction* InstrCombine::combineAnd(Instruction*instr){
    Instruction*ret=_simplify_bin((BinaryInst*)instr);
    return ret;
}
Instruction* InstrCombine::combineShl(Instruction*instr){
    return 0;
}
Instruction* InstrCombine::combineAsr(Instruction*instr){
    return 0;
}

/*
1.如果什么都没做返回空
2.插入了新的指令替换 返回新指令
3.修改本指令,判断指令是否为空
4.替换本指令为其他值(可以不是本指令)
*/
Instruction* InstrCombine::combine(Instruction*instr){
    auto pair=combine_map_.find(instr->getInstrType());
    if(pair==combine_map_.end())return nullptr;
    return(pair->second)(instr);

}

void InstrCombine::removeInsWithWorkset(Instruction*ins){
  work_set_.erase(std::remove(work_set_.begin(), work_set_.end(), ins),work_set_.end());
    ins->getParent()->eraseInstr(ins->getParent()->findInstruction(ins));
    delete ins;
}

Modify InstrCombine::runOnFunc(Function*func){

    if(func->getBasicBlocks().empty())return {};
    cur_func_=func;
    work_set_.clear();
    preProcess(func);
    for(auto b:func->getBasicBlocks()){
        work_set_.insert(work_set_.end(),b->getInstructions().begin(),b->getInstructions().end());
        // work_set_.assign(b->getInstructions().begin(),b->getInstructions().end());
        //不管br和ret了
        // work_set_.pop_back();
    }

    while (!work_set_.empty()) {
        auto old_instr=work_set_.back();
        work_set_.pop_back();
        if(old_instr->useEmpty()){
            if(dynamic_cast<BinaryInst*>(old_instr)||dynamic_cast<CmpInst*>(old_instr))
                removeInsWithWorkset(old_instr);
            continue;
        }
        auto new_instr=combine(old_instr);
        //什么都没做
        if(new_instr==nullptr) continue;
        //插入了新的指令替换此指令
        if(new_instr!=old_instr){
            auto bb=old_instr->getParent();
            auto iter=bb->insertInstr(old_instr->getParent()->findInstruction(old_instr),new_instr);
            old_instr->replaceAllUseWith(new_instr);
            new_instr->setParent(old_instr->getParent());
            // to_erase_.insert({old_instr,++iter});
            removeInsWithWorkset(old_instr);
            work_set_.push_back(new_instr);
            for(auto [v ,i]:new_instr->getUseList()){
                if(dynamic_cast<BinaryInst*>(v))
                    work_set_.push_back((BinaryInst*)v);
            }
            //返回旧指令
        }else {
            for(auto [v,i]:old_instr->getUseList())
                if(auto ins=dynamic_cast<Instruction*>(v))
                    work_set_.push_back(ins);
            if(old_instr->useEmpty()&&!old_instr->isCall()&&!old_instr->isWriteMem())
                removeInsWithWorkset(old_instr);
            // to_erase_.insert({old_instr,old_instr->getParent()->findInstruction(old_instr)});
        }
    }
    // for(auto [ins,iter]:to_erase_){
    //     ins->getParent()->eraseInstr(iter);
    //     delete ins;
    // }
    // to_erase_.clear();
    work_set_.clear();
    return{};
}

InstrCombine::InstrCombine(Module *m,InfoManager*im):FunctionPass(m,im),combine_map_{
    {Instruction::OpID::lor,[this](Instruction* instr)->Instruction* { return combineAdd(instr); }},
    {Instruction::OpID::sub,[this](Instruction* instr)->Instruction* { return combineSub(instr); }},
    {Instruction::OpID::add,[this](Instruction* instr)->Instruction* { return combineAdd(instr); }},
    {Instruction::OpID::mul,[this](Instruction* instr)->Instruction* { return combineMul(instr); }},
    {Instruction::OpID::sdiv,[this](Instruction* instr)->Instruction* { return combineDiv(instr); }},
    {Instruction::OpID::lor,[this](Instruction* instr)->Instruction* { return combineDiv(instr); }},
    {Instruction::OpID::land,[this](Instruction* instr)->Instruction* { return combineMul(instr); }},
    {Instruction::OpID::shl,[this](Instruction* instr)->Instruction* { return combineShl(instr); }},
    {Instruction::OpID::asr,[this](Instruction* instr)->Instruction* { return combineAsr(instr); }},
    // {Instruction::OpID::lsr,[this](Instruction* instr)->Instruction* { return combineMul(instr); }},
    // {Instruction::OpID::fdiv,[this](Instruction* instr)->Instruction* { return combineDiv(instr); }},

}{

}