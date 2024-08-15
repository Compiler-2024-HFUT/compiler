#include "optimization/VirtualRetEli.hpp"
#include "analysis/Info.hpp"
#include "midend/Instruction.hpp"
#include "midend/Type.hpp"
#include "midend/Value.hpp"
#include "optimization/util.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include <cassert>
#include <utility>
#include <vector>
Modify VRE::runOnFunc(Function*func){
    if(func->getBasicBlocks().size()<2)
        return {};
    Modify mod;
    BasicBlock*ret_bb=func->getRetBlock();
    auto & ret_inss=ret_bb->getInstructions();
    if(ret_inss.size()!=2)
        return mod;
    auto phi=ret_inss.front();
    auto ret=ret_inss.back();
    if(func->getReturnType()==Type::getVoidType())
        return{};
    if(ret->getOperand(0)!=phi)
        return mod;
    
    std::vector<std::pair<BasicBlock*,Value*>>bb_val;
    for(int i=0;i<phi->getNumOperands();i+=2){
        bb_val.push_back({(BasicBlock*)phi->getOperand(i+1),phi->getOperand(i)});
    }
    //现在pre的size不一定等于bb_valsize;
    auto &phi_ops=phi->getOperands();

    for(int __i=0;__i<bb_val.size();__i++ ){
        auto [b,val]=bb_val[__i];

        auto &pre_inss=b->getInstructions();
        auto ter=pre_inss.back();
        if(ter->isBr()&&ter->getNumOperands()==1){
            assert(ter->getOperand(0)==ret_bb);
            mod.modify_bb=true;
            ter->removeUseOfOps();
            pre_inss.pop_back();
            delete ter;
            ret_bb->removePreBasicBlock(b);
            b->removeSuccBasicBlock(ret_bb);
            b->getSuccBasicBlocks().clear();
            ReturnInst::createRet(val,b);

            int offset=get_op_offset(phi_ops,val);
            phi->removeOperands(offset,offset+1);
            fixPhiOpUse(phi);
        }
    }
    if(ret_bb->getPreBasicBlocks().empty()){
        phi->removeUseOfOps();
        ret->removeUseOfOps();
        func->removeBasicBlock(ret_bb);
    }
    return mod;
}
Modify GenVR::runOnFunc(Function*func){
    Modify ret{};
    std::vector<std::pair<BasicBlock*, Instruction*>>bb_ret;
    for(auto b:func->getBasicBlocks()){
        if(b->getSuccBasicBlocks().size()==0&&b->getInstructions().back()->isRet()){
            bb_ret.push_back({b,b->getInstructions().back()});
        }
    }
    if(bb_ret.size()>1){
        ret.modify_bb=true;
        ret.modify_instr=true;
        auto ret_bb=BasicBlock::create("",func);
        for(auto [b,ret_ins]:bb_ret){
            auto &ins_list=b->getInstructions();
            ins_list.pop_back();
            BranchInst::createBr(ret_bb,b);
        }
        if(bb_ret.front().second->getNumOperands()==0){
            ReturnInst::createVoidRet(ret_bb);
        }else{
            auto new_phi=PhiInst::createPhi(bb_ret.front().second->getOperand(0)->getType(),ret_bb);
            ret_bb->addInstruction(new_phi);
            ReturnInst::createRet(new_phi,ret_bb);   
            for(auto [bb ,ret_val]:bb_ret){
                new_phi->addPhiPairOperand(ret_val->getOperand(0),bb);
                ret_val->removeUseOfOps();
                delete ret_val;
            }
        }
    }
    return ret;
}
