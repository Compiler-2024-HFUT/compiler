#ifndef INLINE_HPP
#define INLINE_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/IRGen.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

void insertFunc(CallInst* call,std::list<Function*> called);
__attribute__((always_inline))  bool isEmpty(Function* f);
class FuncInline : public Pass{
private:
    ::std::set<CallInst *> func_call_;

    ::std::set<CallInst *> getCallInfo(Module*);
public:
    FuncInline(Module *m) : Pass(m){}
    // void insertFunc(CallInst* call);
    ~FuncInline(){};
    void run() override;
};

#endif