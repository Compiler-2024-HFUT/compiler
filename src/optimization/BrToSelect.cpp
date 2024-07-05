#include "optimization/BrToSelect.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include <iostream>
#include <ostream>
//riscv只支持max,min
void BrToSelect::canGenSel(BasicBlock*bb){
    bb_info_.type=ToSelectInfo::EMPTY;
    BranchInst* br;

    if(auto termin=bb->getTerminator()){
        if(!termin->isBr())return ;
        br=(BranchInst*)termin;
    }else
        return ;
    if(br->getNumOperands()==1)
        return ;
    if(dynamic_cast<CmpInst*>(br->getOperand(0))==0)
        return ;
    CmpInst* cmp=(CmpInst*)br->getOperand(0);
    auto cmpop=cmp->getCmpOp();
    if(cmpop==CmpOp::EQ||cmpop==CmpOp::NE)
        return ;

    // auto lhs=cmp->getOperand(0),rhs=cmp->getOperand(1);

    auto truebb=(BasicBlock*)(br->getOperand(1)),falsebb=(BasicBlock*)(br->getOperand(2));
    if(truebb->getPreBasicBlocks().size()!=1||truebb->getSuccBasicBlocks().size()!=1)
        return ;
    if(truebb->getInstructions().size()>1)
        return;

    //type1 无else
    if(truebb->getSuccBasicBlocks().front()==falsebb){
        if(falsebb->getPreBasicBlocks().size()!=2)return;
        if(falsebb==bb||falsebb==truebb)return;
        auto&instrs=falsebb->getInstructions();
        PhiInst*  phi=nullptr;
        for(auto ins:instrs){
            if(ins->isPhi())
                if(phi==nullptr)
                    phi=(PhiInst*)ins;
                else
                    return;
            else break;
        }
        if(phi==nullptr)    return;
        if(phi->getNumOperands()!=4 )return;
        bb_info_.type=ToSelectInfo::IF_SEL;
        bb_info_.cmp=cmp;
        // bb_info_.br=br;
        bb_info_.true_bb=truebb;
        bb_info_.false_bb=falsebb;
        bb_info_.intersection_bb=falsebb;
        bb_info_.phi=phi;
    }
    //type2 有else
    if(falsebb->getPreBasicBlocks().size()!=1||falsebb->getSuccBasicBlocks().size()!=1)
        return ;
    if(falsebb->getInstructions().size()>1)
        return;
    auto intersection_bb=truebb->getSuccBasicBlocks().front();
    //交汇在同一个bb,且bb只能有两前驱
    if(intersection_bb!=falsebb->getSuccBasicBlocks().front()||intersection_bb->getPreBasicBlocks().size()!=2)
        return ;
    //只能有一个phi
    PhiInst*  phi=nullptr;
    for(auto ins:intersection_bb->getInstructions()){
        if(ins->isPhi())
            if(phi==nullptr)
                phi=(PhiInst*)ins;
            else
                return;
        else break;
    }
    if(phi==nullptr)    return;
    if(phi->getNumOperands()!=4 )return;
    bb_info_.type=ToSelectInfo::IF_ELSE_SEL;
    bb_info_.cmp=cmp;
    // bb_info_.br=br;
    bb_info_.true_bb=truebb;
    bb_info_.false_bb=falsebb;
    bb_info_.intersection_bb=intersection_bb;
    bb_info_.phi=phi;
    return;
}

void BrToSelect::runOnFunc(Function*func){
    if(func->isDeclaration())return;
    auto &bbs=func->getBasicBlocks();
    for(auto iter=bbs.begin();iter!=bbs.end();){
        auto b=*iter;
        ++iter;
        this->canGenSel(b);
        if(bb_info_.type==ToSelectInfo::EMPTY)continue;

        CmpInst*cmp=bb_info_.cmp;
        Value* lhs=cmp->getOperand(0),* rhs=cmp->getOperand(1);
        CmpOp cmpop=cmp->getCmpOp();
        BasicBlock* intersection_bb=bb_info_.intersection_bb;
        PhiInst* phi=bb_info_.phi;
        auto &instrs=b->getInstructions();
        Value* true_val,*false_val;
        if(phi->getOperand(1)==bb_info_.true_bb){
            true_val=phi->getOperand(0);
            false_val=phi->getOperand(2);
        }else{
            true_val=phi->getOperand(2);
            false_val=phi->getOperand(0);
        }
        if(lhs==false_val&&rhs==true_val){
            cmp->removeAllOperand();
            cmp->addOperand(rhs);
            cmp->addOperand(lhs);
            cmp->negation();
            lhs=cmp->getOperand(0);
            rhs=cmp->getOperand(1);
        }
        if(lhs!=true_val||rhs!=false_val){
            std::cout<<true_val->print()<<std::endl;
            std::cout<<lhs->print()<<std::endl;
            continue;
        }
        iter=bbs.begin();
        //删除最后的跳转
        auto old_br=instrs.back();
        instrs.pop_back();
        old_br->removeUseOfOps();
        delete old_br;

        SelectInst*sel;
        if(bb_info_.type==ToSelectInfo::IF_SEL){
            sel=SelectInst::createSelect(phi->getType(),cmp,true_val,false_val,b);
            BranchInst::createBr(bb_info_.intersection_bb,b);
        }else{
            sel=SelectInst::createSelect(phi->getType(),cmp,true_val,false_val,b);
            BranchInst::createBr(bb_info_.intersection_bb,b);
            auto falsebb=bb_info_.false_bb;
            func->removeBasicBlock(falsebb);
            auto _ins=falsebb->getInstructions().front();
            _ins->removeUseOfOps();
            delete _ins;
        }
        phi->getParent()->deleteInstr(phi);
        auto ul=phi->getUseList();
        // for(auto [v,i ]:ul){
        //     if(auto _phi=dynamic_cast<PhiInst*>(v)){
        //         _phi->replaceOperand(i+1,b);
        //     }
        // }
        auto truebb=bb_info_.true_bb;
        func->removeBasicBlock(truebb);
        auto _ins=truebb->getInstructions().front();
        _ins->removeUseOfOps();
        delete _ins;
        phi->replaceAllUseWith(sel);
        delete phi;
    }
}
