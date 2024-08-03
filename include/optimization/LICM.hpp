/*
    循环不变量外移
    https://www.cs.cornell.edu/courses/cs6120/2019fa/blog/loop-reduction/
    https://www.cs.toronto.edu/~pekhimenko/courses/cscd70-w18/docs/Lecture%205%20%5BLICM%20and%20Strength%20Reduction%5D%2002.08.2018.pdf
*/

#ifndef LICM_HPP
#define LICM_HPP

#include "optimization/PassManager.hpp"
#include "analysis/Dataflow.hpp"
#include "analysis/Dominators.hpp"
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