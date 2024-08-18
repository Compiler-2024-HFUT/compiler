#ifndef LOOP_PARALLEL_HPP
#define LOOP_PARALLEL_HPP

#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/Dominators.hpp"
#include "optimization/PassManager.hpp"

#include "analysis/funcAnalyse.hpp"

struct LoopBodyInfo {
    BasicBlock* loop;
    PhiInst* indvar;
    Value* bound;
    PhiInst* rec;
    bool recUsedByOuter;
    bool recUsedByInner;
    Value* recInnerStep;
    CallInst* recNext;
    BasicBlock* exit;
};

class LoopParallel : public FunctionPass {
    bool LoopParallel::isNoSideEffectExpr(Instruction *inst);
    bool extractLoopBody(Function *func, Loop *loop, Module *mod, bool independent, bool allowInnermost,
                     bool allowInnerLoop, bool onlyAddRec, bool estimateBlockSizeForUnroll, bool needSubLoop,
                     bool convertReduceToAtomic, bool duplicateCmp, LoopBodyInfo *ret);
public:
    LoopParallel(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~LoopParallel(){};

    Modify runOnFunc(Function* func) override;

};


#endif