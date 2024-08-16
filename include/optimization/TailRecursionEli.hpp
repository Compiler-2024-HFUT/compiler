#ifndef TAIL_RECURSIONELI_HPP
#define TAIL_RECURSIONELI_HPP

#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "PassManager.hpp"


class TailRecursionElim : public FunctionPass {
public:
    TailRecursionElim(Module *m,InfoManager*im) : FunctionPass(m, im) {}
    Modify runOnFunc(Function*func) final;
    ~TailRecursionElim(){}
private:
};
#endif