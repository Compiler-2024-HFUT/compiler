#ifndef INSTR_RESOLVE_HPP
#define INSTR_RESOLVE_HPP


#include "PassManager.hpp"
#include "analysis/Info.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include <vector>
using ::std::vector;
class InstrResolve : public FunctionPass{
private:
    std::vector<Instruction*> resolveAdd(Instruction*instr);
    std::vector<Instruction*> resolveRAdd(Instruction*instr);
public:
    using FunctionPass::FunctionPass;
    ~InstrResolve(){};
    Modify runOnFunc(Function*func) override;

};
#endif