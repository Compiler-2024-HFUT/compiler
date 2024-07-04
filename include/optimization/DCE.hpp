#ifndef DCE_HPP
#define DCE_HPP


#include "analysis/InfoManager.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include <set>
#include <vector>
class DCE : public FunctionPass{
private:
    // ::std::set<Instruction*> reachable_ins_;
    ::std::set<BasicBlock*> visited_bb_;
    // ::std::set<Instruction*> side_effect_ins_;
    ::std::set<Instruction*> valid_ins_;
    // ::std::set<Function*> callees_;
    ::std::vector<Instruction*> work_list_;
    void dfsReachableInstr(BasicBlock*);
    void clear(){
        // reachable_ins_.clear();
        // side_effect_ins_.clear();
        visited_bb_.clear();
        valid_ins_.clear();
        // callees_.clear();
        work_list_.clear();
    }
    void initInfo(Function*func);
    // bool isValidIns(Instruction*);
public:
    DCE(Module *m,InfoManager*im) : FunctionPass(m,im){}
    ~DCE(){};
    void runOnFunc(Function*func) override;
};

#endif
