/*
    循环形式限制：
        a. 仅单条件（可求LoopCond、LoopTrip）
        b. 可求SCEV，也即要满足求解SCEV的循环格式
        c. 暂不考虑多重循环、多个归纳变量

    // **有合并多条i = i + 1的可能吗？**

    1. 循环次数可被整除
    i = 0;                                      i = 0;
    while(i < 100) {                            while(i < 100){
        n = n + i;              ->                  n = n + i;  ... n = n + i + 4;
        i = i + 1;                                  i = i + 1;  ... i = i + 1;      
    }                                           }
    2. 循环次数不可被整除 或 循环次数未知
    i = 0;                                      i = 0;
    while(i < 91) {                             while(i < 90(91/5*5)){
        n = n + i;                                  n = n + i;  ... n = n + i + 4;
        i = i + 1;               ->                 i = i + 1;  ... i = i + 1;
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

// Magic Num，后期考虑时间、空间局部性进行修改
#define UNROLLING_TIME          3       // 循环展开次数
#define DIRECT_UNROLLING_TIME   30
#define DIRECT_UNROLLING_SIZE   100     // 去除循环结构后的最大指令数

class LoopUnroll : public FunctionPass{
    void visitLoop(Loop *loop);
    void unrollCommonLoop(Loop *loop, LoopTrip trip);   // 情况1
    void unrollPartialLoop(Loop *loop, LoopTrip trip);  // 情况2
    void unrolEntirelLoop(Loop *loop, LoopTrip trip);   // 情况3 
    
    // ？？
    // 循环消除，对于迭代次数为0的循环，可以将其删除
    void removeLoop(Loop *loop);
public:
    LoopUnroll(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~LoopUnroll(){};

    void runOnFunc(Function* func) override;
};

#endif