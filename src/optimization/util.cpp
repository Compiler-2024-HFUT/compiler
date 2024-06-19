#include "optimization/util.hpp"
#include "midend/Instruction.hpp"
// bool is_call_func(CallInst*call,Function*f,std::set<Function*>visited){
//     visited.insert(call->getParent()->getParent());
//     auto func=static_cast<Function*>(call->getOperand(0));
//     if(func==f) return true;
//     if(visited.count(func))
//         return false;
//     bool ret=false;
//     for(auto b:func->getBasicBlocks()){
//         for(auto ins:b->getInstructions()){
//             if(ins->isCall())
//                 ret=ret|is_call_func((CallInst*)ins,(Function*)(ins->getOperand(0)),visited);
//         }
//     }
//     return ret;
// }
void fixPhiOpUse(Instruction*phi){
    auto &oper=phi->getOperands();
    for(int i=0;i<oper.size();++i){
        auto value=phi->getOperand(i);
        value->removeUse(phi);
        value->addUse(phi,i);
    }
}
// bool is_call_by(Function*be_called,Function*call_func,std::set<Function*>visited){
//     for(auto u:be_called->getUseList())
//         if(auto call=dynamic_cast<CallInst*>(u.val_)){
//             if(call->getFunction()==call_func){
//                 return true;
//             }else{

//             }
//         }
// }