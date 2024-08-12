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
