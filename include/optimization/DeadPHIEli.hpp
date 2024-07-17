#ifndef DEAD_PHI_ELI_HPP
#define DEAD_PHI_ELI_HPP


#include "midend/Instruction.hpp"
#include "PassManager.hpp"
class DeadPHIEli:public FunctionPass{
public:
    Modify runOnFunc(Function*func)override;
    DeadPHIEli(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~DeadPHIEli(){};
};

#endif