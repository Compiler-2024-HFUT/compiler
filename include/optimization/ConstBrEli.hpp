#ifndef CONSTBR_HPP
#define CONSTBR_HPP
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"
class ConstBr:public FunctionPass{
public:
    static bool canFold(BasicBlock*bb);
    void runOnFunc(Function*func);
    ConstBr(Module*m);
};
#endif
