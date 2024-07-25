//LIR不是LLVM的IR

#ifndef COMBINE_JJ_HPP
#define COMBINE_JJ_HPP

#include "analysis/InfoManager.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"

#include <vector>
using ::std::vector;

//比较指令LIR化
//合并条件允许情况下的比较指令与跳转指令
class CombineJJ : public FunctionPass{
    private:
        ::std::vector<BasicBlock*> getBasicBlocks(Function *function);
        ::std::list<Instruction *>& getInstList(BasicBlock*bb);
        void mergeJJ(::std::vector<BasicBlock*> BBs);   //合并判断指令与跳转指令（cmp+br--->cmpbr）
        void oppositeJ(::std::vector<BasicBlock*> BBs); //反转判断指令
        template <class T>
        void mergeTJJ(BranchInst* inst_br, T* inst_I_or_F_cmp, BasicBlock* bb);  
        template <class T>
        void oppositeTJ(::std::list<Instruction*>& inst_list, T* inst_I_or_F_cmp, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb);


    public:
        CombineJJ(Module *m,InfoManager*im): FunctionPass(m,im){}
        ~CombineJJ(){};
        Modify runOnFunc(Function *function) override;
};

#endif