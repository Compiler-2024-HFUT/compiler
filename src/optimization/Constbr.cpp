#include "optimization/Constbr.hpp"
#include "optimization/util.hpp"
static ConstantInt* const_true=nullptr;
static ConstantInt* const_false=nullptr;
void eraseBB(BasicBlock*bb);
void rmBBPhi(BasicBlock*valuefrom){
    auto _uselist=valuefrom->getUseList();
    for(auto [v,i ]:_uselist){
        if(auto phi=dynamic_cast<PhiInst*>(v)){
            phi->removeOperands(i-1,i);
            fixPhiOpUse(phi);
            // if(phi->getNumOperands()==2){
            //     phi->replaceAllUseWith(phi->getOperand(0));
            //     phi->getParent()->deleteInstr(phi);
            //     delete phi;
            // }
        }
    }
}
void rmBBPhi(BasicBlock*value_in,BasicBlock*valuefrom){
    auto _uselist=valuefrom->getUseList();
    for(auto [v,i ]:_uselist){
        if(auto phi=dynamic_cast<PhiInst*>(v)){
            if(phi->getParent()!=value_in)continue;
            phi->removeOperands(i-1,i);
            fixPhiOpUse(phi);
            // if(phi->getNumOperands()==2){
            //     phi->replaceAllUseWith(phi->getOperand(0));
            //     phi->getParent()->deleteInstr(phi);
            //     delete phi;
            // }
        }
    }
}

void findConstCond(BasicBlock*bb){
    auto termin=bb->getTerminator();
    if(!termin->isBr()||termin->getNumOperands()!=3)return ;
    auto condbr=static_cast<BranchInst*>(termin);
    auto cmp=condbr->getOperand(0);
    BasicBlock* toerase=nullptr;
        auto truebb=(BasicBlock*)condbr->getOperand(1);
        auto falsebb=(BasicBlock*)condbr->getOperand(2);
        if(cmp==const_true){
            toerase=falsebb;
            bb->deleteInstr(condbr);
            delete condbr;
            bb->removeSuccBasicBlock(falsebb);
            falsebb->removePreBasicBlock(bb);
            rmBBPhi(falsebb,bb);
            BranchInst::createBr(truebb,bb);
            bb->getSuccBasicBlocks().pop_back();
            truebb->getPreBasicBlocks().pop_back();
        }else if(cmp==const_false){
            toerase=truebb;
            bb->deleteInstr(condbr);
            delete  condbr;
            bb->removeSuccBasicBlock(truebb);
            truebb->removePreBasicBlock(bb);
            rmBBPhi(truebb,bb);
            BranchInst::createBr(falsebb,bb);
            bb->getSuccBasicBlocks().pop_back();
            falsebb->getPreBasicBlocks().pop_back();
        }
        if(toerase==nullptr||!toerase->getPreBasicBlocks().empty())return;
        eraseBB(toerase);
}
void eraseBB(BasicBlock*bb){
    if(bb==bb->getParent()->getEntryBlock())
        return;
    if(!bb->getPreBasicBlocks().empty())
        return;
    auto _uselist=bb->getUseList();
    rmBBPhi(bb);
    if(!bb->useEmpty()) {
        exit(235);}
    auto termin=bb->getTerminator();
    if(termin->isRet())
        exit(223); 

    std::list<BasicBlock*> succbbs=bb->getSuccBasicBlocks();
    bb->getParent()->removeBasicBlock(bb);
    bb->getSuccBasicBlocks().clear();
    for(auto succ:succbbs){
        succ->removePreBasicBlock(bb);
        if(succ->getPreBasicBlocks().empty())
            eraseBB(succ);
    }
    delete  bb;

}
void ConstBr::runOnFunc(Function*func){
    const_true=ConstantInt::get(true);
    const_false=ConstantInt::get(false);
    for(auto b:func->getBasicBlocks()){
        findConstCond(b);
    }
}