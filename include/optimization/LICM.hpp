#ifndef LICM_HPP
#define LICM_HPP

#include "optimization/PassManager.hpp"
#include "analysis/LoopInvariant.hpp"
#include "analysis/LoopInfo.hpp"

class LICM : public FunctionPass {
    void visitLoop(Loop *loop);
public:
    LICM(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~LICM(){};

    void runOnFunc(Function* func) override;
};

#endif