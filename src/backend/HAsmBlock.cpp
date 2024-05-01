#include "backend/HAsmBlock.hpp"
#include "backend/HAsmVal.hpp"
#include "backend/HAsm2Asm.hpp"

//#include "logging.hpp"

CalleeSaveRegsInst *HAsmBlock::create_callee_save_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_save_regs) {
    auto new_inst = new CalleeSaveRegsInst(to_save_regs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

CalleeStartStackFrameInst *HAsmBlock::create_callee_start_stack_frame(int stack_size) {
    auto new_inst = new CalleeStartStackFrameInst(stack_size, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

CalleeArgsMoveInst *HAsmBlock::create_callee_args_move(std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs, std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs) {
    auto new_inst = new CalleeArgsMoveInst(to_move_iargs, to_move_fargs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

CalleeEndStackFrameInst *HAsmBlock::create_callee_end_stack_frame(int stack_size) {
    auto new_inst = new CalleeEndStackFrameInst(stack_size, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

CalleeRestoreRegsInst *HAsmBlock::create_callee_restore_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_load_regs) {
    auto new_inst = new CalleeRestoreRegsInst(to_load_regs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

LdTmpRegsInst *HAsmBlock::create_ld_tmp_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_load_tmp_regs) {
    auto new_inst = new LdTmpRegsInst(to_load_tmp_regs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

AllocTmpRegsWithSaveInitialOwnerInst *HAsmBlock::create_alloc_tmp_regs_with_save_initial_owner(
    std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs,
    std::vector<std::pair<RegLoc*, RegBase*>> to_ld_regs
) {
    auto new_inst = new AllocTmpRegsWithSaveInitialOwnerInst(to_store_regs, to_ld_regs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

StoreTmpResultInst *HAsmBlock::create_store_tmp_result(std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs) {
    auto new_inst = new StoreTmpResultInst(to_store_regs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

RestoreAllTmpRegsInst *HAsmBlock::create_restore_all_tmp_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs) {
    auto new_inst = new RestoreAllTmpRegsInst(to_restore_regs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

CallerSaveRegsInst *HAsmBlock::create_caller_save_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs) {
    auto new_inst = new CallerSaveRegsInst(to_store_regs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

CallerRestoreRegsInst *HAsmBlock::create_caller_restore_regs(std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs) {
    auto new_inst = new CallerRestoreRegsInst(to_restore_regs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

CallerArgsMoveInst *HAsmBlock::create_caller_args_move(
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs, 
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs
) {
    auto new_inst = new CallerArgsMoveInst(to_move_fargs, to_move_iargs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

ExpandStackSpaceInst *HAsmBlock::create_expand_stack_space(int expand_size) {
    auto new_inst = new ExpandStackSpaceInst(expand_size, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

ShrinkStackSpaceInst *HAsmBlock::create_shrink_stack_space(int shrink_size) {
    auto new_inst = new ShrinkStackSpaceInst(shrink_size, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

PhiDataMoveInst *HAsmBlock::create_phi_data_move(
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_ilocs, 
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_flocs
) {
    auto new_inst = new PhiDataMoveInst(to_move_ilocs, to_move_flocs, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmFBeqInst *HAsmBlock::create_fbeq(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmFBeqInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFBgeInst *HAsmBlock::create_fbge(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmFBgeInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFBgtInst *HAsmBlock::create_fbgt(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmFBgtInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFBleInst *HAsmBlock::create_fble(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmFBleInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFBltInst *HAsmBlock::create_fblt(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmFBltInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFBneInst *HAsmBlock::create_fbne(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmFBneInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmBeqInst *HAsmBlock::create_beq(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmBeqInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmBgeInst *HAsmBlock::create_bge(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmBgeInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmBltInst *HAsmBlock::create_blt(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmBltInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmBneInst *HAsmBlock::create_bne(HAsmVal *lval, HAsmVal *rval, Label *label) {
    auto new_inst = new HAsmBneInst(lval, rval, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmJInst *HAsmBlock::create_j(Label *label) {
    auto new_inst = new HAsmJInst(label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmAddInst *HAsmBlock::create_add(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmAddInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmSubwInst *HAsmBlock::create_subw(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmSubwInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmMulwInst *HAsmBlock::create_mulw(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmMulwInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmMul64Inst *HAsmBlock::create_mul64(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmMul64Inst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmDivwInst *HAsmBlock::create_divw(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmDivwInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmRemwInst *HAsmBlock::create_remw(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmRemwInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmSrawInst *HAsmBlock::create_sraw(Reg *dst, HAsmVal *rs1, Const *rs2) {
    auto new_inst = new HAsmSrawInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmSllwInst *HAsmBlock::create_sllw(Reg *dst, HAsmVal *rs1, Const *rs2) {
    auto new_inst = new HAsmSllwInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmSrlwInst *HAsmBlock::create_srlw(Reg *dst, HAsmVal *rs1, Const *rs2) {
    auto new_inst = new HAsmSrlwInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmSraInst *HAsmBlock::create_sra(Reg *dst, HAsmVal *rs1, Const *rs2) {
    auto new_inst = new HAsmSraInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmSllInst *HAsmBlock::create_sll(Reg *dst, HAsmVal *rs1, Const *rs2) {
    auto new_inst = new HAsmSllInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmSrlInst *HAsmBlock::create_srl(Reg *dst, HAsmVal *rs1, Const *rs2) {
    auto new_inst = new HAsmSrlInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmLandInst *HAsmBlock::create_land(Reg *dst, Reg *rs1, Const *rs2) {
    auto new_inst = new HAsmLandInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmFaddsInst *HAsmBlock::create_fadds(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmFaddsInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFsubsInst *HAsmBlock::create_fsubs(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmFsubsInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFmulsInst *HAsmBlock::create_fmuls(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmFmulsInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFdivsInst *HAsmBlock::create_fdivs(Reg *dst, HAsmVal *rs1, HAsmVal *rs2) {
    auto new_inst = new HAsmFdivsInst(dst, rs1, rs2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmFcvtwsInst *HAsmBlock::create_fcvtws(Reg *dst, HAsmVal *src) {
    auto new_inst = new HAsmFcvtwsInst(dst, src, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFcvtswInst *HAsmBlock::create_fcvtsw(Reg *dst, HAsmVal *src) {
    auto new_inst = new HAsmFcvtswInst(dst, src, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmZextInst *HAsmBlock::create_zext(Reg *dst, Reg *src) {
    auto new_inst = new HAsmZextInst(dst, src, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmSeqzInst *HAsmBlock::create_seqz(Reg *dst, HAsmVal *cond) {
    auto new_inst = new HAsmSeqzInst(dst, cond, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmSnezInst *HAsmBlock::create_snez(Reg *dst, HAsmVal *cond) {
    auto new_inst = new HAsmSnezInst(dst, cond, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmFeqsInst *HAsmBlock::create_feqs(Reg *dst, HAsmVal *cond1, HAsmVal *cond2) {
    auto new_inst = new HAsmFeqsInst(dst, cond1, cond2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFgesInst *HAsmBlock::create_fges(Reg *dst, HAsmVal *cond1, HAsmVal *cond2) {
    auto new_inst = new HAsmFgesInst(dst, cond1, cond2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFgtsInst *HAsmBlock::create_fgts(Reg *dst, HAsmVal *cond1, HAsmVal *cond2) {
    auto new_inst = new HAsmFgtsInst(dst, cond1, cond2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFlesInst *HAsmBlock::create_fles(Reg *dst, HAsmVal *cond1, HAsmVal *cond2) {
    auto new_inst = new HAsmFlesInst(dst, cond1, cond2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFltsInst *HAsmBlock::create_flts(Reg *dst, HAsmVal *cond1, HAsmVal *cond2) {
    auto new_inst = new HAsmFltsInst(dst, cond1, cond2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFnesInst *HAsmBlock::create_fnes(Reg *dst, HAsmVal *cond1, HAsmVal *cond2) {
    auto new_inst = new HAsmFnesInst(dst, cond1, cond2, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmFlwInst *HAsmBlock::create_flw(Reg *dst, Label *label) {
    auto new_inst = new HAsmFlwInst(dst, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFlwInst *HAsmBlock::create_flw(Reg *dst, Reg *base, HAsmVal *offset) {
    auto new_inst = new HAsmFlwInst(dst, base, offset, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmLwInst *HAsmBlock::create_lw(Reg *dst, Label *label) {
    auto new_inst = new HAsmLwInst(dst, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmLwInst *HAsmBlock::create_lw(Reg *dst, Reg *base, HAsmVal *offset) {
    auto new_inst = new HAsmLwInst(dst, base, offset, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmFswInst *HAsmBlock::create_fsw(HAsmVal *val, Label *label) {
    auto new_inst = new HAsmFswInst(val, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmFswInst *HAsmBlock::create_fsw(HAsmVal *val, Reg *base, HAsmVal *offset) {
    auto new_inst = new HAsmFswInst(val, base, offset, this);
    insts_list_.push_back(new_inst);
    return new_inst;
} 
HAsmSwInst *HAsmBlock::create_sw(HAsmVal *val, Label *label) {
    auto new_inst = new HAsmSwInst(val, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmSwInst *HAsmBlock::create_sw(HAsmVal *val, Reg *base, HAsmVal *offset) {
    auto new_inst = new HAsmSwInst(val, base, offset, this);
    insts_list_.push_back(new_inst);
    return new_inst;
} 

HAsmCallInst *HAsmBlock::create_call(Label *label) {
    auto new_inst = new HAsmCallInst(label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

CallerSaveCallResultInst *HAsmBlock::create_caller_save_call_result(HAsmLoc *dst, Reg* src) {
    auto new_inst = new CallerSaveCallResultInst(dst, src, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
CalleeSaveCallResultInst *HAsmBlock::create_callee_save_call_result(RegLoc* dst, HAsmVal *src) {
    auto new_inst = new CalleeSaveCallResultInst(dst, src, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}
HAsmMemsetInst *HAsmBlock::create_memset(RegBase *base, int total_size, int step_size, bool is_fp) {
    auto new_inst = new HAsmMemsetInst(base, total_size, step_size, is_fp, this);
    insts_list_.push_back(new_inst);
    return new_inst;
} 

HAsmLaInst *HAsmBlock::create_la(Reg *dst, Label *label) {
    auto new_inst = new HAsmLaInst(dst, label, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmMvInst *HAsmBlock::create_mv(Reg *dst, Reg *src) {
    auto new_inst = new HAsmMvInst(dst, src, this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

HAsmRetInst *HAsmBlock::create_ret() {
    auto new_inst = new HAsmRetInst(this);
    insts_list_.push_back(new_inst);
    return new_inst;
}

std::string HAsmBlock::get_asm_code() {
    std::string asm_code;
    if(label_->get_label_name() != "") {
        asm_code += label_->get_label_name() + ":" + HAsm2Asm::newline;
    } 
    for(auto inst: insts_list_) {
        asm_code += inst->get_asm_code();
    }

    return asm_code;
}


std::string HAsmBlock::print() {
    std::string hasm_code;
    if(label_->get_label_name() != "") {
        hasm_code += label_->get_label_name() + ":" + HAsm2Asm::newline;
    } 
    for(auto inst: insts_list_) {
        hasm_code += inst->print();
    }

    return hasm_code;
}