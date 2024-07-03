//该文件里写ASM的内存构建
#ifndef ASMGEN_HPP
#define ASMGEN_HPP

#include "midend/IRVisitor.hpp"

/*需要访问的中端数据结构如下：
module
function
basicblock
instruction（不包含instruction，使用的是instruction的子类）:
binaryinst
cmpinst
fcmpinst
callinst
branchinst
returninst
getelementptrinst
storeinst
memsetinst
loadinst
allocainst
zextinst
sitofpins
fptosiinst
phiinst
cmbrinst
fcmbrinst
loadoffsetinst
storeoffsetinst
*/

class AsmGen : public IRVisitor{
    public:
        AsmGen();
    
    private:
        virtual void visit(Module &node) override;
        virtual void visit(Function &node) override;
        virtual void visit(BasicBlock &node) override;
        virtual void visit(BinaryInst &node) override;
        virtual void visit(CmpInst &node) override;
        virtual void visit(FCmpInst &node) override;
        virtual void visit(CallInst &node) override;
        virtual void visit(BranchInst &node) override;
        virtual void visit(ReturnInst &node) override;
        virtual void visit(GetElementPtrInst &node) override;
        virtual void visit(StoreInst &node) override;
        virtual void visit(MemsetInst &node) override;
        virtual void visit(LoadInst &node) override;
        virtual void visit(AllocaInst &node) override;
        virtual void visit(ZextInst &node) override;
        virtual void visit(SiToFpInst &node) override;
        virtual void visit(FpToSiInst &node) override;
        virtual void visit(PhiInst &node) override;
        virtual void visit(CmpBrInst &node) override;
        virtual void visit(FCmpBrInst &node) override;
        virtual void visit(LoadOffsetInst &node) override;
        virtual void visit(StoreOffsetInst &node) override;



};



#endif