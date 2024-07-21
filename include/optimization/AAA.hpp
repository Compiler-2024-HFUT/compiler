#pragma once

#include "midend/Module.hpp"
#include "PassManager.hpp"
#include<set>


class ActiveVar : public FunctionPass{
public:
    ActiveVar(Module *m) : FunctionPass(m) {}
    ~ActiveVar() {}
    void runOnFunc(Function *func) override;
    void get_def_use();
    void get_in_out_var();
   

private:
    Function* func;
    std::map<BasicBlock *, std::set<Value *>> map_use, map_def;
    std::map<BasicBlock *, std::set<Value *>> map_phi;
    std::map<BasicBlock *, std::set<Value *>> flive_in, flive_out;
    std::map<BasicBlock *, std::set<Value *>> ilive_in, ilive_out;
   
};
