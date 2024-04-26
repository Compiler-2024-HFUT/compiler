#include "optimization/Mem2Reg.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <iostream>
#include <map>
#include <memory>
#include <set>

BasicBlock* Mem2Reg::isOnlyInOneBB(AllocaInst*ai){
    assert(!ai->getUseList().empty() && "There are no uses of the alloca!");
    auto &uses=ai->getUseList();
    BasicBlock*last=nullptr;
    for(auto use:uses){
        auto instr=dynamic_cast<Instruction*>(use.val_);
        
        assert(instr!=nullptr && "The use is not an instruction");
        
        if(last==nullptr) last=instr->getParent();
        else if(last!=instr->getParent()){
            return nullptr;
        }
    }
    return last;

}
void Mem2Reg::rmLocallyAlloc(AllocaInst*ai,BasicBlock* used_bb){
    assert(used_bb!=nullptr && "ai no used");
    
    //auto bb=static_cast<Instruction*>(ai->getUseList().front().val_)->getParent();
    auto &instrs=used_bb->getInstructions();
    Value* cur_val=nullptr;
    for(auto iter=instrs.begin();iter!=instrs.end();){
        auto ins=iter++;
        if(LoadInst* load_instr=dynamic_cast<LoadInst*>(*ins)){
            if(load_instr->getLVal()!=ai) continue;
            assert(cur_val!=nullptr&&"this val did not define before use");
            load_instr->replaceAllUseWith(cur_val);
            used_bb->deleteInstr(load_instr);
        }else if(StoreInst* store_instr=dynamic_cast<StoreInst*>(*ins)){
            if(store_instr->getLVal()!=ai) continue;
            cur_val=store_instr->getRVal();
            used_bb->deleteInstr(store_instr);
        }
    }
    ai->getParent()->deleteInstr(ai);
}
void Mem2Reg::calDefAndUse(AllocaInst*ai,::std::set<BasicBlock*>&def_bbs,::std::set<BasicBlock*>&use_bbs){
    assert(!ai->getUseList().empty() && "There are no uses of the alloca!");
    auto &uses=ai->getUseList();
    for(auto use:uses){
        auto instr=dynamic_cast<Instruction*>(use.val_);
        assert(instr!=nullptr && "The use is not an instruction");
        if(auto store_instr=dynamic_cast<StoreInst*>(instr)){
            def_bbs.insert(store_instr->getParent());
        }else if(auto load_instr=dynamic_cast<LoadInst*>(instr)){
            use_bbs.insert(load_instr->getParent());
        }else{
            assert(0 && "The use is not an instruction");
        }
    }
}
bool Mem2Reg::queuePhi(BasicBlock*bb,AllocaInst*ai,::std::set<PhiInst*>&phi_set){
    //找bb的phi节点表
    auto &bb_phi = new_phi[bb];
    for(auto bp:bb_phi){
        if(bp.first==ai)
            return false;
    }

    //有没有指向此alloc的phi
    auto phi=PhiInst::createPhi(ai->getAllocaType(),bb);
    bb->addInstrBegin(phi);
    
    bb_phi.insert({ai,phi});
    phi_set.insert(phi);
    
    return true;
}
void Mem2Reg::rmDeadPhi(Function*func){
        for(auto b:func->getBasicBlocks()){
            auto &instrs=b->getInstructions();
            for(auto iter=instrs.begin();iter!=instrs.end();){
                auto ins=iter++;
                if((*ins)->isPhi()&&(*ins)->getUseList().empty()){
                    b->deleteInstr(*ins);
                }
            }
        }

}
void Mem2Reg::reName(BasicBlock*bb,BasicBlock*pred,::std::map<AllocaInst*,Value*> incoming_vals){
    static ::std::set<BasicBlock*>visited;
    if(auto bb_alloc_phi=new_phi.find(bb);bb_alloc_phi!=new_phi.end()){
        auto&alloc_phi=bb_alloc_phi->second;
        for(auto &[ai ,phi]:alloc_phi){
            phi->addPhiPairOperand(incoming_vals.find(ai)->second,pred);
            incoming_vals[ai]=phi;
        }
    }
    
    if(visited.count(bb)) return ;
    
    visited.insert(bb);
    auto &instrs=bb->getInstructions();
    for(auto instr_iter=instrs.begin();instr_iter!=instrs.end();){
        auto instr=*instr_iter;
        instr_iter++;
        
        if(LoadInst*load_instr=dynamic_cast<LoadInst*>(instr)){
            auto lval=load_instr->getLVal();
            if(auto ai_lval=dynamic_cast<AllocaInst*>(lval)){
                auto it=incoming_vals.find(ai_lval);
                if(it==incoming_vals.end()) continue;
                Value* v=it->second;
                load_instr->replaceAllUseWith(v);
                bb->deleteInstr(load_instr);
            }
        }else if(StoreInst*store_instr=dynamic_cast<StoreInst*>(instr)){
            auto lval=store_instr->getLVal();
            if(auto ai_lval=dynamic_cast<AllocaInst*>(lval)){
                auto it=incoming_vals.find(ai_lval);
                if(it==incoming_vals.end()) continue;
                it->second=store_instr->getRVal();
                bb->deleteInstr(store_instr);
            }
        }
    }
    
    for(auto succ_bb:bb->getSuccBasicBlocks())
        reName(succ_bb, bb, incoming_vals);
}
void Mem2Reg::generatePhi(AllocaInst*ai,::std::set<BasicBlock*>&define_bbs){
    static ::std::set<PhiInst*> phi_set;
    while(!define_bbs.empty()){
        auto b=*define_bbs.rbegin();
        define_bbs.erase(b);
        if(b->getDomFrontier().empty())continue;
        auto &df_set=b->getDomFrontier();
        for(auto df:df_set)
            if (queuePhi(df, ai,phi_set))
                define_bbs.insert(df);
    }

}
void Mem2Reg::run(){
    for (auto func : moudle_->getFunctions()){
        auto &bb_list=func->getBasicBlocks();
        if(bb_list.empty())continue;
        ::std::unique_ptr<Dominators> dom=std::make_unique<Dominators>(func);
        cur_fun_dom=func_dom_.insert({func,std::move(dom)}).first;

        for(auto bb:func->getBasicBlocks()){
            for(auto instr:bb->getInstructions()){
                if(isAllocVar(instr)){
                    allocas.push_back(static_cast<AllocaInst*>(instr));
                }
            }
        }
        if(allocas.empty())continue;
        for(auto iter=allocas.begin();iter!=allocas.end() ;){
            auto i=iter++;
            auto ai=*i;
            
            //if no use
            if(ai->getUseList().empty()){
                ai->getParent()->deleteInstr(ai);
                allocas.erase(i);
            }
            //only used in one bb
            else if(auto bb=isOnlyInOneBB(ai)){
                rmLocallyAlloc(ai,bb);
                ai->getParent()->deleteInstr(ai);
                allocas.erase(i);
            }
            else{
                ::std::set<BasicBlock*>define_bbs;
                ::std::set<BasicBlock*>use_bbs;
                calDefAndUse(ai,define_bbs,use_bbs);
                generatePhi(ai,define_bbs);
            }
        }

        if(allocas.empty())
            continue;
        
        ::std::map<AllocaInst*,Value*> alloc_va;
        for(auto i:allocas){
            alloc_va.insert({i,ConstantInt::get(114514,moudle_)});
        }
        reName(func->getBasicBlocks().front(),nullptr,alloc_va);
        rmDeadPhi(func);

        for(auto ai_iter=allocas.begin();ai_iter!=allocas.end();){
            auto ai=*ai_iter;ai_iter++;
            if(!ai->getUseList().empty()){
                assert(0&& "do not define before use ");
            }
            ai->getParent()->deleteInstr(ai);
        }
    }
}

bool Mem2Reg::isAllocVar(Instruction *instr){
    if(instr->isAlloca()){
        AllocaInst *alloc=static_cast<AllocaInst*>(instr);
        return alloc->getAllocaType()->isFloatType()  ||alloc->getAllocaType()->isIntegerType();
    }else
        return false;

}