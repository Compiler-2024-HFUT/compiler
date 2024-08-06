#ifndef GEPOPT_HPP
#define GEPOPT_HPP

#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
class GepOpt:public Pass{
    ConstantInt*zero;
    Modify rmGep0(Function*func);
    Modify glbArrInitRm();

public:
    Modify run();
    GepOpt(Module*m,InfoManager*im):Pass(m, im){
        zero=ConstantInt::get(0);
    }
    ~GepOpt(){}
};
#endif