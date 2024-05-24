#ifndef DEAD_PHI_ELI_HPP
#define DEAD_PHI_ELI_HPP


#include "midend/Instruction.hpp"
#include "PassManager.hpp"
class DeadPHIEli:public FunctionPass{
public:
    void runOnFunc(Function*func)override;
    DeadPHIEli(Module *m) : FunctionPass(m){}
    ~DeadPHIEli(){};
};

#endif