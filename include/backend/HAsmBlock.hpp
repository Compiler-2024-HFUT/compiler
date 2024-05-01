#ifndef HASMBLOCK_HPP
#define HASMBLOCK_HPP

#include "midend/BasicBlock.hpp"

#include "HAsmModule.hpp"
#include "HAsmFunc.hpp"
#include "HAsmInst.hpp"
#include "HAsmLoc.hpp"


class HAsmBlock {
public:
    explicit HAsmBlock(HAsmFunc *parent, BasicBlock *bb, Label *label): parent_(parent), bb_(bb), label_(label) {}
    
public:

    CalleeSaveRegsInst *create_callee_save_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_save_regs);

    CalleeStartStackFrameInst *create_callee_start_stack_frame(int stack_size);

    CalleeArgsMoveInst *create_callee_args_move(
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs, 
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs
    );

    CalleeEndStackFrameInst *create_callee_end_stack_frame(int stack_size);

    CalleeRestoreRegsInst *create_callee_restore_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_load_regs);

    LdTmpRegsInst *create_ld_tmp_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_load_tmp_regs);

    AllocTmpRegsWithSaveInitialOwnerInst *create_alloc_tmp_regs_with_save_initial_owner(
        std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs,
        std::vector<std::pair<RegLoc*, RegBase*>> to_ld_regs
    );

    StoreTmpResultInst *create_store_tmp_result(std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs);

    RestoreAllTmpRegsInst *create_restore_all_tmp_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs);

    CallerSaveRegsInst *create_caller_save_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs);

    CallerRestoreRegsInst *create_caller_restore_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs);

    CallerArgsMoveInst *create_caller_args_move(
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs, 
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs
    );

    ExpandStackSpaceInst *create_expand_stack_space(int expand_size);

    ShrinkStackSpaceInst *create_shrink_stack_space(int shrink_size);

    PhiDataMoveInst *create_phi_data_move(
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_ilocs, 
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_flocs
    );

    HAsmFBeqInst *create_fbeq(HAsmVal *lval, HAsmVal *rval, Label *label);
    HAsmFBgeInst *create_fbge(HAsmVal *lval, HAsmVal *rval, Label *label);
    HAsmFBgtInst *create_fbgt(HAsmVal *lval, HAsmVal *rval, Label *label);
    HAsmFBleInst *create_fble(HAsmVal *lval, HAsmVal *rval, Label *label);
    HAsmFBltInst *create_fblt(HAsmVal *lval, HAsmVal *rval, Label *label);
    HAsmFBneInst *create_fbne(HAsmVal *lval, HAsmVal *rval, Label *label);

    HAsmBeqInst *create_beq(HAsmVal *lval, HAsmVal *rval, Label *label);
    HAsmBgeInst *create_bge(HAsmVal *lval, HAsmVal *rval, Label *label);
    HAsmBltInst *create_blt(HAsmVal *lval, HAsmVal *rval, Label *label);
    HAsmBneInst *create_bne(HAsmVal *lval, HAsmVal *rval, Label *label);

    HAsmJInst *create_j(Label *label);

    HAsmAddInst *create_add(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);
    HAsmSubwInst *create_subw(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);
    HAsmMulwInst *create_mulw(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);
    HAsmMul64Inst *create_mul64(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);
    HAsmDivwInst *create_divw(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);
    HAsmRemwInst *create_remw(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);

    HAsmSrawInst *create_sraw(Reg *dst, HAsmVal *rs1, Const *rs2);
    HAsmSllwInst *create_sllw(Reg *dst, HAsmVal *rs1, Const *rs2);
    HAsmSrlwInst *create_srlw(Reg *dst, HAsmVal *rs1, Const *rs2);

    HAsmSraInst *create_sra(Reg *dst, HAsmVal *rs1, Const *rs2);
    HAsmSllInst *create_sll(Reg *dst, HAsmVal *rs1, Const *rs2);
    HAsmSrlInst *create_srl(Reg *dst, HAsmVal *rs1, Const *rs2);

    HAsmLandInst *create_land(Reg *dst, Reg *rs1, Const *rs2);

    HAsmFaddsInst *create_fadds(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);
    HAsmFsubsInst *create_fsubs(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);
    HAsmFmulsInst *create_fmuls(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);
    HAsmFdivsInst *create_fdivs(Reg *dst, HAsmVal *rs1, HAsmVal *rs2);

    HAsmFcvtwsInst *create_fcvtws(Reg *dst, HAsmVal *src);
    HAsmFcvtswInst *create_fcvtsw(Reg *dst, HAsmVal *src);

    HAsmZextInst *create_zext(Reg *dst, Reg *src);

    HAsmSeqzInst *create_seqz(Reg *dst, HAsmVal *cond);
    HAsmSnezInst *create_snez(Reg *dst, HAsmVal *cond);

    HAsmFeqsInst *create_feqs(Reg *dst, HAsmVal *cond1, HAsmVal *cond2);
    HAsmFgesInst *create_fges(Reg *dst, HAsmVal *cond1, HAsmVal *cond2);
    HAsmFgtsInst *create_fgts(Reg *dst, HAsmVal *cond1, HAsmVal *cond2);
    HAsmFlesInst *create_fles(Reg *dst, HAsmVal *cond1, HAsmVal *cond2);
    HAsmFltsInst *create_flts(Reg *dst, HAsmVal *cond1, HAsmVal *cond2);
    HAsmFnesInst *create_fnes(Reg *dst, HAsmVal *cond1, HAsmVal *cond2);

    HAsmFlwInst *create_flw(Reg *dst, Label *label);
    HAsmFlwInst *create_flw(Reg *dst, Reg *base, HAsmVal *offset);
    HAsmLwInst *create_lw(Reg *dst, Label *label);
    HAsmLwInst *create_lw(Reg *dst, Reg *base, HAsmVal *offset);

    HAsmFswInst *create_fsw(HAsmVal *val, Label *label);
    HAsmFswInst *create_fsw(HAsmVal *val, Reg *base, HAsmVal *offset); 
    HAsmSwInst *create_sw(HAsmVal *val, Label *label);
    HAsmSwInst *create_sw(HAsmVal *val, Reg *base, HAsmVal *offset); 

    HAsmCallInst *create_call(Label *label);

    CallerSaveCallResultInst *create_caller_save_call_result(HAsmLoc *dst, Reg* src);
    CalleeSaveCallResultInst *create_callee_save_call_result(RegLoc* dst, HAsmVal *src);
    HAsmMemsetInst *create_memset(RegBase *base, int total_size, int step_size, bool is_fp); 

    HAsmLaInst *create_la(Reg *dst, Label *label);

    HAsmMvInst *create_mv(Reg *dst, Reg *src);

    HAsmRetInst *create_ret();

    void pop_inst() {
        insts_list_.pop_back();
    }

    void push_inst(HAsmInst *inst) {
        insts_list_.push_back(inst);
    }

    BasicBlock *get_bb() { return bb_; }

    HAsmFunc *get_parent() { return parent_; }

    std::string get_asm_code();
    std::string print();

private:
    std::list<HAsmInst*> insts_list_;
    HAsmFunc *parent_;
    BasicBlock *bb_;
    Label *label_;
};

#endif