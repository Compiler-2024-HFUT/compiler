/*
    1. 循环次数可被整除
    i = 0;                                      i = 0;
    while(i < 100) {                            while(i < 100){
        n = n + i;              ->                  n = n + i;  ... n = n + i + 4;
        i = i + 1;                                  i = i + 5;
    }                                           }
    2. 循环次数不可被整除 或 循环次数未知
    i = 0;                                      i = 0;
    while(i < 91) {                             while(i < 90(91/5*5)){
        n = n + i;                                  n = n + i;  ... n = n + i + 4;
        i = i + 1;               ->                 i = i + 5;
    }                                           }
                                                while(i < 91){
                                                    n = n + i;
                                                    i = i + 1;
                                                }
    3. 循环次数较少，直接改为顺序结构
    i = 0;                                      i = 0;
    while(i < 5) {                              n = n + i;
        n = n + i;              ->              n = n + i + 1;
        i = i + 1;                                ... 
    }                                           n = n + i + 4;
 */

#ifndef LOOP_UNROLL_HPP
#define LOOP_UNROLL_HPP

#include "optimization/PassManager.hpp"
#include "analysis/LoopInfo.hpp"

#include <vector>
using std::vector;

class LoopUnroll : public FunctionPass{
    int time;                       // 循环展开次数

    void visitLoop(Loop *loop);
    void unrollCommonLoop(Loop *loop, LoopTrip trip);   // 情况1
    void unrollPartialLoop(Loop *loop);  // 情况2
    void unrolEntirelLoop(Loop *loop);   // 情况3 
    void removeLoop(Loop *loop);
public:
    LoopUnroll(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~LoopUnroll(){};

    void runOnFunc(Function* func) override;
};

#endif