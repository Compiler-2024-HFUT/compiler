#ifndef INLINE_HPP
#define INLINE_HPP

#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include <set>
//不考虑lir情况
std::vector<CallInst*> insertFunc(CallInst* call,std::list<Function*> called);
__attribute__((always_inline))  bool isEmpty(Function* f);
class FuncInline : public Pass{
private:
    ::std::set<CallInst *> func_call_;

    ::std::set<CallInst *> getCallInfo(Module*);
public:
    FuncInline(Module *m, InfoManager *im) : Pass(m, im){}
    // void insertFunc(CallInst* call);
    ~FuncInline(){};
    Modify run() override;
};

#endif
