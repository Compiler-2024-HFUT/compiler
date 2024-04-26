#ifndef DEAD_STORE_ELI_HPP
#define DEAD_STORE_ELI_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/IRGen.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include "analysis/Dominators.hpp"
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <vector>
using ::std::map,::std::set,::std::vector;
class DeadStoreEli : public FunctionPass{
private:
    Function * cur_func_;

    void rmStore();
    bool isAllocVar(Instruction *instr);
public:
    DeadStoreEli(Module *m) : FunctionPass(m){}
    ~DeadStoreEli(){};
    void run() override;
};

#endif