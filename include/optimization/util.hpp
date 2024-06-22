#include "midend/Instruction.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include <set>
// bool is_call_func(CallInst*call,Function*f,std::set<Function*>visited);
void fixPhiOpUse(Instruction*phi);
void deleteBasicBlock(BasicBlock*bb);