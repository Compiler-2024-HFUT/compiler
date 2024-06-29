#ifndef DEAD_BB_ELI_HPP
#define DEAD_BB_ELI_HPP


#include "midend/Instruction.hpp"
#include "PassManager.hpp"
/*
b1:
    br b2
b2:     #pre:b1
    ......
br b3 b4

-------->

b1:
    br ,b3 b4
*/
class CombinBB:public FunctionPass{
public:
    void runOnFunc(Function*func)override;
    CombinBB(Module *m) : FunctionPass(m){}
    ~CombinBB(){};
};

#endif