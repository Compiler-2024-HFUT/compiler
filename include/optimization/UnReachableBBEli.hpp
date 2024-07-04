#ifndef UNREACH_BBELI_HPP
#define UNREACH_BBELI_HPP
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"
class UnReachableBBEli:public FunctionPass{
    std::set<BasicBlock*>erased;
public:
    void eraseBB(BasicBlock*bb);
    void runOnFunc(Function*func);
    using FunctionPass::FunctionPass;
};
#endif
