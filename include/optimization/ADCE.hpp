#ifndef ADCE_HPP
#define ADCE_HPP

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
#include <unordered_set>
#include <utility>
#include <vector>
using ::std::map,::std::set,::std::vector;
class ADCE : public FunctionPass{
private:
    Function * cur_func_;
    ::std::unordered_set<Instruction*> alive_instr_;
    ::std::list<Instruction*> work_list_;
    ::std::set<BasicBlock*> alive_blocks_;

public:
    ADCE(Module *m) : FunctionPass(m){}
    ~ADCE(){};
    void run() override;
};

#endif