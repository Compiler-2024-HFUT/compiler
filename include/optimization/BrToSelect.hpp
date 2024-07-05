#ifndef BRTOSELECT_HPP
#define BRTOSELECT_HPP

#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
class BrToSelect:public FunctionPass{
    struct ToSelectInfo{
        enum selectType:int{
            EMPTY=0,
            IF_SEL,
            IF_ELSE_SEL,
        } type;
        CmpInst* cmp;
        // BranchInst* br;
        BasicBlock* true_bb,*false_bb,*intersection_bb;
        PhiInst* phi;
    } bb_info_;
    void canGenSel(BasicBlock *bb);

  public:
    virtual void runOnFunc(Function*func)override;
    // BrToSelect(Module *m, InfoManager *im)
    using FunctionPass::FunctionPass;
    ~BrToSelect(){};
};

#endif
