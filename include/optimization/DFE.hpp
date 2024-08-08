#ifndef DFE_HPP
#define DFE_HPP


#include "analysis/InfoManager.hpp"
#include "analysis/funcAnalyse.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "PassManager.hpp"
#include "midend/Module.hpp"
class DFE : public ModulePass{
private:
    FuncAnalyse *funanaly;
    // void init() override{
    //     funanaly=info_man_->getInfo<FuncAnalyse>();
    // }
public:
    DFE(Module *m,InfoManager*im) : ModulePass(m,im),funanaly(0){}
    ~DFE(){};
    Modify runOnModule(Module*module_) override;
};

#endif
