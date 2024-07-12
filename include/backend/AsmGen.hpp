//该文件里写ASM的内存构建
#ifndef ASMGEN_HPP
#define ASMGEN_HPP

#include "midend/IRVisitor.hpp"
#include "Asm.hpp"
#include "RegAlloc.hpp"
/*需要访问的中端数据结构如下：
module
function
basicblock
instruction（不包含instruction，使用的是instruction的子类）:
binaryinst
cmpinst
fcmpinst
callinst
branchinst
returninst
getelementptrinst
storeinst
memsetinst
loadinst
allocainst
zextinst
sitofpins
fptosiinst
phiinst
cmbrinst
fcmbrinst
loadoffsetinst
storeoffsetinst
*/

class AsmGen : public IRVisitor{
    public:
        AsmGen(Module* module){
            asm_unit = new AsmUnit(module);
        }
        AsmUnit* getAsmUnit(){return asm_unit;}
    
    //访问者函数
    private:
        virtual void visit(Module &node) override;
        virtual void visit(Function &node) override;
        virtual void visit(BasicBlock &node) override;
        virtual void visit(BinaryInst &node) override;
        virtual void visit(CmpInst &node) override;
        virtual void visit(FCmpInst &node) override;
        virtual void visit(CallInst &node) override;
        virtual void visit(BranchInst &node) override;
        virtual void visit(ReturnInst &node) override;
        virtual void visit(GetElementPtrInst &node) override;
        virtual void visit(StoreInst &node) override;
        virtual void visit(MemsetInst &node) override;
        virtual void visit(LoadInst &node) override;
        virtual void visit(AllocaInst &node) override;
        virtual void visit(ZextInst &node) override;
        virtual void visit(SiToFpInst &node) override;
        virtual void visit(FpToSiInst &node) override;
        virtual void visit(PhiInst &node) override;
        virtual void visit(CmpBrInst &node) override;
        virtual void visit(FCmpBrInst &node) override;
        virtual void visit(LoadOffsetInst &node) override;
        virtual void visit(StoreOffsetInst &node) override;

    private:
        //& linearized & labeling bbs
        void linearizing_and_labeling_bbs();

        //& stack space alloc 
        int stack_space_allocation();
    private:
        AsmUnit* asm_unit;
        Subroutine* subroutine;
        Sequence* sequence;

        //& active intervals of value for reg alloc
        std::map<Value*, Interval*> ival2interval;
        std::map<Value*, Interval*> fval2interval;

        int cur_tmp_reg_saved_stack_offset = 0;
        int caller_saved_regs_stack_offset = 0;
        int caller_trans_args_stack_offset = 0;

    //& regs temporary used by inst need to be saved
    std::set<Interval*> use_tmp_regs_interval;
        std::set<int> cur_tmp_iregs;                                //// 当前用来保存栈上值的临时寄存器
        std::set<int> cur_tmp_fregs;                                //// 当前用来保存栈上值的临时寄存器
        std::map<int, IRIA*> tmp_iregs_loc;                      //// 临时寄存器原值保存的地址(位于函数栈布局中临时寄存器保存区)
        std::map<int, IRIA*> tmp_fregs_loc;                      //// 临时寄存器原值保存的地址(位于函数栈布局中临时寄存器保存区)

        std::set<IRIA*> free_locs_for_tmp_regs_saved;

        //& linearizing bbs and gen labels for bbs in function
        std::vector<BasicBlock*> linear_bbs;
        std::map<BasicBlock*, Label *> bb2label; 

        //& function info statistic
        std::pair<std::set<int>, std::set<int>> used_iregs_pair; 
        std::pair<std::set<int>, std::set<int>> used_fregs_pair; 
        
        //& stack alloc 
        std::map<Value*, IRIA*> val2stack;

        const std::set<int> callee_saved_iregs = {
            static_cast<int>(RISCV::GPR::s0),
            static_cast<int>(RISCV::GPR::s2),
            static_cast<int>(RISCV::GPR::s3),
            static_cast<int>(RISCV::GPR::s4),
            static_cast<int>(RISCV::GPR::s5),
            static_cast<int>(RISCV::GPR::s6),
            static_cast<int>(RISCV::GPR::s7),
            static_cast<int>(RISCV::GPR::s8),
            static_cast<int>(RISCV::GPR::s9),
            static_cast<int>(RISCV::GPR::s10),
            static_cast<int>(RISCV::GPR::s11)
        };

        const std::set<int> callee_saved_fregs = {
            static_cast<int>(RISCV::FPR::fs2),
            static_cast<int>(RISCV::FPR::fs3),
            static_cast<int>(RISCV::FPR::fs4),
            static_cast<int>(RISCV::FPR::fs5),
            static_cast<int>(RISCV::FPR::fs6),
            static_cast<int>(RISCV::FPR::fs7),
            static_cast<int>(RISCV::FPR::fs8),
            static_cast<int>(RISCV::FPR::fs9),
            static_cast<int>(RISCV::FPR::fs10),
            static_cast<int>(RISCV::FPR::fs11)
        };

        //& 方便代码生成 
    
        const int var_align = 2;
        const int func_align = 1;
        const int reg_size = 8;

        const int arg_reg_base = 10;  //~ reg_a0 or reg_fa0

        //& init or destruct stack space for callee 
        void callee_stack_prologue(int stack_size);
        void callee_stack_epilogue(int stack_size);

        //& args move for callee or caller
        std::vector<std::pair<AddressMode*, AddressMode*>> callee_iargs_move(Function *func);
        std::vector<std::pair<AddressMode*, AddressMode*>> callee_fargs_move(Function *func);
        std::vector<std::pair<AddressMode*, AddressMode*>> caller_iargs_move(CallInst *call);
        std::vector<std::pair<AddressMode*, AddressMode*>> caller_fargs_move(CallInst *call);

        //& temporary use regs for inst(all ops need to be loaded to regs for risc arch)
        void ld_tmp_regs_for_inst(Instruction *inst);
        void alloc_tmp_regs_for_inst(Instruction *inst);
        void store_tmp_reg_for_inst(Instruction *inst);

        //& regs save and restore about func call
        void caller_reg_store(Function* func,CallInst* call);
        void caller_reg_restore(Function* func, CallInst* call);

        std::map<int, IRIA*> caller_saved_ireg_locs;             //// caller在调用函数前保存寄存器的位置
        std::map<int, IRIA*> caller_saved_freg_locs;             //// caller在调用函数前保存寄存器的位置
        std::vector<int> caller_save_iregs;
        std::vector<int> caller_save_fregs;

        
        std::set<int> istore_list;                                   //// 待保存原值的临时使用的整数寄存器
        std::set<int> fstore_list;                                   //// 待保存原值的临时使用的浮点寄存器

        std::set<Value*> to_store_ivals; 
        std::set<Value*> to_store_fvals;

            //& phi inst gen
    void phi_union(Instruction *br_inst);
    void tmp_regs_restore();
    std::vector<std::pair<AddressMode*, AddressMode*>> idata_move(std::vector<AddressMode*>& src, std::vector<AddressMode*>&dst);
    std::vector<std::pair<AddressMode*, AddressMode*>> fdata_move(std::vector<AddressMode*>& src, std::vector<AddressMode*>&dst);


        //& help funcs for asm code gen
    Val *get_asm_reg(Value* val);

        //& global variable label gen for function using these global variable
    std::map<GlobalVariable*, Label*> global_variable_labels_table;

    
};



#endif