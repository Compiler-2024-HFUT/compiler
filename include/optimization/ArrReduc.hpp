#ifndef ARR_REDUC_HPP
#define ARR_REDUC_HPP
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "analysis/funcAnalyse.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "midend/Value.hpp"
#include "optimization/PassManager.hpp"
#include <map>
#include <set>
class ArrReduc:public FunctionPass{
private:
    void init()override{
        funca=info_man_->getInfo<FuncAnalyse>();
    }
    FuncAnalyse *funca;
    std::set<BasicBlock*>visited;
    std::map<GetElementPtrInst*,LoadInst*> gep_load;
public:
    bool recurGep(AllocaInst*alloc,GetElementPtrInst* gep);
    bool canDelete(AllocaInst*alloc);
    virtual Modify runOnFunc(Function*func) override;
    Modify runOnBB(BasicBlock*bb,std::map<GetElementPtrInst*,LoadInst*> gep_load,std::map<GetElementPtrInst*,Value*>gep_curval);
    Modify runOnBBConst(BasicBlock*bb,std::map<GetElementPtrInst*,LoadInst*> gep_load,std::map<GetElementPtrInst*,Value*>gep_curval);
    ArrReduc(Module*m,InfoManager *im):FunctionPass(m,im){}
};
#endif