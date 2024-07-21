#ifndef CONSTBR_HPP
#define CONSTBR_HPP
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"
class ConstBr:public FunctionPass{

    ConstantInt* const_true;
    ConstantInt* const_false;
    std::set<BasicBlock*>erased;

    void eraseBB(BasicBlock*bb);
    bool constCondFold(BasicBlock*bb);
public:
    static bool canFold(BasicBlock*bb);
    virtual Modify runOnFunc(Function*func);
    ConstBr(Module*m, InfoManager *im);
};
#endif
