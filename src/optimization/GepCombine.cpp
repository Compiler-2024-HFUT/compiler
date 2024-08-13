#include "analysis/Info.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include "optimization/GepCombine.hpp"
#include <cstdlib>
#include <utility>
#include <vector>
//after vn breakgep;
//ins ==> add的左子树 ptr==>为3操作数指针
GetElementPtrInst* __isBeGepUseEq(Instruction*ins,Value*ptr){
    for(auto [u,i]:ins->getUseList()){
        if(auto gep=dynamic_cast<GetElementPtrInst*>(u)){
            if(gep->getNumOperands()!=2)
                continue;
            if(gep->getOperand(0)==ptr)
                return gep;
        }
    }
    return 0;
}

bool GepCombine::adduseOneCombine(Function*func){
    std::vector<Instruction*>erase;
    for(auto b:func->getBasicBlocks()){
        auto &ins_list=b->getInstructions();
        for(auto _iter=ins_list.begin();_iter!=ins_list.end();){
            auto ins=*_iter;
            auto cur_iter=_iter++;
            if(ins->isGep()&&ins->getNumOperands()==2){
                if(auto add=dynamic_cast<BinaryInst*>(ins->getOperand(1));add&&add->isAdd()&&add->useOne()){
                    if(dynamic_cast<ConstantInt*>((add->getOperand(1)))){
                        erase.push_back(add);
                        auto inser_bb=add->getParent();
                        auto new_gep1=GetElementPtrInst::createGep(ins->getOperand(0),{add->getOperand(0)},inser_bb);
                        inser_bb->getInstructions().pop_back();
                        auto new_gep2=GetElementPtrInst::createGep(new_gep1,{add->getOperand(1)},b);
                        ins->replaceAllUseWith(new_gep2);
                        erase.push_back(ins);
                        b->getInstructions().pop_back();
                        ins_list.insert(cur_iter,new_gep2);
                        inser_bb->insertInstr(inser_bb->findInstruction(add),new_gep1);
                    }
                }
            }
        }

    }
    for(auto i:erase){
        i->removeUseOfOps();
    }
    for(auto i:erase){
        i->getParent()->deleteInstr(i);
        delete i;
    }
    if(erase.empty())
        return false;
    return true;
}
/*
    op1=add op0 op114514
    op2=gep ptr op1
    op3=add op1 2
    op4=gep ptr op3
    ====>
    op1=add op0 op114514
    op2=gep ptr op1
    op4=gep op2 2
*/
Modify GepCombine::runOnFunc(Function*func){
    if(func->isDeclaration())
        return{};
    Modify ret;
    ret.modify_instr=adduseOneCombine(func);
    for(auto b:func->getBasicBlocks()){
        auto ins_list=b->getInstructions();
        for(auto iter=ins_list.begin();iter!=ins_list.end();){
            auto cur_iter=iter;
            auto ins=*cur_iter;
            ++iter;
            if(ins->isGep()&&ins->getNumOperands()==2){
                if(ins->useEmpty()){
                    b->eraseInstr(cur_iter);
                    delete  ins;
                    continue;
                }
                work_set_.push_back((GetElementPtrInst*)ins);
            }
        }
    }
    while(!work_set_.empty()){
        auto gep=work_set_.back();
        work_set_.pop_back();
        Value* offset=gep->getOperands().back();
        if(auto add=dynamic_cast<Instruction*>(offset);add&&add->isAdd()){
            if(dynamic_cast<ConstantInt*>(add->getOperand(1))){
                if(auto lhs=dynamic_cast<Instruction*>(add->getOperand(0))){
                    auto gep_has_offset=__isBeGepUseEq(lhs,gep->getOperand(0));
                    if(gep_has_offset==0)
                        continue;
                    //必须在前面
                    if(!dom->isLBeforeR(gep_has_offset,gep))
                        continue;
                    auto bb=gep->getParent();
                    auto new_gep=GetElementPtrInst::createGep(gep_has_offset,{add->getOperand(1)},bb);
                    gep->replaceAllUseWith(new_gep);
                    bb->getInstructions().pop_back();
                    bb->insertInstr(bb->findInstruction(gep),new_gep);
                    bb->deleteInstr(gep);
                    delete gep;
                    if(dynamic_cast<ConstantInt*>(gep_has_offset->getOperand(1))){
                        work_set_.push_back(new_gep);
                    }
                }
                
            }
        }

    }
    ret.modify_instr|=immOverRange(func);
    return ret;
}
bool GepCombine::immOverRange(Function*func){
    bool ret=false;
    for(auto b:func->getBasicBlocks())
        ret|=immOverRangeOnBB(b,{});
    return ret;
}
bool GepCombine::immOverRangeOnBB(BasicBlock*bb,std::vector<std::pair<GetElementPtrInst*, int const>>gep_offset){
    bool ret=false;
    auto &inst_list=bb->getInstructions();
    for(auto __iter=inst_list.begin();__iter!=inst_list.end();){
        auto cur_iter=__iter;
        ++__iter;
        Instruction* inst=*cur_iter;
        if(inst->isGep()&&inst->getNumOperands()==2){
            auto offset_const=dynamic_cast<ConstantInt*>(inst->getOperands().back());
            if(offset_const==0)continue;
            int const  offset=offset_const->getValue();
            if(offset<=511&&offset>=-512)
                continue;
            bool _modify=false;
            for(auto [old_gep,old_offset]:gep_offset){
                if(old_gep->getOperand(0)==inst->getOperand(0)){
                    int const offset_gAp=offset-old_offset;
                    if(offset_gAp<=511&&offset_gAp>=-512){
                        auto new_gep=GetElementPtrInst::createGep(old_gep,{ConstantInt::get(offset-old_offset)},bb);
                        inst_list.pop_back();
                        inst->replaceAllUseWith(new_gep);
                        inst_list.insert(inst_list.erase(cur_iter),new_gep);
                        inst->removeUseOfOps();
                        delete inst;
                        _modify=true;
                        ret=true;
                    }
                }
            }
            if(_modify==false)
                gep_offset.push_back({(GetElementPtrInst*)inst,offset});
        }
    }
    return ret;

}