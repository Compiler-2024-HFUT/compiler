#include "optimization/ConstBrEli.hpp"
#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "optimization/util.hpp"
#include <cassert>
#include <cstdio>
#include <list>
#include <vector>

ConstBr::ConstBr(Module*m, InfoManager *im) : FunctionPass(m, im){
    const_true=ConstantInt::get(true);
    const_false=ConstantInt::get(false);
}

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
            if(phi->getNumOperands()==2&&phi->getParent()->getPreBasicBlocks().size()<2){
                phi->replaceAllUseWith(phi->getOperand(0));
                phi->getParent()->deleteInstr(phi);
                delete phi;
            }
        }
    }
}

bool ConstBr::constCondFold(BasicBlock*bb){
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

    bb->getInstructions().pop_back();
    condbr->removeUseOfOps();
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
void ConstBr::eraseBB(BasicBlock*bb){
    if(bb==bb->getParent()->getEntryBlock())
        return;
    if(!bb->getPreBasicBlocks().empty())
        return;
    assert(!bb->getTerminator()->isRet());

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
Modify ConstBr::unReachableBBEli(Function*func){
    Modify ret{};
    std::vector<Instruction*>to_del;
    erased.clear();
    std::set<BasicBlock*>reachable;
    std::list<BasicBlock*>work{func->getEntryBlock()};
    while(!work.empty()){
        auto b=work.front();
        work.pop_front();
        reachable.insert(b);
        for(auto s:b->getSuccBasicBlocks()){
            if(reachable.count(s))
                continue;
            work.push_back(s);
            reachable.insert(s);
        }
    }
    auto toeraselist=func->getBasicBlocks();
    for(auto b:toeraselist){
        if(reachable.count(b))
            continue;
        func->removeBasicBlock(b);
        erased.insert(b);
        to_del.insert(to_del.end(),b->getInstructions().begin(),b->getInstructions().end());
    }
    for(Instruction* i:to_del){
        i->removeUseOfOps();
    }
    for(auto b:erased){
        rmBBPhi(b);
    }
    for(auto i:to_del){
        assert(i->getUseList().empty());
        delete i;
    }
    if(!erased.empty()){
        ret.modify_bb=true;
        ret.modify_instr=true;
    }
    for(auto b:erased){
        delete b;
    }
    return ret;
}
Modify ConstBr::runOnFunc(Function*func){
    Modify ret{};
    auto &bbs=func->getBasicBlocks();
    if(bbs.empty())return ret;
    bool changed=true;
    do{
        erased.clear();
        changed=false;
        // for(auto b:bbs){
        //     for(auto i:b->getInstructions()){
        //         if((dynamic_cast<BinaryInst*>(i))){
        //             auto lhs=dynamic_cast<Constant*>(i->getOperand(0)),rhs=dynamic_cast<Constant*>(i->getOperand(1));
        //             if(lhs&&rhs){
        //                 i->replaceAllUseWith(Constant::get(lhs,i->getInstrType(),rhs));
        //             }
        //         }else if((i->isCmp())){
        //             auto lhs=dynamic_cast<ConstantInt*>(i->getOperand(0)),rhs=dynamic_cast<ConstantInt*>(i->getOperand(1));
        //             auto cmpop=((CmpInst*)i)->getCmpOp();
        //             if(lhs&&rhs){
        //                 i->replaceAllUseWith(ConstantInt::getFromICmp(lhs,cmpop,rhs));
        //             }
        //         }
        //     }
        // }
        for(auto iter=bbs.begin();iter!=bbs.end();){
            if(!canFold(*iter)){
                ++iter;
                continue;
            }
            //删除bb可能出问题，从头开始吧
            if(constCondFold(*iter)){
                ret.modify_bb=true;
                ret.modify_instr=true;
                changed=true;
                iter=bbs.begin();
            }
        }
        std::vector<Instruction*> to_del;
        for(auto b:erased){
            // to_del.assign(b->getInstructions().begin(), b->getInstructions().end());
            to_del.insert(to_del.end(),b->getInstructions().begin(),b->getInstructions().end());
        }
        for(Instruction* i:to_del){
            i->removeUseOfOps();
        }
        for(auto b:erased){
            delete b;
        }
        auto &bl=func->getBasicBlocks();
        for(auto b:bl){
            if(b->getPreBasicBlocks().empty())
                if(b!=bl.front()){
                    bl.remove(b);
                    bl.push_front(b);
                    break;
                }
        }
        auto unr_ret=unReachableBBEli(func);
        ret=ret|unr_ret;
        changed|=unr_ret.modify_instr;
        for(auto i:to_del){
            assert(i->getUseList().empty());
            delete i;
        }
    }while(changed);
    return ret;
}
