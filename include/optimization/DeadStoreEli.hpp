#ifndef DEAD_STORE_ELI_HPP
#define DEAD_STORE_ELI_HPP


#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include <set>
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