#ifndef ARR_REDUC_HPP
#define ARR_REDUC_HPP
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"
#include <map>
#include <set>
class ArrReduc:public FunctionPass{
private:
    std::set<BasicBlock*>visited;
    std::map<GetElementPtrInst*,LoadInst*> gep_load;
public:
    bool recurGep(AllocaInst*alloc,GetElementPtrInst* gep);
    bool canDelete(AllocaInst*alloc);
    Modify runOnFunc(Function*func);
    Modify runOnBB(BasicBlock*bb,std::map<GetElementPtrInst*,LoadInst*> gep_load);
    ArrReduc(Module*m,InfoManager *im):FunctionPass(m,im){}
};
#endif