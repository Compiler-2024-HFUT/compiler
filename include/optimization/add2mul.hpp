#ifndef ADD2MUL_HPP
#define ADD2MUL_HPP
#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"
class Add2Mul:public FunctionPass {
public:
    Modify runOnFunc(Function *func) override;
    using FunctionPass::FunctionPass;
    ~Add2Mul(){};
};
#endif
