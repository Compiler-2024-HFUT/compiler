#ifndef COMBININSTRCONST_HPP
#define COMBININSTRCONST_HPP 
#include "midend/Instruction.hpp"
#include "optimization/PassManager.hpp"
#include <vector>
class CombinInstrConst:public FunctionPass{
    Modify runOnFunc(Function*func);
    void preProcess(Function*func);
    bool combinMullAdd(BinaryInst*instr);
    bool combinConstMulAdd(BinaryInst*instr);
    std::vector<Value*>work_set_;
public:
    using FunctionPass::FunctionPass;
};
#endif