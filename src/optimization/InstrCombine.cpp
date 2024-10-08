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
            if(!bin_lhs->useOne())
                return 0;
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
        }else if(auto bin_rhs=dynamic_cast<BinaryInst*>(rhs);bin_rhs&&bin_rhs->getInstrType()==op&&bin_lhs->getInstrType()==op&&dynamic_cast<Constant*>(bin_lhs->getOperand(1))&&dynamic_cast<Constant*>(bin_rhs->getOperand(1))){
        //add (add v1 c1) (add v2 c2) ===> add v1 v2 (c2+c1)
            if(!bin_lhs->useOne()||!bin_rhs->useOne())
                return 0;
            auto c1=bin_lhs->getOperand(1);
            auto c2=bin_rhs->getOperand(1);
            bin_lhs->replaceOperand(1,bin_rhs->getOperand(0));
            bin_rhs->replaceOperand(0,c1);
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
            // a/9*3
            if(bin_lhs->isDiv()&&dynamic_cast<ConstantInt*>(bin_lhs->getOperand(1))){
                auto lhs_op1=(ConstantInt*)(bin_lhs->getOperand(1));
                if(cr->getValue()%lhs_op1->getValue()==0){
                    return BinaryInst::create(Instruction::OpID::mul,bin_lhs->getOperand(0),ConstantInt::get(cr->getValue()/lhs_op1->getValue()));
                }else if(lhs_op1->getValue()%cr->getValue()==0){
                    return BinaryInst::create(Instruction::OpID::sdiv,bin_lhs->getOperand(0),ConstantInt::get(lhs_op1->getValue()/cr->getValue()));
                }
            }else if(bin_lhs->isMul()&&dynamic_cast<ConstantInt*>(bin_lhs->getOperand(1))){
                auto lhs_op1=(ConstantInt*)(bin_lhs->getOperand(1));
                    return BinaryInst::create(Instruction::OpID::mul,bin_lhs->getOperand(0),ConstantInt::get(cr->getValue()*lhs_op1->getValue()));
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
    if(__is__neg(instr)){
        if(auto r_ins=dynamic_cast<Instruction*>(rhs)){
            if(r_ins->isSub())
            return BinaryInst::create(Instruction::OpID::sub, r_ins->getOperand(1),r_ins->getOperand(0));
        }
    }
    auto bin_rhs=dynamic_cast<Instruction*>(rhs);
    if(bin_rhs){
        if(bin_rhs->isSub()){
            //c1-(c2-a)==>(c1-c2)+a ==>c3+a==>a+c3
            if(auto rhs_lhs_ci=dynamic_cast<ConstantInt*>(bin_rhs->getOperand(0));rhs_lhs_ci&&l_const){
                auto new_lhs=ConstantInt::getFromBin(l_const,Instruction::OpID::sub,rhs_lhs_ci);
                return BinaryInst::create(Instruction::OpID::add,bin_rhs->getOperand(1),new_lhs);
            }
        }else if(bin_rhs->isAdd()&&bin_rhs->useOne()){
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
    auto bin_lhs=dynamic_cast<Instruction*>(lhs);
    if(bin_lhs){
        if(bin_lhs->isAdd()){
            //(b+ a) - b => a
            if(bin_lhs->getOperand(0)==rhs){
                return replaceInstUsesWith(instr,bin_lhs->getOperand(1));
            //(a + b) - b => a
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
    auto blhs=dynamic_cast<Instruction*>(lhs),brhs=dynamic_cast<Instruction*>(rhs);
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
            if(blhs->isAdd()&&blhs->useOne()){
                if(auto lhs_rhs_ci=dynamic_cast<ConstantInt*>(blhs->getOperand(1))){
                    instr->replaceOperand(0,blhs->getOperand(0));
                    instr->replaceOperand(1,ConstantInt::getFromBin(cons_r,Instruction::OpID::add,lhs_rhs_ci));
                    ret=instr;
                }
            //(-a)+b ==> b-a
            //a+(-b) ==> a-b
            }else if(__is__neg(blhs)){
                return BinaryInst::create(Instruction::OpID::sub,rhs,blhs->getOperand(1));
            }
            return ret;
        }
    }
    if(blhs&&blhs->isMul()&&blhs->getOperand(0)==rhs&&blhs->useOne()){
        if(auto lhs_rhs_ci=dynamic_cast<ConstantInt*>(blhs->getOperand(1))){
            blhs->replaceOperand(1,ConstantInt::get(lhs_rhs_ci->getValue()+1));
            return replaceInstUsesWith(instr,blhs);
        }
    }
    if(brhs){
        if(__is__neg(brhs))
            return BinaryInst::create(Instruction::OpID::sub,lhs,brhs->getOperand(1));
        //a+(a*c1)==>a*(c1+1)
        else if(brhs->isMul()&&brhs->getOperand(0)==lhs&&brhs->useOne()){
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
    }if(cr!=nullptr){
        if(cr->getValue()==1){
            return replaceInstUsesWith(instr,lhs);
        }else if(cr->getValue()==-1){
            return BinaryInst::create(Instruction::OpID::sub,ConstantInt::get(0),lhs);
        }else{
            auto lhs_bin=dynamic_cast<BinaryInst*>(lhs);
            if(lhs_bin==0)return ret;
            //a*c1/c2 ==>a* (c1/c2)
            if(lhs_bin->isMul()){
                if(auto lhs_rhs_const=dynamic_cast<ConstantInt*>(lhs_bin->getOperand(1))){
                    if(lhs_rhs_const->getValue()%cr->getValue()==0){
                        ret=BinaryInst::create(Instruction::OpID::mul,lhs_bin->getOperand(0),
                            ConstantInt::get(lhs_rhs_const->getValue()/cr->getValue()));
                    }else if(cr->getValue()%lhs_rhs_const->getValue()==0){
                        ret=BinaryInst::create(Instruction::OpID::sdiv,lhs_bin->getOperand(0),
                            ConstantInt::get(cr->getValue()/lhs_rhs_const->getValue()));
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
    if(auto bin_rhs=__is__neg(rhs)){
        if(bin_rhs->getOperand(1)==lhs){
            return replaceInstUsesWith(instr,ConstantInt::get(-1));
        }
    }
    if(auto bin_lhs=__is__neg(lhs)){
        if(bin_lhs->getOperand(1)==rhs){
            return replaceInstUsesWith(instr,ConstantInt::get(-1));
        }
    }
    return ret;
}
Instruction* InstrCombine::combineRem(Instruction*instr){
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    // 0 % a -> 0
    if(auto cl=dynamic_cast<ConstantInt*>(lhs)){
        if(cl->getValue()==0){
            return replaceInstUsesWith(instr,cl);
        }
    }
    // a % 1 -> 0
    if(auto cr=dynamic_cast<ConstantInt*>(rhs)){
        if(cr->getValue()==1){
            return replaceInstUsesWith(instr,ConstantInt::get(0));
        }
    }
    // a % a -> 0
    if(rhs==lhs){
        return replaceInstUsesWith(instr,ConstantInt::get(0));
    }

    return 0;
}
Instruction* InstrCombine::combineFAdd(Instruction*instr){
    Instruction*ret=_simplify_bin((BinaryInst*)instr);
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    auto blhs=dynamic_cast<Instruction*>(lhs);
    auto brhs=dynamic_cast<Instruction*>(rhs);
    if(auto cons_r=dynamic_cast<ConstantFP*>(rhs)){
        if(cons_r->getValue()==0.0)
            return replaceInstUsesWith(instr,lhs);
        //(a - c1) + c2 => a + (c2 - c1)
        if(blhs){
            if(blhs->isFAdd()&&blhs->useOne()){
                if(auto lhs_rhs_ci=dynamic_cast<ConstantFP*>(blhs->getOperand(1))){
                    instr->replaceOperand(0,blhs->getOperand(0));
                    instr->replaceOperand(1,ConstantFP::getFromBin(cons_r,Instruction::OpID::fadd,lhs_rhs_ci));
                    ret=instr;
                    return ret;
                }

            }
        }
        return ret;
    }
            //(-a)+b ==> b-a
            //a+(-b) ==> a-b
    if(blhs&&__is__neg(blhs)){
                return BinaryInst::create(Instruction::OpID::fsub,rhs,blhs->getOperand(1));
            }
    if(brhs&&__is__neg(brhs)){
        return BinaryInst::create(Instruction::OpID::fsub,lhs,brhs->getOperand(1));
    }
    //(a*c1)+a==>a*(c1+1)
    if(blhs&&blhs->isFMul()&&blhs->getOperand(0)==rhs&&blhs->useOne()){
        if(auto lhs_rhs_ci=dynamic_cast<ConstantFP*>(blhs->getOperand(1))){
            blhs->replaceOperand(1,ConstantFP::get(lhs_rhs_ci->getValue()+1.f));
            return replaceInstUsesWith(instr,blhs);
        }
    }
    return ret;
}
Instruction* InstrCombine::combineFSub(Instruction*instr){
    Instruction*ret=0;
    auto lhs=instr->getOperand(0),rhs=instr->getOperand(1);
    auto l_const=dynamic_cast<ConstantFP*>(lhs),r_const=dynamic_cast<ConstantFP*>(rhs);
    //a-a ==>0
    if(lhs==rhs)
        return replaceInstUsesWith(instr,ConstantFP::get(0));
    // a- (0-b)==>a+b
    if(__is__neg(rhs)){
        return BinaryInst::create(Instruction::OpID::fadd, lhs, ((Instruction*)rhs)->getOperand(1));
    }
    if(__is__neg(instr)){
        if(auto r_ins=dynamic_cast<Instruction*>(rhs)){
            if(r_ins->isSub())
            return BinaryInst::create(Instruction::OpID::fsub, r_ins->getOperand(1),r_ins->getOperand(0));
        }
    }
    auto bin_rhs=dynamic_cast<Instruction*>(rhs);
    if(bin_rhs){
        if(bin_rhs->isFSub()){
            //c1-(c2-a)==>(c1-c2)+a ==>c3+a==>a+c3
            if(auto rhs_lhs_ci=dynamic_cast<ConstantFP*>(bin_rhs->getOperand(0));rhs_lhs_ci&&l_const){
                auto new_lhs=ConstantFP::getFromBin(l_const,Instruction::OpID::fsub,rhs_lhs_ci);
                return BinaryInst::create(Instruction::OpID::add,bin_rhs->getOperand(1),new_lhs);
            }
        }else if(bin_rhs->isFAdd()&&bin_rhs->useOne()){
            //c1-(c2+a)==>(c1-c2)-a ==>c3-a
            if(auto rhs_lhs_ci=dynamic_cast<ConstantFP*>(bin_rhs->getOperand(0));rhs_lhs_ci&&l_const){
                instr->replaceOperand(0,ConstantFP::getFromBin(l_const,Instruction::OpID::fsub,rhs_lhs_ci));
                instr->replaceOperand(1,bin_rhs->getOperand(1));
                return instr;
            }
        }
        //a-(a+b)==>-b
        if(bin_rhs->isFAdd()){
            if(bin_rhs->getOperand(0)==lhs){
                return BinaryInst::create(Instruction::OpID::fsub, ConstantFP::get(0), bin_rhs->getOperand(1));
            }else if(bin_rhs->getOperand(1)==lhs){
                //a-(b+a)
                return BinaryInst::create(Instruction::OpID::fsub, ConstantFP::get(0), bin_rhs->getOperand(0));
            }
        }
    }
    auto bin_lhs=dynamic_cast<Instruction*>(lhs);
    if(bin_lhs){
        if(bin_lhs->isFAdd()){
            //(b+ a) - b => a
            if(bin_lhs->getOperand(0)==rhs){
                return replaceInstUsesWith(instr,bin_lhs->getOperand(1));
            //(a + b) - b => a
            }else if(bin_lhs->getOperand(1)==rhs){
                return replaceInstUsesWith(instr,bin_lhs->getOperand(0));
            }
        }
    }
    

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
        //  a<=-1  ===>a<0
        if(ci_rhs->getValue()==-1){
            if(instr->getCmpOp()==LE){
                auto ret=CmpInst::createCmp(LT,lhs,ConstantInt::get(0),instr->getParent());
                instr->getParent()->getInstructions().pop_back();
                ret->setParent(0);
                return ret;
            }        //  a>=1  a>0
        }else if(ci_rhs->getValue()==1){
                if(instr->getCmpOp()==GE){
                auto ret=CmpInst::createCmp(GT,lhs,ConstantInt::get(0),instr->getParent());
                instr->getParent()->getInstructions().pop_back();
                ret->setParent(0);
                return ret;
            }
        }if(auto bin_lhs=dynamic_cast<BinaryInst*>(lhs)){
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
    {Instruction::OpID::srem,[this](Instruction* instr)->Instruction* { return combineRem(instr); }},
    {Instruction::OpID::lxor,[this](Instruction* instr)->Instruction* { return combineXor(instr); }},
    {Instruction::OpID::land,[this](Instruction* instr)->Instruction* { return combineAnd(instr); }},
    {Instruction::OpID::shl,[this](Instruction* instr)->Instruction* { return combineShl(instr); }},
    {Instruction::OpID::asr,[this](Instruction* instr)->Instruction* { return combineAsr(instr); }},
    {Instruction::OpID::cmp,[this](Instruction* instr)->Instruction* { return combineCmp(instr); }},
    {Instruction::OpID::fadd,[this](Instruction* instr)->Instruction* { return combineFAdd(instr); }},
    {Instruction::OpID::fsub,[this](Instruction* instr)->Instruction* { return combineFSub(instr); }},
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
    // {Instruction::OpID::sdiv,[this](Instruction* instr)->Instruction* { return reducDiv(instr); }},
    {Instruction::OpID::lxor,[this](Instruction* instr)->Instruction* { return reducXor(instr); }},
    {Instruction::OpID::land,[this](Instruction* instr)->Instruction* { return reducAnd(instr); }},
    {Instruction::OpID::shl,[this](Instruction* instr)->Instruction* { return reducShl(instr); }},
    {Instruction::OpID::asr,[this](Instruction* instr)->Instruction* { return reducAsr(instr); }},
    // {Instruction::OpID::fsub,[this](Instruction* instr)->Instruction* { return combineFSub(instr); }},
    {Instruction::OpID::fadd,[this](Instruction* instr)->Instruction* { return reducFAdd(instr); }},
    {Instruction::OpID::fmul,[this](Instruction* instr)->Instruction* { return reducFMul(instr); }},
    // {Instruction::OpID::srem,[this](Instruction* instr)->Instruction* { return reducRem(instr); }},

    // {Instruction::OpID::lsr,[this](Instruction* instr)->Instruction* { return combineMul(instr); }},
    // {Instruction::OpID::fdiv,[this](Instruction* instr)->Instruction* { return combineDiv(instr); }},

}{

}