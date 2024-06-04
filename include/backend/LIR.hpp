//LIR不是LLVM的IR

#ifndef LIR_HPP
#define LIR_HPP

#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"

#include <vector>
using ::std::vector;

class LIR : public FunctionPass{
    private:
        ::std::vector<BasicBlock*> getBasicBlocks(Module* m);
        ::std::list<Instruction *>& getInstList(BasicBlock*bb);
        void mergeJJ(::std::vector<BasicBlock*> BBs);   //合并判断指令与跳转指令（cmp+br--->cmpbr）
        void oppositeJ(::std::vector<BasicBlock*> BBs); //反转判断指令
        void breakGEP(::std::vector<BasicBlock*> BBs);   //降级getelementptr指令
        void makeOffset(::std::vector<BasicBlock*> BBs);    //访存指令偏移量化
        template <class T>
        void mergeTJJ(BranchInst* inst_br, T* inst_I_or_F_cmp, BasicBlock* bb);  
        template <class T>
        void oppositeTJ(::std::list<Instruction*>& inst_list, T* inst_I_or_F_cmp, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb);
        template <class T>
        void makeOffsetT(T* inst_load_or_store, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb);
        void handleGEP(GetElementPtrInst* inst_gep, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb, bool flag);
        void handleAdd(Instruction* inst_ptr, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb, bool flag);

    public:
        LIR(Module *m): FunctionPass(m){}
        ~LIR(){};
        void run() override;
};

#endif