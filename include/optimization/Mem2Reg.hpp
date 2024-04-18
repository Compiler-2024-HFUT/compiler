#ifndef MEM2REG_HPP
#define MEM2REG_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/IRGen.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include "analysis/Dominators.hpp"
#include <list>
#include <memory>
#include <utility>
using ::std::map,::std::set,::std::vector;
class Mem2Reg : public FunctionPass{
private:
    ::std::_Rb_tree_iterator<std::pair<Function *const, std::unique_ptr<Dominators>>> cur_fun_dom;
    ::std::map<Function*, std::unique_ptr<Dominators>> func_dom_;
    ::std::map<Value *, std::vector<Value *>> var_val_stack;//全局变量初值提前存入栈中

    void removeSL();
    void removeOne(BasicBlock*cur_bb,map<Value *, Instruction *> define_list,map<Value *, Value *> new_value);
    void generatePhi();


    void reName(BasicBlock *bb);
    void removeAlloca();

	bool isLocalVarOp(Instruction *instr);
    bool isAllocVar(Instruction *instr);
    
public:
    Mem2Reg(Module *m) : FunctionPass(m){}
    ~Mem2Reg(){};
    void run() override;
};

#endif