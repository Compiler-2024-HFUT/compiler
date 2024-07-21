#ifndef ADCE_HPP
#define ADCE_HPP


#include "analysis/Info.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include <list>
#include <set>
#include <unordered_set>
#include <vector>
using ::std::map,::std::set,::std::vector;
class ADCE : public FunctionPass{
private:
    Function * cur_func_;
    ::std::unordered_set<Instruction*> alive_instr_;
    ::std::list<Instruction*> work_list_;
    ::std::set<BasicBlock*> alive_blocks_;

public:
    ADCE(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~ADCE(){};
    Modify runOnFunc(Function*func) override;
};

#endif