#include "optimization/G2L.hpp"
#include "analysis/Dominators.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include <cassert>
#include <vector>
static ::std::set<BasicBlock*>visited;
static set<StoreInst*>to_delete;
BasicBlock* isOnlyInOneBB(GlobalVariable*global){
    // assert(!global->getUseList().empty() && "There are no uses of the alloca!");
    auto &uses=global->getUseList();
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
void G2L::rmLocallyGlob(GlobalVariable*global,BasicBlock* used_bb,Value*incoming){
    assert(used_bb!=nullptr && "global no used");
    
    auto &instrs=used_bb->getInstructions();
    Value* cur_val=incoming;
    Value*final_store=incoming;
    bool _stored=false;
    for(auto _iter=instrs.begin();_iter!=instrs.end();){
        auto cur_iter=_iter++;
        if(LoadInst* load_instr=dynamic_cast<LoadInst*>(*cur_iter)){
            if(load_instr->getLVal()!=global) continue;
            if(cur_val==nullptr){
                cur_val=load_instr;
                continue;
            }
            load_instr->replaceAllUseWith(cur_val);
            used_bb->eraseInstr(cur_iter);
        }else if(StoreInst* store_instr=dynamic_cast<StoreInst*>(*cur_iter)){
            if(store_instr->getLVal()!=global) continue;
            cur_val=store_instr->getRVal();
            used_bb->eraseInstr(cur_iter);
            _stored=true;
        }else if(CallInst*call=dynamic_cast<CallInst*>(*cur_iter)){
            auto call_func=call->getOperand(0);
            if(use_list_.count((Function*)call_func)&&_stored){
                auto s=StoreInst::createStore(cur_val,global,used_bb);
                used_bb->getInstructions().pop_back();
                _iter=used_bb->getInstructions().insert(cur_iter,s);
                final_store=cur_val;
            }
            if(def_list_.count((Function*)call_func)){
                cur_val=LoadInst::createLoad(global->getType()->getPointerElementType(),global,used_bb);
                used_bb->getInstructions().pop_back();
                _iter=used_bb->getInstructions().insert(cur_iter,(Instruction*)cur_val);
                // to_delete.insert((Instruction*)cur_val);
            }
        }
    }
    if(cur_val!=final_store){
        auto term_ins=used_bb->getInstructions().back();
        if(term_ins->getFunction()==module_->getMainFunction())
            return;
        used_bb->getInstructions().pop_back();
        StoreInst::createStore(cur_val,global,used_bb);
        used_bb->getInstructions().push_back(term_ins);
    }
}
void G2L::calOneGlobal(){

    for(Use u:cur_global->getUseList()){
        Instruction* ins=dynamic_cast<Instruction*>(u.val_);
        assert(ins&&"error");
        Function* func=ins->getFunction();

        if(auto glo_ins_iter=global_instrs_.find(func);glo_ins_iter!=global_instrs_.end()){
            glo_ins_iter->second.push_back(ins);
        }else
            global_instrs_.insert({func,{ins}});
        // du_list_.insert(func);
        if(LoadInst* load=dynamic_cast<LoadInst*>(ins)){
            use_list_.insert(func);
        }else if(StoreInst* store=dynamic_cast<StoreInst*>(ins)){
            def_list_.insert(func);
        }else
            assert(0&&"error");
        // }else if(CallInst* call=dynamic_cast<CallInst*>(ins)){
        //         use_iter->second.insert(func);
        //         def_iter->second.insert(func);
    }
    bool changed=true;
    while (changed) {
        changed=false;
        for(auto f:module_->getFunctions()){
            if(f->getBasicBlocks().empty())continue;

            for(auto u:f->getUseList()){
                auto call=dynamic_cast<CallInst*>(u.val_);
                assert(call&&"error");
                auto call_func=call->getFunction();
                if(def_list_.count(f))
                    if(!def_list_.count(call_func)){
                        def_list_.insert(call_func);
                        // du_list_.insert(call_func);
                        changed=true;
                    }
                if(use_list_.count(f))
                    if(!use_list_.count(call_func)){
                        use_list_.insert(call_func);
                        // du_list_.insert(call_func);
                        changed=true;
                    }
            }
        }
    }
}
void G2L::calDefAndUse(Function*cur_func,::std::set<BasicBlock*>&def_bbs,::std::set<BasicBlock*>&use_bbs){
    for(auto bb:cur_func->getBasicBlocks()){
        for(auto instr_iter=bb->getInstructions().begin();instr_iter!=bb->getInstructions().end();){
            auto cur_iter=instr_iter++;
            auto instr=*cur_iter;
            if(auto store_instr=dynamic_cast<StoreInst*>(instr)){
                if(store_instr->getLVal()==cur_global)
                    def_bbs.insert(bb);
            }else if(auto load_instr=dynamic_cast<LoadInst*>(instr)){
                if(load_instr->getLVal()==cur_global)
                    use_bbs.insert(bb);
            }else if(auto call=dynamic_cast<CallInst*>(instr)){
                auto func=(Function*)call->getOperand(0);
                if(def_list_.count(func))
                    def_bbs.insert(bb);
                if(use_list_.count(func))
                    use_bbs.insert(bb);
            }
        }
    }
}
void G2L::generatePhi(::std::set<BasicBlock*>&define_bbs,::std::set<PhiInst*> &phi_set){
    while(!define_bbs.empty()){
        auto b=*define_bbs.rbegin();
        define_bbs.erase(b);
        if(cur_dom_->getDomFrontier(b).empty())continue;
        auto &df_set=cur_dom_->getDomFrontier(b);
        for(auto df:df_set)
            if (queuePhi(df, phi_set))
                define_bbs.insert(df);
    }

}
bool G2L::queuePhi(BasicBlock*bb,::std::set<PhiInst*>&phi_set){
    //找bb的phi节点表
    if(new_phi.find(bb)!=new_phi.end())
        return false;
    auto phi=PhiInst::createPhi(cur_global->getType()->getPointerElementType(),bb);
    bb->addInstrBegin(phi);
    new_phi.insert({bb,phi});
    phi_set.insert(phi);
    return true;
}
bool G2L::needStore(Value* incoming,Value*last_store){
    // if(incoming==nullptr)return false;
    if(incoming==last_store)
        return false;
    // if(auto phi=dynamic_cast<PhiInst*>(incoming)){
    
    // }
    return true;
}
void G2L::reName(BasicBlock*bb,BasicBlock*pred,Value* incoming_val,Value*last_store){
    if(auto bb_phi=new_phi.find(bb);bb_phi!=new_phi.end()){
        auto&_phi=bb_phi->second;
        if(incoming_val!=nullptr)
            _phi->addPhiPairOperand(incoming_val,pred);
        incoming_val=_phi;
    }
    if(visited.count(bb)) return ;
    
    visited.insert(bb);
    auto &instrs=bb->getInstructions();
    for(auto instr_iter=instrs.begin();instr_iter!=instrs.end();){
        auto cur_iter=instr_iter++;
        auto instr=*cur_iter;
        // std::cout<<"111"<<std::endl;
        if(LoadInst*load_instr=dynamic_cast<LoadInst*>(instr)){
            auto lval=load_instr->getLVal();
            if(lval!=cur_global)continue;
            if(incoming_val!=nullptr){
                load_instr->replaceAllUseWith(incoming_val);
                bb->eraseInstr(cur_iter);
            }else
                incoming_val=load_instr;
        }else if(StoreInst*store_instr=dynamic_cast<StoreInst*>(instr)){
            auto lval=store_instr->getLVal();
            if(lval!=cur_global)continue;
                incoming_val=store_instr->getRVal();
            bb->eraseInstr(cur_iter);
        }else if(CallInst*call=dynamic_cast<CallInst*>(*cur_iter)){
            auto call_func=call->getOperand(0);
            if(use_list_.count((Function*)call_func)&&needStore(incoming_val,last_store)){
                auto s=StoreInst::createStore(incoming_val,cur_global,bb);
                bb->getInstructions().pop_back();
                bb->getInstructions().insert(cur_iter,s);
                // final_store=incoming_val;
                // stored.insert(incoming_val);
                last_store=incoming_val;
                to_delete.insert(s);
            }
            if(def_list_.count((Function*)call_func)){
                incoming_val=LoadInst::createLoad(cur_global->getType()->getPointerElementType(),cur_global,bb);
                bb->getInstructions().pop_back();
                bb->getInstructions().insert(instr_iter,(Instruction*)incoming_val);
                // to_delete.insert((Instruction*)incoming_val);
                // stored.insert(incoming_val);
                last_store=incoming_val;
            }
        }
        // std::cout<<"112"<<std::endl;
        
    }
    if(bb->getSuccBasicBlocks().empty()&&needStore(incoming_val,last_store)){
        auto ret=bb->getInstructions().back();
        if(ret->getFunction()==module_->getMainFunction())
            return;
        bb->getInstructions().pop_back();
        to_delete.insert(StoreInst::createStore(incoming_val,cur_global,bb));
        bb->getInstructions().push_back(ret);
    }
    // std::cout<<"113"<<std::endl;
    for(auto succ_bb:bb->getSuccBasicBlocks())
        reName(succ_bb, bb, incoming_val,last_store);
}

void G2L::runGlobal(){
    for(auto func:module_->getFunctions()){
        if(func->getBasicBlocks().empty())continue;
        if(!global_instrs_.count(func))continue;
        //only def continue;
        if(!use_list_.count(func))continue;
        if(auto bb=isOnlyInOneBB(cur_global);bb&&cur_dom_->getDomFrontier(bb).empty()){
            if(func==module_->getMainFunction())
                rmLocallyGlob(cur_global,bb,cur_global->getInit());
            else
                rmLocallyGlob(cur_global,bb,nullptr);
            continue;
        }

        // cur_dom_=info_man_->getInfo<Dominators>();
        funcClear();
        visited.clear();

        ::std::set<BasicBlock*>define_bbs{};
        ::std::set<BasicBlock*>use_bbs{};
        ::std::set<PhiInst*> phi_set{};
        calDefAndUse(func,define_bbs,use_bbs);
        generatePhi(define_bbs,phi_set);

        Value* incoming=nullptr;
        BasicBlock* entry_bb=nullptr;
        LoadInst* load=nullptr;
        Value*last_store=nullptr;
        if(func!=module_->getMainFunction()){
            entry_bb=func->getEntryBlock();
            load=LoadInst::createLoad(cur_global->getType()->getPointerElementType(),cur_global,entry_bb);
            entry_bb->getInstructions().pop_back();
            entry_bb->addInstrBegin(load);
            last_store=load;
        }else {
            incoming=cur_global->getInit();
            last_store=incoming;
        }
        reName(func->getEntryBlock(),nullptr,incoming,last_store);
        if(func!=module_->getMainFunction())
            if(load->useEmpty()){
                entry_bb->deleteInstr(load);
                delete load;
            }
    }
}
Modify G2L::run(){
    cur_dom_=info_man_->getInfo<Dominators>();
    for(auto cur_global:module_->getGlobalVariables()){
        if(cur_global->isConst()) continue;
        if(cur_global->getType()!=Type::getInt32PtrType()&&cur_global->getType()!=Type::getFloatPtrType())
            continue;
        this->cur_global=cur_global;
        clear();
        funcClear();
        visited.clear();
        if(cur_global->useEmpty())
            continue;
        calOneGlobal();
            
        if(!def_list_.empty()){
            runGlobal();
            continue;
        }else {
            std::vector<Use>v(cur_global->getUseList().begin(),cur_global->getUseList().end());
            for(auto u:v){
                if(auto load=dynamic_cast<LoadInst*>(u.val_)){
                    load->replaceAllUseWith(cur_global->getInit());
                    load->getParent()->deleteInstr(load);
                    delete load;
                }
            }
            continue;
        }
        clear();
        funcClear();
        visited.clear();
        calOneGlobal();
        runGlobal();
    }
    return {};
}