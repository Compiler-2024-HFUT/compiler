#ifndef SPLIT_ARR_HPP
#define SPLIT_ARR_HPP
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "midend/Value.hpp"
#include "optimization/PassManager.hpp"
#include <set>
#include <vector>
class SplitArr:public FunctionPass{
    std::vector<AllocaInst*> arr_set;
    std::set<Instruction*>erase;
public:
    void replaceArray(Value *array,  std::vector<AllocaInst*>& new_array);
    Modify runOnFunc(Function*func);
    bool canSpilt(Value*);
    bool spiltArray(AllocaInst*alloc);
    using FunctionPass::FunctionPass;
    ~SplitArr(){};
};
#endif
