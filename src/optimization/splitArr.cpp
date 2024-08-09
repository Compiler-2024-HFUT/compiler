#include "analysis/Info.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "midend/Type.hpp"
#include "midend/Value.hpp"
#include "optimization/splitArr.hpp"
#include <cstddef>
#include <functional>
bool __ismemset(Instruction*ins){
    CallInst* _memset = dynamic_cast<CallInst*>(ins);
    if(_memset==0)
        return 0;
    return (_memset->getOperand(0)->getName()=="memset_i"||_memset->getOperand(0)->getName()=="memset_f");
}
bool SplitArr::canSpilt(Value *value) {
    for (auto user : value->getUseList()) {
        if (auto gep = dynamic_cast<GetElementPtrInst *>(user.val_)) {
            for (auto i = 1; i < gep->getNumOperands(); i++)
                if (!dynamic_cast<ConstantInt *>(gep->getOperand(i))) return false;
            if (!canSpilt(gep)) return false;
        }else if(auto call = dynamic_cast<CallInst *>(user.val_)) {
            return __ismemset(call);
        }
    }
    return true;
}

//请确保所有offset都为常量
size_t getoffset(GetElementPtrInst*gep){
    int offset=((ConstantInt*)(gep->getOperands().back()))->getValue();
    while(true){
        auto ptr=gep->getOperand(0);
        if(dynamic_cast<GetElementPtrInst*>(ptr)){
            gep=(GetElementPtrInst*)ptr;
            offset+=((ConstantInt*)(gep->getOperands().back()))->getValue();
        }else{
            break;
        }
    }
    return offset;
}
auto SplitArr::replaceArray(Value *array,  std::vector<AllocaInst*>& new_array)->void {
    std::function<void(Value*)> dfs = [&](Value* inst) -> void {
        if (auto gep = dynamic_cast<GetElementPtrInst*>(inst)) {
            for (auto &&user : gep->getUseList()) dfs(user.val_);
            auto idx = getoffset(gep);
            Value *new_value = new_array[idx];
            gep->replaceAllUseWith(new_value);
        } else if (CallInst* _memset = dynamic_cast<CallInst*>(inst);_memset&&__ismemset(_memset)) {
            auto bb = _memset->getParent();
            int base = 0;
            if(auto gep=dynamic_cast<GetElementPtrInst*>(array)){
                base=getoffset(gep);
            }         
            Constant*zero=(_memset->getOperand(2)->getType()->isIntegerType()?(Constant*)ConstantInt::get(0):(Constant*)ConstantFP::get(0));
            for (auto i = base; i < ((ConstantInt*)_memset->getOperand(2))->getValue()+base; i++) {
                auto new_st=StoreInst::createStore(zero,new_array[i],bb);
                bb->getInstructions().pop_back();
                bb->insertInstr(bb->findInstruction(_memset),new_st);
                erase.insert(_memset);
            }
        }
    };
    for (auto user : array->getUseList()) dfs(user.val_);
}

auto SplitArr::spiltArray(AllocaInst *_alloca)->bool {
    //if(_alloc is not arr) return false;
    if (!canSpilt(_alloca))
        return false;
    auto bb = _alloca->getParent();
    std::vector<AllocaInst *> new_allocas;
    auto base_type = _alloca->getType()->getPointerElementType()->getArrayElementType();
    new_allocas.resize(_alloca->getAllocaType()->getSize()/4);
    for (auto i = 0; i < _alloca->getAllocaType()->getSize()/4; i++) {
        auto new_alloca_ = AllocaInst::createAlloca(base_type,bb);
        new_allocas[i]=new_alloca_;
    }
    replaceArray(_alloca, new_allocas);
    for (auto &&new_alloca_ : new_allocas)
        bb->addInstrAfterPhiInst( new_alloca_);
    // bb->deleteInstr(_alloca);
    return true;
}
Modify SplitArr::runOnFunc(Function*func){
    if(func->isDeclaration())
        return {};
    Modify ret;
    auto entry=func->getEntryBlock();
    for(auto ins:entry->getInstructions()){
        if(ins->isAlloca()&&ins->getType()->getPointerElementType()->isArrayType())
            arr_set.push_back((AllocaInst*)ins);
    }
    for(auto arr:arr_set){
        // std::cout<<arr->getAllocaType()->getSize()/4<<std::endl;
        ret.modify_instr|=spiltArray(arr);
    }
    for(auto era:erase){
        era->getParent()->deleteInstr(era);
    }
    erase.clear();
    arr_set.clear();
    return ret;
}