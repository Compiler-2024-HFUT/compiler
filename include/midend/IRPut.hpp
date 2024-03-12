#ifndef IRBUILDER_HPP
#define IRBUILDER_HPP

#include "BasicBlock.hpp"
#include "Instruction.hpp"
#include "Value.hpp"


class IRBuilder {
  public:
    IRBuilder(BasicBlock *bb, Module *m) : bb(bb), module(m) {}


    Module *getModule() const { return module; }

    //返回插入指令的BB
    BasicBlock *getInsertBB() const { return this->bb; }

    //设置插入指令的BB
    void setInsertBB(BasicBlock *bb) { this->bb = bb; } //& insert instructions into the basic block".

    BinaryInst *putIAdd( Value *lhs, Value *rhs){ return BinaryInst::makeAdd( lhs, rhs, this->bb, module);}  
    BinaryInst *putISub( Value *lhs, Value *rhs){ return BinaryInst::makeSub( lhs, rhs, this->bb, module);}
    BinaryInst *putIMul( Value *lhs, Value *rhs){ return BinaryInst::makeMul( lhs, rhs, this->bb, module);}
    BinaryInst *putIMul64( Value *lhs, Value *rhs){ return BinaryInst::makeMul64( lhs, rhs, this->bb, module);}
    BinaryInst *putISDiv( Value *lhs, Value *rhs){ return BinaryInst::makeSDiv( lhs, rhs, this->bb, module);}
    BinaryInst *putISRem( Value *lhs, Value *rhs){ return BinaryInst::makeSRem( lhs, rhs, this->bb, module);}

    BinaryInst *putIAnd( Value *lhs, Value *rhs){ return BinaryInst::makeAnd( lhs, rhs, this->bb, module);}
    BinaryInst *putIOr( Value *lhs, Value *rhs){ return BinaryInst::makeOr( lhs, rhs, this->bb, module);}

    BinaryInst *putFAdd(Value *lhs, Value *rhs) { return BinaryInst::makeFAdd(lhs, rhs, this->bb, module); }
    BinaryInst *putFSub(Value *lhs, Value *rhs) { return BinaryInst::makeFSub(lhs, rhs, this->bb, module); }
    BinaryInst *putFMul(Value *lhs, Value *rhs) { return BinaryInst::makeFMul(lhs, rhs, this->bb, module); }
    BinaryInst *putFDiv(Value *lhs, Value *rhs) { return BinaryInst::makeFDiv(lhs, rhs, this->bb, module); }


    CmpInst *putICmpEq( Value *lhs, Value *rhs){ return CmpInst::makeCmp(EQ, lhs, rhs, this->bb, module); }
    CmpInst *putICmpNe( Value *lhs, Value *rhs){ return CmpInst::makeCmp(NE, lhs, rhs, this->bb, module); }
    CmpInst *putICmpGt( Value *lhs, Value *rhs){ return CmpInst::makeCmp(GT, lhs, rhs, this->bb, module); }
    CmpInst *putICmpGe( Value *lhs, Value *rhs){ return CmpInst::makeCmp(GE, lhs, rhs, this->bb, module); }
    CmpInst *putICmpLt( Value *lhs, Value *rhs){ return CmpInst::makeCmp(LT, lhs, rhs, this->bb, module); }
    CmpInst *putICmpLe( Value *lhs, Value *rhs){ return CmpInst::makeCmp(LE, lhs, rhs, this->bb, module); }

    FCmpInst *putFCmpNe(Value *lhs, Value *rhs) { return FCmpInst::makeFCmp(NE, lhs, rhs, this->bb, module);}
    FCmpInst *putFCmpLt(Value *lhs, Value *rhs) { return FCmpInst::makeFCmp(LT, lhs, rhs, this->bb, module);}
    FCmpInst *putFCmpLe(Value *lhs, Value *rhs) { return FCmpInst::makeFCmp(LE, lhs, rhs, this->bb, module);}
    FCmpInst *putFCmpGe(Value *lhs, Value *rhs) { return FCmpInst::makeFCmp(GE, lhs, rhs, this->bb, module);}
    FCmpInst *putFCmpGt(Value *lhs, Value *rhs) { return FCmpInst::makeFCmp(GT, lhs, rhs, this->bb, module);}
    FCmpInst *putFCmpEq(Value *lhs, Value *rhs) { return FCmpInst::makeFCmp(EQ, lhs, rhs, this->bb, module);}

    BinaryInst *putAsr(Value *lhs, Value *rhs) { return BinaryInst::makeAsr(lhs, rhs, this->bb, module);}
    BinaryInst *putLsl(Value *lhs, Value *rhs) { return BinaryInst::makeLsl(lhs, rhs, this->bb, module);}
    BinaryInst *putLsr(Value *lhs, Value *rhs) { return BinaryInst::makeLsr(lhs, rhs, this->bb, module);}
    BinaryInst *putAsr64(Value *lhs, Value *rhs) { return BinaryInst::makeAsr64(lhs, rhs, this->bb, module);}
    BinaryInst *putLsl64(Value *lhs, Value *rhs) { return BinaryInst::makeLsl64(lhs, rhs, this->bb, module);}
    BinaryInst *putLsr64(Value *lhs, Value *rhs) { return BinaryInst::makeLsr64(lhs, rhs, this->bb, module);}

    CallInst *putCall(Value *func, std::vector<Value *> args) {
        return CallInst::makeCall(static_cast<Function *>(func) ,args, this->bb); 
    }

    BranchInst *putBr(BasicBlock *if_true){ return BranchInst::makeBr(if_true, this->bb); }
    BranchInst *putCondBr(Value *cond, BasicBlock *if_true, BasicBlock *if_false){ return BranchInst::makeCondBr(cond, if_true, if_false,this->bb); }

    ReturnInst *putRet(Value *val) { return ReturnInst::makeRet(val,this->bb); }
    ReturnInst *putVoidRet() { return ReturnInst::makeVoidRet(this->bb); }

    GetElementPtrInst *putGep(Value *ptr, std::vector<Value *> idxs) { return GetElementPtrInst::makeGep(ptr, idxs, this->bb); }

    StoreInst *putStore(Value *val, Value *ptr) { return StoreInst::makeStore(val, ptr, this->bb ); }
    LoadInst * putLoad(Type *ty, Value *ptr) { return LoadInst::makeLoad(ty, ptr, this->bb); }
    LoadInst * putLoad(Value *ptr) {
        return LoadInst::makeLoad(ptr->getType()->getPtrElementType(), ptr, this->bb); 
    }
    MemsetInst *putMemset(Value *ptr) { return MemsetInst::makeMemset(ptr, this->bb ); }

    AllocaInst *putAlloca(Type *ty) { return AllocaInst::makeAlloca(ty, this->bb); }

    ZextInst *putZext(Value *val, Type *ty) { return ZextInst::makeZext(val, ty, this->bb); }

    SiToFpInst *putSitofp(Value *val, Type *ty) { return SiToFpInst::makeSitofp(val, ty, this->bb); }
    FpToSiInst *putFptosi(Value *val, Type *ty) { return FpToSiInst::makeFptosi(val, ty, this->bb); }
  private:
    BasicBlock *bb;
    Module *module;
};
#endif