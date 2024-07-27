#ifndef INSTR_COMBINE_HPP
#define INSTR_COMBINE_HPP


#include "PassManager.hpp"
#include "analysis/Info.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include <vector>
using ::std::vector;
class InstrCombine : public FunctionPass{
private:
    Function * cur_func_;
    ::std::vector<Instruction*> work_set_;
    // ::std::map<Instruction*,std::list<Instruction*>::iterator> to_erase_;
    Instruction* combine(Instruction*instr);
    Instruction* combineAdd(Instruction*instr);
    Instruction* combineMul(Instruction*instr);
    Instruction* combineFAdd(Instruction*instr);
    Instruction* combineFMul(Instruction*instr);
    Instruction* combineSub(Instruction*instr);
    Instruction* combineDiv(Instruction*instr);
    Instruction* combineShl(Instruction*instr);
    Instruction* combineAsr(Instruction*instr);
    Instruction* combineOr(Instruction*instr);
    Instruction* combineXor(Instruction*instr);
    Instruction* combineAnd(Instruction*instr);
    Instruction* combineCmp(Instruction* instr);
    Instruction* combineICmp(CmpInst* instr);
    Instruction* combineFCmp(FCmpInst* instr);
    Instruction* combineConst(BinaryInst* instr);
    // Instruction* _simplify_bin(BinaryInst*bin_ins);

    Instruction* replaceInstUsesWith(Instruction*,Value*);
    void removeInsWithWorkset(Instruction*ins);
    void  preProcess(Function*cur_func);
    
    // using visitfn = std::function<Instruction*(Instruction*)>;
    
    const ::std::map<Instruction::OpID,std::function<Instruction*(Instruction*)>> combine_map_;
public:
    InstrCombine(Module *m,InfoManager*im);
    ~InstrCombine(){};
    Modify runOnFunc(Function*func) override;

};

class InstrReduc : public FunctionPass{
private:
    Function * cur_func_;
    ::std::vector<Instruction*> work_set_;
    Instruction* reduc(Instruction*instr);
    Instruction* reducAdd(Instruction*instr);
    Instruction* reducMul(Instruction*instr);
    Instruction* reducFAdd(Instruction*instr);
    Instruction* reducFMul(Instruction*instr);
    Instruction* reducSub(Instruction*instr);
    Instruction* reducDiv(Instruction*instr);
    Instruction* reducShl(Instruction*instr);
    Instruction* reducAsr(Instruction*instr);
    Instruction* reducOr(Instruction*instr);
    Instruction* reducXor(Instruction*instr);
    Instruction* reducAnd(Instruction*instr);

    // using visitfn = std::function<Instruction*(Instruction*)>;
    
    const ::std::map<Instruction::OpID,std::function<Instruction*(Instruction*)>> reduc_map_;
public:
    InstrReduc(Module *m,InfoManager*im);
    ~InstrReduc(){};
    Modify runOnFunc(Function*func) override;

};

#endif