#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"

#include "HAsmFunc.hpp"
#include "HAsmBlock.hpp"
#include "HAsmLoc.hpp"

#include "RegAlloc.hpp"


class CodeGen {
public:
    CodeGen(Module *m) {
        m_ = new HAsmModule(m);
    }

public:
    //& main apis for asm code gen
    void module_gen();
    void globals_def_gen();
    void function_gen();
    void bb_gen();
    void instr_gen(Instruction *inst);

    //& phi inst gen
    void phi_union(Instruction *br_inst);
    void tmp_regs_restore();
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> idata_move(std::vector<HAsmLoc*>& src, std::vector<HAsmLoc*>&dst);
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> fdata_move(std::vector<HAsmLoc*>& src, std::vector<HAsmLoc*>&dst);

    //& stack space alloc 
    int stack_space_allocation();

    //& init or destruct stack space for callee 
    void callee_stack_prologue(int stack_size);
    void callee_stack_epilogue(int stack_size);

    //& args move for callee or caller
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> callee_iargs_move(Function *func);
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> callee_fargs_move(Function *func);
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> caller_iargs_move(CallInst *call);
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> caller_fargs_move(CallInst *call);

    //& linearized & labeling bbs
    void linearizing_and_labeling_bbs();

    //& regs save and restore about func call
    void caller_reg_store(Function* func,CallInst* call);
    void caller_reg_restore(Function* func, CallInst* call);

    //& temporary use regs for inst(all ops need to be loaded to regs for risc arch)
    void ld_tmp_regs_for_inst(Instruction *inst);
    void alloc_tmp_regs_for_inst(Instruction *inst);
    void store_tmp_reg_for_inst(Instruction *inst);

    //& help funcs for asm code gen
    Reg *get_asm_reg(Value* val);

    HAsmModule *get_module() { return m_; }

private:
    //& 方便代码生成 
    
    const int var_align = 2;
    const int func_align = 1;
    const int reg_size = 8;

    const int arg_reg_base = 10;  //~ reg_a0 or reg_fa0

    const std::vector<int> all_available_ireg_ids = {
        reg_t0, reg_t1, reg_t2, reg_t3, reg_t4, reg_t5, reg_t6,
        reg_s2, reg_s3, reg_s4, reg_s5, reg_s6, reg_s7, reg_s8, reg_s9, reg_s10, reg_s11,
        reg_a0, reg_a1, reg_a2, reg_a3, reg_a4, reg_a5, reg_a6, reg_a7
    };

    const std::vector<int> all_available_freg_ids = {
        reg_ft0, reg_ft1, reg_ft2, reg_ft3, reg_ft4, reg_ft5, reg_ft6, reg_ft7, reg_ft8, reg_ft9, reg_ft10, reg_ft11,
        reg_fs2, reg_fs3, reg_fs4, reg_fs5, reg_fs6, reg_fs7, reg_fs8, reg_fs9, reg_fs10, reg_fs11,
        reg_fa0, reg_fa1, reg_fa2, reg_fa3, reg_fa4, reg_fa5, reg_fa6, reg_fa7
    };

    const std::set<int> callee_saved_iregs = {
        reg_s0, reg_s2, reg_s3, 
        reg_s4, reg_s5, reg_s6, reg_s7, 
        reg_s8, reg_s9, reg_s10, reg_s11
    };

    const std::set<int> callee_saved_fregs = {
        reg_fs2, reg_fs3, 
        reg_fs4, reg_fs5, reg_fs6, reg_fs7, 
        reg_fs8, reg_fs9, reg_fs10, reg_fs11
    };


private:
    //& stack alloc 
    std::map<Value*, RegBase*> val2stack;

    //& global variable label gen for function using these global variable
    std::map<GlobalVariable*, Label*> global_variable_labels_table;

    //& function info statistic
    std::pair<std::set<int>, std::set<int>> used_iregs_pair; 
    std::pair<std::set<int>, std::set<int>> used_fregs_pair; 

    //& active intervals of value for reg alloc
    std::map<Value*, Interval*> ival2interval;
    std::map<Value*, Interval*> fval2interval;

    //& linearizing bbs and gen labels for bbs in function
    std::vector<BasicBlock*> linear_bbs;
    std::map<BasicBlock*, Label *> bb2label; 

    //& regs temporary used by inst need to be saved
    std::set<Interval*> use_tmp_regs_interval;
    std::set<int> cur_tmp_iregs;                                //// 当前用来保存栈上值的临时寄存器
    std::set<int> cur_tmp_fregs;                                //// 当前用来保存栈上值的临时寄存器
    std::map<int, RegBase*> tmp_iregs_loc;                      //// 临时寄存器原值保存的地址(位于函数栈布局中临时寄存器保存区)
    std::map<int, RegBase*> tmp_fregs_loc;                      //// 临时寄存器原值保存的地址(位于函数栈布局中临时寄存器保存区)
    
    std::set<RegBase*> free_locs_for_tmp_regs_saved;

    std::set<Value*> to_store_ivals; 
    std::set<Value*> to_store_fvals;

    std::set<int> istore_list;                                   //// 待保存原值的临时使用的整数寄存器
    std::set<int> fstore_list;                                   //// 待保存原值的临时使用的浮点寄存器

    std::map<int, RegBase*> caller_saved_ireg_locs;             //// caller在调用函数前保存寄存器的位置
    std::map<int, RegBase*> caller_saved_freg_locs;             //// caller在调用函数前保存寄存器的位置
    std::vector<int> caller_save_iregs;
    std::vector<int> caller_save_fregs;

    int cur_tmp_reg_saved_stack_offset = 0;
    int caller_saved_regs_stack_offset = 0;
    int caller_trans_args_stack_offset = 0;

private:
    HAsmModule *m_;
    HAsmFunc *cur_func_;
    HAsmBlock *cur_bb_;
};

#endif
