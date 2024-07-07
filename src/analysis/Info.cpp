#include "analysis/Info.hpp"
#include "midend/Module.hpp"
#include "midend/Function.hpp"

void ModuleInfo::analyse() {
    analyseOnModule(module_);
    invalid = false;
}
void ModuleInfo::reAnalyse() {
    analyse();
}

void FunctionInfo::analyse() {
    for (Function* f : module_->getFunctions()) {
        // skip undefined func
        if(f->getBasicBlocks().size() == 0) continue;
        analyseOnFunc(f);
    }
    invalid = false;    // validate
}
void FunctionInfo::reAnalyse() {
    analyse();
}
