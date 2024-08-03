#include "optimization/InstrCombine.hpp"
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "optimization/PassManager.hpp"
#include <algorithm>
#include <cassert>
#include <list>
std::list<Instruction*> InstrResolve::resolveAdd(Instruction*instr){
    auto lhs=dynamic_cast<Instruction*>(instr->getOperand(0));
    if(lhs==0)
        return {};
    if(!lhs->isMul()){
        if(lhs->isAdd()){
            auto rc=dynamic_cast<ConstantInt*>(instr->getOperand(1));
            auto lhs_lhs=dynamic_cast<Instruction*>(lhs->getOperand(0));
            auto lhs_rhs=dynamic_cast<Instruction*>(lhs->getOperand(1));
            //((a+c1)+b)+c2==>a+b+(c1+c2)==>a+b+c3
            if(lhs_lhs==0||lhs_rhs==0||rc)
                return{};
            if(lhs_lhs->isAdd()&&dynamic_cast<ConstantInt*>(lhs_lhs->getOperand(1))){
                if(auto llr=dynamic_cast<ConstantInt*>(lhs_lhs->getOperand(1))){
                    auto ret1=BinaryInst::create(Instruction::OpID::add,lhs_lhs->getOperand(0),lhs->getOperand(1));
                    auto ret2=BinaryInst::create(Instruction::OpID::add,ret1,ConstantInt::get(llr->getValue()+rc->getValue()));
                    ret1->setParent(instr->getParent());
                    ret2->setParent(instr->getParent());
                    return {ret1,ret2};
                }
            }else if(lhs_rhs->isAdd()&&dynamic_cast<ConstantInt*>(lhs_rhs->getOperand(1))){
                if(auto lrr=dynamic_cast<ConstantInt*>(lhs_rhs->getOperand(1))){
                    auto ret1=BinaryInst::create(Instruction::OpID::add,lhs->getOperand(0),lhs_rhs->getOperand(0));
                    auto ret2=BinaryInst::create(Instruction::OpID::add,ret1,ConstantInt::get(lrr->getValue()+rc->getValue()));
                    ret1->setParent(instr->getParent());
                    ret2->setParent(instr->getParent());
                    return {ret1,ret2};
                }
            }
            //(a+(b+c1))+c2
        }
        return {};
    }
    auto rhs=dynamic_cast<Instruction*>(instr->getOperand(1));
    if(rhs==0)
        return{};
    auto lhs_lhs=dynamic_cast<Instruction*>(lhs->getOperand(0));
    auto lhs_rhs=dynamic_cast<ConstantInt*>(lhs->getOperand(1));
    if(lhs_lhs==0||lhs_rhs==0)
        return {};
    if(!lhs_lhs->isAdd())
        return{};
    if(auto lhs_lhs_lhs=dynamic_cast<Instruction*>(lhs_lhs->getOperand(0))){
        auto lhs_lhs_rhs=dynamic_cast<ConstantInt*>(lhs_lhs->getOperand(1));
        if(lhs_lhs_rhs){
            auto ins1=BinaryInst::create(Instruction::OpID::mul,lhs_lhs_lhs,lhs_rhs);
            auto ins2=BinaryInst::create(Instruction::OpID::add,ins1,rhs);
            auto ins3=BinaryInst::create(Instruction::OpID::add,ins2,ConstantInt::get(lhs_rhs->getValue()*lhs_lhs_rhs->getValue()));
            ins1->setParent(instr->getParent());
            ins2->setParent(instr->getParent());
            ins3->setParent(instr->getParent());
            return {ins1,ins2,ins3};
        }
    }
    return{};
}
std::list<Instruction*> InstrResolve::resolveRAdd(Instruction*instr){
    auto lhs=dynamic_cast<Instruction*>(instr->getOperand(0)),rhs=dynamic_cast<Instruction*>(instr->getOperand(1));
    if(lhs==0||rhs==0)
        return {};
    if(!rhs->isMul())
        return {};
    auto rhs_lhs=dynamic_cast<Instruction*>(rhs->getOperand(0));
    auto rhs_rhs=dynamic_cast<ConstantInt*>(rhs->getOperand(1));
    if(rhs_lhs==0||rhs_rhs==0)
        return {};
    if(!rhs_lhs->isAdd())
        return{};

    if(auto rhs_lhs_lhs=dynamic_cast<Instruction*>(rhs_lhs->getOperand(0))){
        auto rhs_lhs_rhs=dynamic_cast<ConstantInt*>(rhs_lhs->getOperand(1));
        if(rhs_lhs_rhs){
            auto ins1=BinaryInst::create(Instruction::OpID::mul,rhs_lhs_lhs,rhs_rhs);
            auto ins2=BinaryInst::create(Instruction::OpID::add,lhs,ins1);
            auto ins3=BinaryInst::create(Instruction::OpID::add,ins2,ConstantInt::get(rhs_rhs->getValue()*rhs_lhs_rhs->getValue()));
            ins1->setParent(instr->getParent());
            ins2->setParent(instr->getParent());
            ins3->setParent(instr->getParent());
            return {ins1,ins2,ins3};
        }
    }    
    return{};
}
// Instruction* resolveMul(Instruction*instr){

// }

Modify InstrResolve::runOnFunc(Function*func){
    Modify ret{};
    for(auto b:func->getBasicBlocks()){
        for(auto iter=b->getInstructions().begin();iter!=b->getInstructions().end();){
            auto ins=*iter;
            auto cur_iter=iter;
            iter++;
            std::list<Instruction*> new_ins;
            if(ins->isAdd())
                new_ins=resolveAdd(ins);
            else continue;
            // else if(i->isMul()){
            //     ret=resolveMul(i);
            // }
            if(new_ins.empty())
                new_ins=resolveRAdd(ins);
            if(new_ins.empty())
                continue;
            ret.modify_instr=true;
            ins->replaceAllUseWith(new_ins.back());
            b->getInstructions().insert(cur_iter,new_ins.begin(),new_ins.end());
        }
    }
    return ret;
}

int pow2(int num){
    return num!=0?2<<num:1;
}
//要考虑负吗
int islog2(int num){
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
Instruction* InstrCombine::replaceInstUsesWith(Instruction*old_instr,Value* new_val){
    for(auto [v,i]:old_instr->getUseList())
        if(auto ins=dynamic_cast<Instruction*>(v))
            work_set_.push_back(ins);
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
            return replaceInstUsesWith(instr,cr);
        if(cr_val==1){
            return replaceInstUsesWith(instr,lhs);
        }
        if(bin_lhs){
            if(bin_lhs->isLsl()&&dynamic_cast<ConstantInt*>(bin_lhs->getOperand(1))){
                auto lhs_op1=(ConstantInt*)(bin_lhs);
                return BinaryInst::create(Instruction::OpID::mul,bin_lhs,
                    ConstantInt::getFromBin(cr,Instruction::OpID::shl,lhs_op1));
            }
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
    //a-a ==>0
    if(lhs==rhs)
        return replaceInstUsesWith(instr,ConstantInt::get(0));
    // a- (0-b)==>a+b
    if(__is__neg(rhs)){
        return BinaryInst::create(Instruction::OpID::add, lhs, ((Instruction*)rhs)->getOperand(1));
    }
    auto bin_rhs=dynamic_cast<Instruction*>(rhs);
    if(bin_rhs){
        if(bin_rhs->isSub()){
            //c1-(c2-a)==>(c1-c2)+a ==>c3+a==>a+c3
            if(auto rhs_lhs_ci=dynamic_cast<ConstantInt*>(bin_rhs->getOperand(0));rhs_lhs_ci&&l_const){
                auto new_lhs=ConstantInt::getFromBin(l_const,Instruction::OpID::sub,rhs_lhs_ci);
                return BinaryInst::create(Instruction::OpID::add,bin_rhs->getOperand(1),new_lhs);
            }
        }else if(bin_rhs->isAdd()){
            //c1-(c2+a)==>(c1-c2)-a ==>c3-a
            if(auto rhs_lhs_ci=dynamic_cast<ConstantInt*>(bin_rhs->getOperand(0));rhs_lhs_ci&&l_const){
                instr->replaceOperand(0,ConstantInt::getFromBin(l_const,Instruction::OpID::sub,rhs_lhs_ci));
                instr->replaceOperand(1,bin_rhs->getOperand(1));
                return instr;
            }
        }
        //a-(a+b)==>-b
        if(bin_rhs->isAdd()){
            if(bin_rhs->getOperand(0)==lhs){
                return BinaryInst::create(Instruction::OpID::sub, ConstantInt::get(0), bin_rhs->getOperand(1));
            }else if(bin_rhs->getOperand(1)==lhs){
                return BinaryInst::create(Instruction::OpID::sub, ConstantInt::get(0), bin_rhs->getOperand(0));
            }
        }
    }
    //(a + b) - b => a
    auto bin_lhs=dynamic_cast<Instruction*>(lhs);
    if(bin_lhs){
        if(bin_lhs->isAdd()){
            if(bin_lhs->getOperand(0)==rhs){
                return replaceInstUsesWith(instr,bin_lhs->getOperand(1));
            }else if(bin_lhs->getOperand(1)==rhs){
                return replaceInstUsesWith(instr,bin_lhs->getOperand(0));
            }
        }
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
            return replaceInstUsesWith(instr,lhs);
        else if(auto consl=dynamic_cast<ConstantInt*>(lhs)){
            return replaceInstUsesWith(instr,ConstantInt::get(consl->getValue()+cons_r->getValue()));
        }
        //(a - c1) + c2 => a + (c2 - c1)
        if(blhs){
            if(blhs->isAdd()){
                if(auto lhs_rhs_ci=dynamic_cast<ConstantInt*>(blhs->getOperand(1))){
                    instr->replaceOperand(0,blhs->getOperand(0));
                    instr->replaceOperand(1,ConstantInt::getFromBin(cons_r,Instruction::OpID::add,lhs_rhs_ci));
                    ret=instr;
                }
            //(-a)+b ==> b-a
            //a+(-b) ==> a-b
            }else if(blhs->isNeg()){
            return BinaryInst::create(Instruction::OpID::sub,rhs,blhs->getOperand(1));
            //(a*c1)+a==>a*(c1+1)
            }else if(blhs->isMul()&&blhs->getOperand(0)==rhs){
                if(auto lhs_rhs_ci=dynamic_cast<ConstantInt*>(blhs->getOperand(1))){
                    blhs->replaceOperand(1,ConstantInt::get(lhs_rhs_ci->getValue()+1));
                    return replaceInstUsesWith(instr,blhs);
                }
            }
        }
        return ret;
    }
    if(brhs){
        if(brhs->isNeg())
            return BinaryInst::create(Instruction::OpID::sub,lhs,brhs->getOperand(1));
        //a+(a*c1)==>a*(c1+1)
        else if(brhs->isMul()&&brhs->getOperand(0)==lhs){
            if(auto rhs_lhs_ci=dynamic_cast<ConstantInt*>(brhs->getOperand(1))){
                brhs->replaceOperand(1,ConstantInt::get(rhs_lhs_ci->getValue()+1));
                return replaceInstUsesWith(instr,brhs);
            }
        }
    }
    
    //todo
    //(a + c1) + c2 => a + (c1 + c2)
    //c1 + (a + c2) => a + (c1 + c2)
    //c1 + (a - c2) => a + (c1 - c2)

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
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    Instruction* ret=nullptr;
    if(lhs==rhs)
        return replaceInstUsesWith(instr,ConstantInt::get(1));

    auto cl=dynamic_cast<ConstantInt*>(lhs),cr=dynamic_cast<ConstantInt*>(rhs);
    if(cl){
        if(cl->getValue()==0)
            return replaceInstUsesWith(instr,cl);
    }
    if(cl&&cr){
        return replaceInstUsesWith(instr,ConstantInt::get(cl->getValue()/cr->getValue()));
    }
    if(cr!=nullptr){
        if(cr->getValue()==1){
            ret=replaceInstUsesWith(instr,lhs);
        }else{
            auto lhs_bin=dynamic_cast<BinaryInst*>(lhs);
            if(lhs_bin==0)return ret;
            //a*c1/c2 ==>a* (c1/c2)
            if(lhs_bin->isMul()){
                if(auto lhs_rhs_const=dynamic_cast<ConstantInt*>(lhs_bin->getOperand(1))){
                    if(lhs_rhs_const->getValue()%cr->getValue()==0){
                        ret=BinaryInst::create(Instruction::OpID::mul,lhs_bin->getOperand(0),
                            ConstantInt::get(lhs_rhs_const->getValue()/cr->getValue()));
                    }
                }
            //a/c1/c2 ==>a/(c1*c2)
            }else if(lhs_bin->isDiv()){
                if(auto lhs_rhs_const=dynamic_cast<ConstantInt*>(lhs_bin->getOperand(1))){
                    auto new_cr=ConstantInt::getFromBin(lhs_rhs_const,Instruction::OpID::mul,cr);
                    assert(new_cr!=0);
                    instr->removeAllOperand();
                    instr->addOperand(lhs_bin->getOperand(0));
                    instr->addOperand(new_cr);
                    ret=instr;
                }
            }else if(lhs_bin->isAsr()){

            }else if(lhs_bin->isLsl()){

            }
            // int log2_cr=log2(cr->getValue());
            // if(pow(2,log2_cr)!=cr->getValue()) return ret;
            // ConstantInt* new_cr=ConstantInt::get(log2_cr);
            // if(new_cr->getValue()>0)
            //     ret=BinaryInst::create(Instruction::OpID::asr,lhs,new_cr);
            // ret->setParent(instr->getParent());
        }
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
Instruction* InstrCombine::combineCmp(Instruction*instr){
    if(auto icmp=dynamic_cast<CmpInst*>(instr)){
        return combineICmp(icmp);
    }else{
        return combineFCmp(((FCmpInst*)instr));
    }
}
Instruction* InstrCombine::combineICmp(CmpInst* instr){
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);

    if(ConstantInt* ci_rhs=dynamic_cast<ConstantInt*>(rhs)){
        if(auto bin_lhs=dynamic_cast<BinaryInst*>(lhs)){
            //(icmp slt (sub nsw A B), 0) -> (icmp slt A, B)
            if(bin_lhs->getUseList().size()!=1)
                return 0;
            if(bin_lhs->isSub()){
                if(ci_rhs->getValue()==0){
                    instr->replaceOperand(0,bin_lhs->getOperand(0));
                    instr->replaceOperand(1,bin_lhs->getOperand(1));
                    return instr;
                // }else if(){

                // }else{

                }
            }
        }
    }
    return 0;
}
Instruction* InstrCombine::combineFCmp(FCmpInst* instr){
    // auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
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
            if(dynamic_cast<BinaryInst*>(old_instr)||old_instr->isCmp()||old_instr->isFCmp())
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
            work_set_.push_back(old_instr);
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
    {Instruction::OpID::lor,[this](Instruction* instr)->Instruction* { return combineOr(instr); }},
    {Instruction::OpID::sub,[this](Instruction* instr)->Instruction* { return combineSub(instr); }},
    {Instruction::OpID::add,[this](Instruction* instr)->Instruction* { return combineAdd(instr); }},
    {Instruction::OpID::mul,[this](Instruction* instr)->Instruction* { return combineMul(instr); }},
    {Instruction::OpID::sdiv,[this](Instruction* instr)->Instruction* { return combineDiv(instr); }},
    {Instruction::OpID::lxor,[this](Instruction* instr)->Instruction* { return combineXor(instr); }},
    {Instruction::OpID::land,[this](Instruction* instr)->Instruction* { return combineAnd(instr); }},
    {Instruction::OpID::shl,[this](Instruction* instr)->Instruction* { return combineShl(instr); }},
    {Instruction::OpID::asr,[this](Instruction* instr)->Instruction* { return combineAsr(instr); }},
    // {Instruction::OpID::fsub,[this](Instruction* instr)->Instruction* { return combineFSub(instr); }},
    {Instruction::OpID::fadd,[this](Instruction* instr)->Instruction* { return combineFAdd(instr); }},
    {Instruction::OpID::fmul,[this](Instruction* instr)->Instruction* { return combineFMul(instr); }},

    // {Instruction::OpID::lsr,[this](Instruction* instr)->Instruction* { return combineMul(instr); }},
    // {Instruction::OpID::fdiv,[this](Instruction* instr)->Instruction* { return combineDiv(instr); }},

}{

}


Instruction* InstrReduc::reduc(Instruction*instr){
    auto pair=reduc_map_.find(instr->getInstrType());
    if(pair==reduc_map_.end())return 0;
    return(pair->second)(instr);

}


Modify InstrReduc::runOnFunc(Function*func){
    for(auto b:func->getBasicBlocks()){
        auto &inslist=b->getInstructions();
        work_set_.insert(work_set_.end(),inslist.begin(),inslist.end());
        work_set_.pop_back();
    }
    Modify ret;
    int changed=true;
    do{
        changed=false;
        for(int i=0;i<work_set_.size();++i){
            auto old_ins=work_set_[i];
            if(auto newins=reduc(old_ins)){
                old_ins->getParent()->replaceInsWith(old_ins,newins);
                work_set_[i]=newins;
                changed=true;
                ret.modify_instr=true;
            }
        }

    }while(changed);
    return ret;
}
Instruction*InstrReduc::reducMul(Instruction*instr){
    auto rhs=instr->getOperand(1);
    if(auto rhs_ci=dynamic_cast<ConstantInt*>(rhs)){
        if(auto log=islog2(rhs_ci->getValue());log>0){
            return BinaryInst::create(Instruction::OpID::shl,instr->getOperand(0),ConstantInt::get(log));
        }
    }
    return 0;
}
Instruction*InstrReduc::reducAdd(Instruction*instr){
    auto lhs=instr->getOperand(0);
    if(lhs==instr->getOperand(1)){
        return BinaryInst::create(Instruction::OpID::shl,instr->getOperand(0),ConstantInt::get(1));
    }
    return 0;
}
Instruction*InstrReduc::reducSub(Instruction*instr){
    return 0;
}
Instruction*InstrReduc::reducDiv(Instruction*instr){
    auto rhs=instr->getOperand(1);
    if(auto rhs_ci=dynamic_cast<ConstantInt*>(rhs)){
        if(auto log=islog2(rhs_ci->getValue());log>0){
            return BinaryInst::create(Instruction::OpID::asr,instr->getOperand(0),ConstantInt::get(log));
        }
    }
    return 0;
}
Instruction*InstrReduc::reducOr(Instruction*instr){
    return 0;
}
Instruction*InstrReduc::reducXor(Instruction*instr){
    return 0;
}
Instruction*InstrReduc::reducAnd(Instruction*instr){
    return 0;
}
Instruction*InstrReduc::reducAsr(Instruction*instr){
    return 0;
}
Instruction*InstrReduc::reducShl(Instruction*instr){
    return 0;
}
Instruction*InstrReduc::reducFAdd(Instruction*instr){
    return 0;
}
Instruction*InstrReduc::reducFMul(Instruction*instr){
    return 0;
}
Instruction*InstrReduc::reducRem(Instruction*instr){
    // auto rhs=instr->getOperand(1);
    // if(auto rhs_ci=dynamic_cast<ConstantInt*>(rhs)){
    //     if(auto log=islog2(rhs_ci->getValue());log>0){
    //         return BinaryInst::create(Instruction::OpID::land,instr->getOperand(0),ConstantInt::get(rhs_ci->getValue()-1));
    //     }
    // }
    return 0;
}
InstrReduc::InstrReduc(Module *m,InfoManager*im):FunctionPass(m,im),reduc_map_{
    {Instruction::OpID::lor,[this](Instruction* instr)->Instruction* { return reducOr(instr); }},
    {Instruction::OpID::sub,[this](Instruction* instr)->Instruction* { return reducSub(instr); }},
    {Instruction::OpID::add,[this](Instruction* instr)->Instruction* { return reducAdd(instr); }},
    {Instruction::OpID::mul,[this](Instruction* instr)->Instruction* { return reducMul(instr); }},
    {Instruction::OpID::sdiv,[this](Instruction* instr)->Instruction* { return reducDiv(instr); }},
    {Instruction::OpID::lxor,[this](Instruction* instr)->Instruction* { return reducXor(instr); }},
    {Instruction::OpID::land,[this](Instruction* instr)->Instruction* { return reducAnd(instr); }},
    {Instruction::OpID::shl,[this](Instruction* instr)->Instruction* { return reducShl(instr); }},
    {Instruction::OpID::asr,[this](Instruction* instr)->Instruction* { return reducAsr(instr); }},
    // {Instruction::OpID::fsub,[this](Instruction* instr)->Instruction* { return combineFSub(instr); }},
    {Instruction::OpID::fadd,[this](Instruction* instr)->Instruction* { return reducFAdd(instr); }},
    {Instruction::OpID::fmul,[this](Instruction* instr)->Instruction* { return reducFMul(instr); }},
    {Instruction::OpID::srem,[this](Instruction* instr)->Instruction* { return reducRem(instr); }},

    // {Instruction::OpID::lsr,[this](Instruction* instr)->Instruction* { return combineMul(instr); }},
    // {Instruction::OpID::fdiv,[this](Instruction* instr)->Instruction* { return combineDiv(instr); }},

}{

}