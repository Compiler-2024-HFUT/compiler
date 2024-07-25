//LIR不是LLVM的IR

#ifndef MEMINSTOFFSET_HPP
#define MEMINSTOFFSET_HPP

#include "analysis/InfoManager.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"

#include <vector>
using ::std::vector;

class MemInstOffset : public FunctionPass{
    private:
        ::std::vector<BasicBlock*> getBasicBlocks(Function *function);
        ::std::list<Instruction *>& getInstList(BasicBlock*bb);
        void makeOffset(::std::vector<BasicBlock*> BBs);    //访存指令偏移量化
        template <class T>
        void makeOffsetT(T* inst_load_or_store, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb);
        void handleGEP(GetElementPtrInst* inst_gep, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb, bool flag);
        void handleAdd(BinaryInst* inst_ptr, ::std::list<Instruction*>& inst_list, ::std::list<Instruction*>::iterator& inst_pos, BasicBlock* bb, bool flag);

    public:
        MemInstOffset(Module *m,InfoManager*im): FunctionPass(m,im){}
        ~MemInstOffset(){};
        Modify runOnFunc(Function *function) override;
};

#endif