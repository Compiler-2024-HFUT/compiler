//LIR不是LLVM的IR

#ifndef BREAK_GEP_HPP
#define BREAK_GEP_HPP

#include "analysis/InfoManager.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"

#include <vector>
using ::std::vector;

class BreakGEP : public FunctionPass{
    private:
        ::std::vector<BasicBlock*> getBasicBlocks(Function *function);
        ::std::list<Instruction *>& getInstList(BasicBlock*bb);
        void breakGEP(::std::vector<BasicBlock*> BBs);   //降级getelementptr指令

    public:
        BreakGEP(Module *m,InfoManager*im): FunctionPass(m,im){}
        ~BreakGEP(){};
        Modify runOnFunc(Function *function) override;
};

#endif