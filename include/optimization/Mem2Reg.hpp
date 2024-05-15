#ifndef MEM2REG_HPP
#define MEM2REG_HPP

#include "PassManager.hpp"
#include "analysis/Dominators.hpp"
#include <list>
#include <memory>
#include <set>
#include <utility>
using ::std::map,::std::set,::std::vector;
class Mem2Reg : public FunctionPass{
private:
    ::std::_Rb_tree_iterator<std::pair<Function *const, std::unique_ptr<Dominators>>> cur_fun_dom;
    ::std::map<Function*, ::std::unique_ptr<Dominators>> func_dom_;

    ::std::map<BasicBlock*, ::std::map<AllocaInst*,PhiInst*>> new_phi;
    ::std::list<AllocaInst*>allocas;


    void reName(BasicBlock *bb);
    void removeAlloca();
    BasicBlock* isOnlyInOneBB(AllocaInst*ai);
    void calDefAndUse(AllocaInst*ai,::std::set<BasicBlock*>&def,::std::set<BasicBlock*>&use);
    void rmLocallyAlloc(AllocaInst* ai,BasicBlock* used_bb);

    bool queuePhi(BasicBlock*bb,AllocaInst*ai,::std::set<PhiInst*>&phi_set);
    void rmDeadPhi(Function*func);
    void reName(BasicBlock*bb,BasicBlock*pred,::std::map<AllocaInst*,Value*> incoming_vals);
    void generatePhi(AllocaInst*ai,::std::set<BasicBlock*>&define_bbs);

    bool isAllocVar(Instruction *instr);

public:
    Mem2Reg(Module *m) : FunctionPass(m){}
    ~Mem2Reg(){};
    void run() override;
};

#endif