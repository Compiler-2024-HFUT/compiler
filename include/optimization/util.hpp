#ifndef OPT_UTIL_HPP
#define OPT_UTIL_HPP
#include "midend/Instruction.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
// bool is_call_func(CallInst*call,Function*f,std::set<Function*>visited);
void fixPhiOpUse(Instruction*phi);
// void deleteBasicBlock(BasicBlock*bb);
void deleteIns(BasicBlock*bb,Instruction*ins);
void rmBBPhi(BasicBlock*valuefrom);
#endif