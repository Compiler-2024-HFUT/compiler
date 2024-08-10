
#ifndef GENLOADIMM_HPP
#define GENLOADIMM_HPP

#include "analysis/InfoManager.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"

class GenLoadImm : public FunctionPass{
    public:
        GenLoadImm(Module *m,InfoManager*im): FunctionPass(m,im){}
        ~GenLoadImm(){};
        Modify runOnFunc(Function *function) override;
};

#endif
