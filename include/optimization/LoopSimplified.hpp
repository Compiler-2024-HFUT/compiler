/*
    After DeadPhiEli

    循环简化实现的功能：
    // In LLVM: https://llvm.org/docs/LoopTerminology.html#loop-terminology-loop-simplify
    1. 插入preheader，使得header只有一个preBB，用于LICM
    2. 只有一条BackEdge/Latch，加上preheader，保证循环header的preBB只有2个
    3. 循环的exits的preBB都不在循环之外，也即header支配所有exit
    // 出于需要自行添加的
    4. 对于多重循环存在的多个IndVar，保证其IndVar只出现在其header内，而不跑到其外循环的header内
*/

#ifndef LOOP_SIMPLIFIED_HPP
#define LOOP_SIMPLIFIED_HPP

#include "optimization/PassManager.hpp"
#include "analysis/LoopInfo.hpp"

#include <vector>
#include <list>
using std::vector;
using std::list;

class LoopSimplified : public FunctionPass{

    void visitLoop(Loop *loop);
    void processLoop(Loop *loop);
    BB *insertPreheader(Loop *loop);
    BB *splitExit(Loop *loop, BB *exit);
    BB *insertUniqueBackedge(Loop *loop);
    BB *mergeExits(Loop *loop);
    // void findAndMoveIndVar(Loop *loop);
public:
    LoopSimplified(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~LoopSimplified(){};

    void runOnFunc(Function* func) override;
};

#endif