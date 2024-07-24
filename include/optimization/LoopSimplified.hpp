#ifndef LOOP_SIMPLIFIED_HPP
#define LOOP_SIMPLIFIED_HPP

#include "optimization/PassManager.hpp"
#include "analysis/LoopInfo.hpp"

#include <vector>
#include <list>
using std::vector;
using std::list;

class LoopSimplified : public FunctionPass{

    void processLoop(Loop *loop);
    BB *insertPreheader(Loop *loop);
    BB *splitExit(Loop *loop, BB *exit);
    BB *insertUniqueBackedge(Loop *loop);
    BB *mergeExits(Loop *loop);
    void moveInvariant(Loop *loop, BB *from, BB *to);
public:
    LoopSimplified(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~LoopSimplified(){};

    void runOnFunc(Function* func) override;
};

#endif