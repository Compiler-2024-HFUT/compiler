#include "optimization/ConstBrEli.hpp"
#include "midend/Instruction.hpp"
#include "optimization/util.hpp"
#include <algorithm>
#include <cassert>
static ConstantInt* const_true=nullptr;
static ConstantInt* const_false=nullptr;
std::set<BasicBlock*>erased;
ConstBr::ConstBr(Module*m, InfoManager *im) : FunctionPass(m, im){
    const_true=ConstantInt::get(true);
    const_false=ConstantInt::get(false);
}
void eraseBB(BasicBlock*bb);
/*
value_in:
    %1=phi[%0,valuefrom]，[%2，bb3]
-------------->
value_in:
    %1=phi[%2，bb3]
*/
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

bool constCondFold(BasicBlock*bb){
    auto termin=bb->getTerminator();
    /*多判断一下 */
    if(!termin->isBr()||termin->getNumOperands()!=3)return false;
    auto condbr=static_cast<BranchInst*>(termin);
    auto cmp=condbr->getOperand(0);
    // BasicBlock* toerase=nullptr;
    BasicBlock* toerase=nullptr;
    BasicBlock* succ_bb=nullptr;
    if(cmp==const_true){
        toerase=(BasicBlock*)condbr->getOperand(2);
        succ_bb=(BasicBlock*)condbr->getOperand(1);
    }else if(cmp==const_false){
        toerase=(BasicBlock*)condbr->getOperand(1);
        succ_bb=(BasicBlock*)condbr->getOperand(2);
    }
    if(toerase==nullptr)return false;

    bb->deleteInstr(condbr);
    delete condbr;
    bb->removeSuccBasicBlock(toerase);
    toerase->removePreBasicBlock(bb);
    rmBBPhi(toerase,bb);
    BranchInst::createBr(succ_bb,bb);
    /*
        createbr会添加这两个关系，要删掉
        if_true->addPreBasicBlock(bb);
        bb->addSuccBasicBlock(if_true);
    */
    bb->getSuccBasicBlocks().pop_back();
    succ_bb->getPreBasicBlocks().pop_back();

    if(toerase->getPreBasicBlocks().empty()){
        eraseBB(toerase);
        return true;
    }
    return false;
}
void eraseBB(BasicBlock*bb){
    if(bb==bb->getParent()->getEntryBlock())
        return;
    if(!bb->getPreBasicBlocks().empty())
        return;
    if(bb->getTerminator()->isRet())
        exit(223);

    erased.insert(bb);
    std::list<BasicBlock*> succbbs=bb->getSuccBasicBlocks();
    bb->getParent()->removeBasicBlock(bb);
    rmBBPhi(bb);
    for(auto succ:succbbs){
        if(erased.count(succ))continue;
        if(succ->getPreBasicBlocks().empty())
            eraseBB(succ);
    }
}
bool ConstBr::canFold(BasicBlock*bb){
    Instruction* termin=bb->getTerminator();
    if(!termin->isBr())
        return false;
    BranchInst*br=(BranchInst*)termin;
    //此bb的终结指令的第一个op是constexpr
    if(dynamic_cast<Constant*>(br->getOperand(0)))
        return true;
    return false;
}
void ConstBr::runOnFunc(Function*func){
    auto &bbs=func->getBasicBlocks();
    if(bbs.empty())return;
    erased.clear();
    for(auto iter=bbs.begin();iter!=bbs.end();){
        if(!canFold(*iter)){
            ++iter;
            continue;
        }
        //删除bb可能出问题，从头开始吧
        if(constCondFold(*iter))
            iter=bbs.begin();
    }
    std::list<Instruction*> to_del;
    for(auto b:erased){
        std::copy(b->getInstructions().begin(),b->getInstructions().end(),to_del.end());
    }
    for(Instruction* i:to_del){
        i->removeUseOfOps();
    }
    for(auto i:to_del){
        assert(i->getUseList().empty());
        delete i;
    }
    for(auto b:erased){
        delete b;
    }
}
