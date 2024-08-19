#include "midend/IRVisitor.hpp"
#include "midend/BasicBlock.hpp"

void Module::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void Function::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void BasicBlock::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void BinaryInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void CmpInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void FCmpInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void CallInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void BranchInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void ReturnInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void GetElementPtrInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void StoreInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void MemsetInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void LoadInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void AllocaInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void ZextInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void SiToFpInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void PhiInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void FpToSiInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void CmpBrInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void FCmpBrInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void LoadOffsetInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void StoreOffsetInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void LoadImmInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void CastInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}

void AtomicAddInst::accept(IRVisitor &visitor){
    visitor.visit(*this);
}