#ifndef VRE_HPP
#define VRE_HPP
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"

class VRE:public FunctionPass{
public:
    Modify runOnFunc(Function*func);
    using FunctionPass::FunctionPass;
    ~VRE(){};
};
class GenVR:public FunctionPass{
public:
    Modify runOnFunc(Function*func);
    using FunctionPass::FunctionPass;
    ~GenVR(){};
};
#endif
