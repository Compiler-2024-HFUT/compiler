#ifndef IR_VISITOR_HPP
#define IR_VISITOR_HPP

#include "Module.hpp"
#include "Function.hpp"
#include "Instruction.hpp"

class IRVisitor{
    public:
        virtual void visit(Module &node) = 0;
        virtual void visit(Function &node) = 0;
        virtual void visit(BasicBlock &node) = 0;
        virtual void visit(BinaryInst &node) = 0;
        virtual void visit(CmpInst &node) = 0;
        virtual void visit(FCmpInst &node) = 0;
        virtual void visit(CallInst &node) = 0;
        virtual void visit(BranchInst &node) = 0;
        virtual void visit(ReturnInst &node) = 0;
        virtual void visit(GetElementPtrInst &node) = 0;
        virtual void visit(StoreInst &node) = 0;
        virtual void visit(MemsetInst &node) = 0;
        virtual void visit(LoadInst &node) = 0;
        virtual void visit(AllocaInst &node) = 0;
        virtual void visit(ZextInst &node) = 0;
        virtual void visit(SiToFpInst &node) = 0;
        virtual void visit(FpToSiInst &node) = 0;
        virtual void visit(PhiInst &node) = 0;
        virtual void visit(CmpBrInst &node) = 0;
        virtual void visit(FCmpBrInst &node) = 0;
        virtual void visit(LoadOffsetInst &node) = 0;
        virtual void visit(StoreOffsetInst &node) = 0;
        virtual void visit(LoadImmInst &node) = 0;
        virtual void visit(CastInst &node) = 0;

};






#endif