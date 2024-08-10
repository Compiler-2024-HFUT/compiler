/*
    After LICM
*/

#ifndef LOOP_STRENGTH_REDUCTION_HPP
#define LOOP_STRENGTH_REDUCTION_HPP

#include "optimization/PassManager.hpp"
#include "analysis/LoopInvariant.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/SCEV.hpp"

class LoopStrengthReduction : public FunctionPass {
    void visitLoop(Loop *loop);
public:
    LoopStrengthReduction(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~LoopStrengthReduction(){};

    Modify runOnFunc(Function* func) override;
};

#endif