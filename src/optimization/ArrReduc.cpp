#include "optimization/ArrReduc.hpp"
#include "analysis/Info.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
Modify ArrReduc::runOnFunc(Function*func){
    if(func->isDeclaration())
        return {};
    visited.clear();
    auto ret= runOnBB(func->getEntryBlock(),{},{});
    visited.clear();
    return ret|runOnBBConst(func->getEntryBlock(),{},{});
}
void store_rm(Value*lval,std::map<GetElementPtrInst*,LoadInst*> &gep_load,std::map<GetElementPtrInst*,Value*>&gep_curval){
    while(dynamic_cast<GetElementPtrInst*>(lval)){
        lval=((GetElementPtrInst*)lval)->getOperand(0);
    }
    for (auto it = gep_load.begin();it != gep_load.end();) {
		auto gep_ptr=it->first->getOperand(0);
        while(dynamic_cast<GetElementPtrInst*>(gep_ptr)){
            gep_ptr=((GetElementPtrInst*)gep_ptr)->getOperand(0);
        }
        if(gep_ptr==lval)
			it = gep_load.erase(it);
        else
			++it;
    }
    for (auto it = gep_curval.begin();it != gep_curval.end();) {
		auto gep_ptr=it->first->getOperand(0);
        while(dynamic_cast<GetElementPtrInst*>(gep_ptr)){
            gep_ptr=((GetElementPtrInst*)gep_ptr)->getOperand(0);
        }
		if (gep_ptr==lval)
			it = gep_curval.erase(it);
		else
			++it;
    }

}
Modify ArrReduc::runOnBB(BasicBlock*bb,std::map<GetElementPtrInst*,LoadInst*> gep_load,std::map<GetElementPtrInst*,Value*>gep_curval){
    Modify ret;
    visited.insert(bb);
    auto &ins_list=bb->getInstructions();
    for(auto __iter=ins_list.begin();__iter!=ins_list.end();){
        auto cur_iter=__iter++;
        auto instr=*cur_iter;
        if(instr->isLoad()){
            auto load=(LoadInst*)instr;
            auto lval=load->getLVal();
            if(auto gep=dynamic_cast<GetElementPtrInst*>(lval)){
                auto gep_iter=gep_load.find(gep);
                auto gepval_iter=gep_curval.find(gep);
                if(gepval_iter!=gep_curval.end()){
                    load->replaceAllUseWith(gepval_iter->second);
                    load->removeUseOfOps();
                    __iter=ins_list.erase(cur_iter);
                    ret.modify_instr=true;
                    continue;
                }
                if(gep_iter!=gep_load.end()){
                    load->replaceAllUseWith(gep_iter->second);
                    load->removeUseOfOps();
                    __iter=ins_list.erase(cur_iter);
                    ret.modify_instr=true;
                }else{
                    gep_load.insert({gep,load});
                }
            }
        }else if(instr->isStore()){
            auto store=(StoreInst*)instr;
            auto lval=store->getLVal();
            if(auto gep=dynamic_cast<GetElementPtrInst*>(lval)){
                if(gep->getNumOperands()==3){
                    if(dynamic_cast<ConstantInt*>(gep->getOperand(2))){
                        store_rm(gep->getOperand(0),gep_load,gep_curval);
                        continue;
                    }
                }else{
                    if(dynamic_cast<ConstantInt*>(gep->getOperand(1))){
                        store_rm(gep->getOperand(0),gep_load,gep_curval);
                        continue;
                    }
                }
                auto gep_iter=gep_load.find(gep);
                if(gep_iter!=gep_load.end()){
                    gep_load.erase(gep_iter);
                }
                gep_curval.insert({gep,store->getRVal()});
            }
        }else if(instr->isCall()){
            if(auto iter=funca->all_se_info.find((Function*)instr->getOperand(0));iter!=funca->all_se_info.end()){
                if(iter->second.isWriteGlobal()||iter->second.isWriteParamArray()){
                    gep_load.clear();
                    gep_curval.clear();
                }
            }
        }
    }
    for(auto succ_bb:bb->getSuccBasicBlocks()){
        if(visited.count(succ_bb))
            continue;
        if(succ_bb->getPreBasicBlocks().size()==1&&succ_bb->getPreBasicBlocks().front()==bb){
            ret=ret|runOnBB(succ_bb,gep_load,gep_curval);
        }else{
            ret=ret|runOnBB(succ_bb,{},{});
        }
    }
    return ret;
}
Modify ArrReduc::runOnBBConst(BasicBlock*bb,std::map<GetElementPtrInst*,LoadInst*> gep_load,std::map<GetElementPtrInst*,Value*>gep_curval){
    Modify ret;
    visited.insert(bb);
    auto &ins_list=bb->getInstructions();
    for(auto __iter=ins_list.begin();__iter!=ins_list.end();){
        auto cur_iter=__iter++;
        auto instr=*cur_iter;
        if(instr->isLoad()){
            auto load=(LoadInst*)instr;
            auto lval=load->getLVal();
            if(auto gep=dynamic_cast<GetElementPtrInst*>(lval)){
                auto gep_iter=gep_load.find(gep);
                auto gepval_iter=gep_curval.find(gep);
                if(gepval_iter!=gep_curval.end()){
                    load->replaceAllUseWith(gepval_iter->second);
                    load->removeUseOfOps();
                    __iter=ins_list.erase(cur_iter);
                    ret.modify_instr=true;
                    continue;
                }
                if(gep_iter!=gep_load.end()){
                    load->replaceAllUseWith(gep_iter->second);
                    load->removeUseOfOps();
                    __iter=ins_list.erase(cur_iter);
                    ret.modify_instr=true;
                }else{
                    gep_load.insert({gep,load});
                }
            }
        }else if(instr->isStore()){
            auto store=(StoreInst*)instr;
            auto lval=store->getLVal();
            if(auto gep=dynamic_cast<GetElementPtrInst*>(lval)){
                if(gep->getNumOperands()==3){
                    if(dynamic_cast<ConstantInt*>(gep->getOperand(2))==0){
                        store_rm(gep->getOperand(0),gep_load,gep_curval);
                        continue;
                    }
                }else{
                    if(dynamic_cast<ConstantInt*>(gep->getOperand(1))==0){
                        store_rm(gep->getOperand(0),gep_load,gep_curval);
                        continue;
                    }

                }
                auto gep_iter=gep_load.find(gep);
                if(gep_iter!=gep_load.end()){
                    gep_load.erase(gep_iter);
                }
                gep_curval.insert({gep,store->getRVal()});
            }
        }else if(instr->isCall()){
            if(auto iter=funca->all_se_info.find((Function*)instr->getOperand(0));iter!=funca->all_se_info.end()){
                if(iter->second.isWriteGlobal()||iter->second.isWriteParamArray()){
                    gep_load.clear();
                    gep_curval.clear();
                }
            }
        }
    }
    for(auto succ_bb:bb->getSuccBasicBlocks()){
        if(visited.count(succ_bb))
            continue;
        if(succ_bb->getPreBasicBlocks().size()==1&&succ_bb->getPreBasicBlocks().front()==bb){
            ret=ret|runOnBBConst(succ_bb,gep_load,gep_curval);
        }else{
            ret=ret|runOnBBConst(succ_bb,{},{});
        }
    }
    return ret;
}