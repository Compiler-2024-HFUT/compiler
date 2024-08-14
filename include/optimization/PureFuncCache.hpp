#ifndef PURE_FUNC_CACHE_HPP
#define PURE_FUNC_CACHE_HPP
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "analysis/funcAnalyse.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "PassManager.hpp"


class PureFuncCache : public FunctionPass {
    FuncAnalyse* fa;
public:
    void init()override {
        fa=info_man_->getInfo<FuncAnalyse>();
    }
    PureFuncCache(Module *m,InfoManager*im) : FunctionPass(m, im) {}
    Modify runOnFunc(Function*func) final;
    ~PureFuncCache(){}

};
#endif