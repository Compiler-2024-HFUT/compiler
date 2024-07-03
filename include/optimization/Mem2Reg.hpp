#ifndef MEM2REG_HPP
#define MEM2REG_HPP

#include "PassManager.hpp"
#include "analysis/Dominators.hpp"
#include <list>
#include <set>
using ::std::map,::std::set,::std::vector;
class Mem2Reg : public FunctionPass{
private:
    Dominators*cur_dom_;
    ::std::map<BasicBlock*, ::std::map<AllocaInst*,PhiInst*>> new_phi;
    ::std::list<AllocaInst*>allocas;


    BasicBlock* isOnlyInOneBB(AllocaInst*ai);
    void calDefAndUse(AllocaInst*ai,::std::set<BasicBlock*>&def,::std::set<BasicBlock*>&use);
    void rmLocallyAlloc(AllocaInst* ai,BasicBlock* used_bb);

    bool queuePhi(BasicBlock*bb,AllocaInst*ai,::std::set<PhiInst*>&phi_set);
    void reName(BasicBlock*bb,BasicBlock*pred,::std::map<AllocaInst*,Value*> incoming_vals);
    void generatePhi(AllocaInst*ai,::std::set<BasicBlock*>&define_bbs,::std::set<PhiInst*> &phi_set);
    bool isAllocVar(Instruction *instr);

public:
    Mem2Reg(Module *m, InfoManager *im) : FunctionPass(m, im){
        cur_dom_=info_man_->getInfo<Dominators>();
    }
    ~Mem2Reg(){};
    // void run()=default;
    void runOnFunc(Function*func)override;
};

#endif