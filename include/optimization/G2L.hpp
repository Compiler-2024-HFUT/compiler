#ifndef G2L_HPP
#define G2L_HPP


#include "analysis/Dominators.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include "midend/Value.hpp"
#include <list>
#include <set>
using ::std::map,::std::set,::std::list;
class G2L : public Pass{
private:
    Dominators*cur_dom_;
    GlobalVariable*cur_global;
    map<Function*, list<Instruction*>> global_instrs_;
    // set<Function*> du_list_;
    set<Function*> use_list_;
    set<Function*> def_list_;

    set<Value*>stored;
    map<BasicBlock*, PhiInst*> new_phi;
    bool needStore(Value*incoming);
    void reName(BasicBlock*bb,BasicBlock*pred,Value* incoming_val,bool _stored);
    void generatePhi(::std::set<BasicBlock*>&define_bbs,::std::set<PhiInst*> &phi_set);
    bool queuePhi(BasicBlock*bb,::std::set<PhiInst*>&phi_set);
    void calDefAndUse(Function*cur_func,::std::set<BasicBlock*>&def_bbs,::std::set<BasicBlock*>&use_bbs);
    void calOneGlobal();
    void runGlobal();
    void rmLocallyGlob(GlobalVariable*global,BasicBlock*use);
    __attribute__((always_inline)) void clear(){
        global_instrs_.clear();
        // du_list_.clear();
        use_list_.clear();
        def_list_.clear();
        new_phi.clear();
        stored.clear();
    }
    __attribute__((always_inline)) void funcClear(){
        set<Value*>stored;
        map<BasicBlock*, PhiInst*> new_phi;
    }
public:
    G2L(Module *m) : Pass(m){}
    ~G2L(){};
    void run() override;
    // void runOnFunc(Function*const func,list<GlobalVariable*>&global_list);
};

#endif