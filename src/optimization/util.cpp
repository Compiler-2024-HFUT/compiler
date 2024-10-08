#include "optimization/util.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include <cassert>
#include <vector>
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
//将alloc提前

void fixPhiOpUse(Instruction*phi){
    auto &oper=phi->getOperands();
    phi->removeUseOfOps();
    for(int i=0;i<oper.size();++i){
        auto value=phi->getOperand(i);
        value->addUse(phi,i);
    }
}
void rmBBPhi(BasicBlock*valuefrom){
    auto _uselist=valuefrom->getUseList();
    for(auto [v,i ]:_uselist){
        if(auto phi=dynamic_cast<PhiInst*>(v)){
            phi->removeOperands(i-1,i);
            fixPhiOpUse(phi);
            if(phi->getNumOperands()==2&&valuefrom->getPreBasicBlocks().size()<2){
                phi->replaceAllUseWith(phi->getOperand(0));
                phi->getParent()->deleteInstr(phi);
                delete phi;
            }
        }
    }
}
// void deleteBasicBlock(BasicBlock*bb){
//     ::std::list<Instruction*> &instrs=bb->getInstructions();
//     for(auto i:instrs){
//         i->removeUseOfOps();
//     }
//     while(!instrs.empty()){
//         auto iter=instrs.begin();
//         auto instr=*iter;
//         assert(instr->getUseList().empty()&&"removed basicblock has cannot remove instruction");
//         instrs.pop_front();
//         delete instr;
//     }
//     delete  bb;
// }
void deleteIns(BasicBlock*bb,Instruction*ins){
    bb->deleteInstr(ins);
    delete  ins;
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
int get_op_offset(std::vector<Value*>&ops, Value*v){
    auto size=ops.size();
    for(int i=0;i<size;++i){
        if(v==ops[i]){
            return i;
        }
    }
    assert(0);
    return -1;
}
