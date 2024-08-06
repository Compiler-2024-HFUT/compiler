#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "midend/Value.hpp"
#include "optimization/GepOpt.hpp"
#include <cassert>
#include <list>
#include <map>
#include <sys/cdefs.h>
#include <vector>
// Modify GepOpt::glbArrInitRm(){
//     ::std::vector<GlobalVariable*>glbarr;
//     std::vector<Value*> uselist;
//     std::list<GetElementPtrInst*> geplist;
//     for(auto glb:module_->getGlobalVariables()){
//         auto type=glb->getType();
//         if(!type->getPointerElementType()->isArrayType())
//             continue;
//         ConstantArray*init=dynamic_cast<ConstantArray*>(glb->getInit());
//         if(init==0)
//             continue;
//         for(auto u:glb->getUseList()){
//             uselist.push_back(u.val_);
//         }
//         bool other_op=false;

//         while(!uselist.empty()){
//             auto use=uselist.back();
//             uselist.pop_back();
//             if(auto gep=dynamic_cast<GetElementPtrInst*>(use)){
//                 if(gep->getNumOperands()==3){
//                     if(auto off=dynamic_cast<ConstantInt*>(gep->getOperand(2));off==0){
//                         other_op=true;
//                         break;
//                     }
//                 }
//                 else if(gep->getNumOperands()==2){
//                     if(auto off=dynamic_cast<ConstantInt*>(gep->getOperand(1));off==0){
//                         other_op=true;
//                         break;
//                     }
//                 }
//                 for(auto u:use->getUseList()){
//                     uselist.push_back(u.val_);
//                 }
//             }else if(dynamic_cast<CallInst*>(use)||dynamic_cast<StoreInst*>(use)){
//                 other_op=true;
//                 break;
//             }
//         }

//         if(other_op)
//             continue;

//         for(auto u:glb->getUseList()){
//             if(auto gep=dynamic_cast<GetElementPtrInst*>(u.val_)){
//                 geplist.push_back(gep);
//             }else{
//                 uselist.push_back(u.val_);
//             }
//         }
//         std::map<Value*,int>offset{{glb,0}};
//         while(!geplist.empty()){
//             auto gep=geplist.front();
//             geplist.pop_front();
//             for(auto u:gep->getUseList()){
//                 if(auto gep_gep=dynamic_cast<GetElementPtrInst*>(u.val_)){
//                     geplist.push_back(gep_gep);
//                 }else{
//                     uselist.push_back(u.val_);
//                 }
//             }
//             if(gep->getNumOperands()==3){
//                 if(auto off=dynamic_cast<ConstantInt*>(gep->getOperand(2))){
//                     offset.insert({gep,off->getValue()+offset.find(gep->getOperand(0))->second});
//                 }
//             }
//             else if(gep->getNumOperands()==2){
//                 if(auto off=dynamic_cast<ConstantInt*>(gep->getOperand(1))){
//                     offset.insert({gep,off->getValue()+offset.find(gep->getOperand(0))->second});
//                 }
//             }else{assert(0);}

//         }
//         while(!uselist.empty()){
//             auto use=uselist.back();
//             uselist.pop_back();
//             if(dynamic_cast<GetElementPtrInst*>(use)){
//                 for(auto u:use->getUseList()){
//                     uselist.push_back(u.val_);
//                 }
//             }else if(auto load=dynamic_cast<LoadInst*>(use)){
//                 load->replaceAllUseWith(init->getElementValue(offset.find(load->getLVal())->second));
//                 delete load;
//             }
//         }
//     }
//     return{};
// }
Modify GepOpt::rmGep0(Function*func){
    Modify ret;
    for(auto b:func->getBasicBlocks()){
        auto &inss=b->getInstructions();
        for(auto iter=inss.begin();iter!=inss.end();){
            auto cur_it=iter++;
            auto ins=*cur_it;
            if(ins->isGep()&&ins->getNumOperands()==2&&ins->getOperand(1)==zero){
                ins->replaceAllUseWith(ins->getOperand(0));
                ins->removeUseOfOps();
                ret.modify_instr=true;
                iter=inss.erase(cur_it);
                delete ins;
            }
        }
    }
    return ret;
}
Modify GepOpt::run(){
    Modify ret;
    for(auto f:module_->getFunctions()){
        if(!f->isDeclaration())
            ret=ret|rmGep0(f);
    }
    // glbArrInitRm();
    return ret;
}