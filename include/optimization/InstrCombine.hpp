#ifndef INSTR_COMBINE_HPP
#define INSTR_COMBINE_HPP


#include "PassManager.hpp"
#include "analysis/Info.hpp"
#include "midend/Function.hpp"
#include <list>
#include <map>
#include <set>
using ::std::map,::std::set,::std::vector;
class InstrCombine : public FunctionPass{
private:
    Function * cur_func_;
    ::std::list<Instruction*> work_set_;
    Instruction* combine(Instruction*instr);
    Instruction* combineAdd(Instruction*instr);
    Instruction* combineMul(Instruction*instr);
    Instruction* combineDiv(Instruction*instr);
    Instruction* combineConst(BinaryInst* instr);
    
    Instruction* replaceInstUsesWith(Instruction*,Value*);
    
    void  preProcess(Function*cur_func);
    
    // using visitfn = std::function<Instruction*(Instruction*)>;
    
    const ::std::map<Instruction::OpID,std::function<Instruction*(Instruction*)>> combine_map_;
public:
    InstrCombine(Module *m,InfoManager*im);
    ~InstrCombine(){};
    Modify runOnFunc(Function*func) override;

};

#endif