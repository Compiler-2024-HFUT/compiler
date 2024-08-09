#include "optimization/DFE.hpp"
#include "analysis/Info.hpp"
#include "analysis/funcAnalyse.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
Modify DFE::runOnModule(Module*module_){
    Modify ret{};
    funanaly=info_man_->getInfo<FuncAnalyse>();
    for(auto fun:module_->getFunctions()){
        if(fun->isDeclaration())
            continue;
        auto se=funanaly->all_se_info.find(fun)->second;
        if(!se.isWriteGlobal()&&!se.isWriteParamArray()&&!se.isPutVar()&&!se.isGetVar()){
            auto ul=fun->getUseList();
            for(auto [u,i]:ul){
                if(!u->useEmpty())
                    continue;
                auto call=dynamic_cast<CallInst*>(u);
                if(call){
                    call->getParent()->deleteInstr(call);
                    delete call;
                    ret.modify_instr=true;
                    ret.modify_call=true;
                }
            }
        }
    }
    return ret;
}
