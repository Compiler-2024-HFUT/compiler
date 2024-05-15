#ifndef LIR_HPP
#define LIR_HPP

#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"

#include <vector>
using ::std::vector;
class LIR : public FunctionPass{
    private:
        ::std::vector<BasicBlock*> getBasicBlocks(Module* m);
        void mergeJJ(::std::vector<BasicBlock*> BBs);   //合并判断指令与跳转指令
        void oppositeJ(::std::vector<BasicBlock*> BBs); //反转判断指令
        void downGEP(::std::vector<BasicBlock*> BBs);   //降级getelementptr指令
        void offset(::std::vector<BasicBlock*> BBs);    //访存指令偏移量化

        void mergeIJJ(BranchInst* inst_br, CmpInst* inst_cmp, BasicBlock* bb);
        void mergeFJJ(BranchInst* inst_br, FCmpInst* inst_fcmp, BasicBlock* bb);

    public:
        LIR(Module *m): FunctionPass(m){}
        ~LIR(){};
        void run() override;
};

#endif