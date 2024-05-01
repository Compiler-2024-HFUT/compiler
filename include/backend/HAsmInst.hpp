#ifndef HASMINST_HPP
#define HASMINST_HPP

#include <string>

#include "HAsmLoc.hpp"
#include "HAsmVal.hpp"

class HAsmBlock;
class HAsmFunc;

class HAsmInst {
public:
    enum AsmOpID {
        callee_save_regs,
        callee_start_stack_frame,
        callee_args_move,
        callee_restore_regs,
        callee_end_stack_frame,
        caller_save_regs,
        caller_args_move,
        caller_restore_regs,
        ld_tmp_regs,
        store_tmp_result,
        alloc_tmp_regs_with_save_initial_owner,
        restore_all_tmp_regs,
        phi_data_move,
        caller_save_call_result,
        callee_save_call_result,
        expand_stack_space,
        shrink_stack_space,
        add,
        subw,
        mulw,
        mul64,
        divw,
        remw,
        sraw,
        sllw,
        srlw,
        sra,
        sll,
        srl,
        land,
        fadds,
        fsubs,
        fmuls,
        fdivs,
        fcvtws,
        fcvtsw,
        zext,
        snez,
        seqz,
        feqs,
        fles,
        flts,
        fges,
        fgts,
        fnes,
        lw,
        flw,
        sw,
        fsw,
        memset,
        call,
        la,
        mv,
        beq,
        bne,
        bge,
        blt,
        fbeq,
        fbge,
        fbgt,
        fble,
        fblt,
        fbne,
        j,
        ret
    };
public:
    AsmOpID get_hasminst_op() { return op_id_; }
    static std::string get_instr_op_name(AsmOpID id) {
        switch(id) {
            case callee_save_regs: return "callee_save_regs"; break;
            case callee_start_stack_frame: return "callee_start_stack_frame"; break;
            case callee_args_move: return "callee_args_move"; break;
            case callee_restore_regs: return "callee_restore_regs"; break;
            case callee_end_stack_frame: return "callee_end_stack_frame"; break;
            case caller_save_regs: return "caller_save_regs"; break;
            case caller_args_move: return "caller_args_move"; break;
            case caller_restore_regs: return "caller_restore_regs"; break;
            case ld_tmp_regs: return "ld_tmp_regs"; break;
            case store_tmp_result: return "store_tmp_result"; break;
            case alloc_tmp_regs_with_save_initial_owner: return "alloc_tmp_regs_with_save_initial_owner"; break;
            case restore_all_tmp_regs: return "restore_all_tmp_regs"; break;
            case phi_data_move: return "phi_data_move"; break;
            case caller_save_call_result: return "caller_save_call_result"; break;
            case callee_save_call_result: return "callee_save_call_result"; break;
            case expand_stack_space: return "expand_stack_space"; break;
            case shrink_stack_space: return "shrink_stack_space"; break;
            case add: return "add"; break;
            case subw: return "subw"; break;
            case mulw: return "mulw"; break;
            case mul64: return "mul64"; break;
            case divw: return "divw"; break;
            case remw: return "remw"; break;
            case sraw: return "sraw"; break;
            case sllw: return "sllw"; break;
            case srlw: return "srlw"; break;
            case sra: return "sra"; break;
            case sll: return "sll"; break;
            case srl: return "srl"; break;
            case land: return "land"; break;
            case fadds: return "fadds"; break;
            case fsubs: return "fsubs"; break;
            case fmuls: return "fmuls"; break;
            case fdivs: return "fdivs"; break;
            case fcvtws: return "fcvtws"; break;
            case fcvtsw: return "fcvtsw"; break;
            case zext: return "zext"; break;
            case snez: return "snez"; break;
            case seqz: return "seqz"; break;
            case feqs: return "feqs"; break;
            case fles: return "fles"; break;
            case flts: return "flts"; break;
            case fges: return "fges"; break;
            case fgts: return "fgts"; break;
            case fnes: return "fnes"; break;
            case lw: return "lw"; break;
            case flw: return "flw"; break;
            case sw: return "sw"; break;
            case fsw: return "fsw"; break; 
            case memset: return "memset"; break;
            case call: return "call"; break;
            case la: return "la"; break;
            case mv: return "mv"; break;
            case beq: return "beq"; break;
            case bne: return "bne"; break;
            case bge: return "bge"; break;
            case blt: return "blt"; break;
            case fbeq: return "fbeq"; break;
            case fbge: return "fbge"; break;
            case fbgt: return "fbgt"; break;
            case fble: return "fble"; break;
            case fblt: return "fblt"; break;
            case fbne: return "fbne"; break;
            case j: return "j"; break;
            case ret: return "ret"; break;
            default: return "unknown"; break;
        }
    }

public:
    bool is_callee_save_regs() { return op_id_ == callee_save_regs; }
    bool is_callee_start_stack_frame() { return op_id_ == callee_start_stack_frame; }
    bool is_callee_args_move() { return op_id_ == callee_args_move; }
    bool is_callee_restore_regs() { return op_id_ == callee_restore_regs; }
    bool is_callee_end_stack_frame() { return op_id_ == callee_end_stack_frame; }
    bool is_caller_save_regs() { return op_id_ == caller_save_regs; }
    bool is_caller_args_move() { return op_id_ == caller_args_move; }
    bool is_caller_restore_regs() { return op_id_ == caller_restore_regs; }
    bool is_ld_tmp_regs() { return op_id_ == ld_tmp_regs; }
    bool is_store_tmp_result() { return op_id_ == store_tmp_result; }
    bool is_alloc_tmp_regs_with_save_initial_owner() { return op_id_ == alloc_tmp_regs_with_save_initial_owner; }
    bool is_restore_all_tmp_regs() { return op_id_ == restore_all_tmp_regs; }
    bool is_phi_data_move() { return op_id_ == phi_data_move; }
    bool is_caller_save_call_result() { return op_id_ == caller_save_call_result; }
    bool is_callee_save_call_result() { return op_id_ == callee_save_call_result; }
    bool is_expand_stack_space() { return op_id_ == expand_stack_space; }
    bool is_shrink_stack_space() { return op_id_ == shrink_stack_space; }
    bool is_add() { return op_id_ == add; }
    bool is_subw() { return op_id_ == subw; }
    bool is_mulw() { return op_id_ == mulw; }
    bool is_mul64() { return op_id_ == mul64; }
    bool is_divw() { return op_id_ == divw; }
    bool is_remw() { return op_id_ == remw; }
    bool is_sraw() { return op_id_ == sraw; }
    bool is_sllw() { return op_id_ == sllw; }
    bool is_srlw() { return op_id_ == srlw; }
    bool is_sra() { return op_id_ == sra; }
    bool is_sll() { return op_id_ == sll; }
    bool is_srl() { return op_id_ == srl; }
    bool is_land() { return op_id_ == land; }
    bool is_fadds() { return op_id_ == fadds; }
    bool is_fsubs() { return op_id_ == fsubs; }
    bool is_fmuls() { return op_id_ == fmuls; }
    bool is_fdivs() { return op_id_ == fdivs; }
    bool is_fcvtws() { return op_id_ == fcvtws; }
    bool is_fcvtsw() { return op_id_ == fcvtsw; }
    bool is_zext() { return op_id_ == zext; }
    bool is_snez() { return op_id_ == snez; }
    bool is_seqz() { return op_id_ == seqz; }
    bool is_feqs() { return op_id_ == feqs; }
    bool is_fles() { return op_id_ == fles; }
    bool is_flts() { return op_id_ == flts; }
    bool is_fges() { return op_id_ == fges; }
    bool is_fgts() { return op_id_ == fgts; }
    bool is_fnes() { return op_id_ == fnes; }
    bool is_lw() { return op_id_ == lw; }
    bool is_flw() { return op_id_ == flw; }
    bool is_sw() { return op_id_ == sw; }
    bool is_fsw() { return op_id_ == fsw; }
    bool is_memset() { return op_id_ == memset; }
    bool is_call() { return op_id_ == call; }
    bool is_la() { return op_id_ == la; }
    bool is_mv() { return op_id_ == mv; }
    bool is_beq() { return op_id_ == beq; }
    bool is_bne() { return op_id_ == bne; }
    bool is_bge() { return op_id_ == bge; }
    bool is_blt() { return op_id_ == blt; }
    bool is_fbeq() { return op_id_ == fbeq; }
    bool is_fbge() { return op_id_ == fbge; }
    bool is_fbgt() { return op_id_ == fbgt; }
    bool is_fble() { return op_id_ == fble; }
    bool is_fblt() { return op_id_ == fblt; }
    bool is_fbne() { return op_id_ == fbne; }
    bool is_j() { return op_id_ == j; }
    bool is_ret() { return op_id_ == ret; }

public:
    explicit HAsmInst(AsmOpID id, HAsmBlock *parent): op_id_(id), parent_(parent) {}    

    HAsmBlock *get_parent() { return parent_; }

    HAsmFunc *get_hasm_func();

    virtual std::string get_asm_code() = 0;
    virtual std::string print() = 0;

private:
    AsmOpID op_id_;
    HAsmBlock *parent_;
};

class CalleeSaveRegsInst: public HAsmInst {
public:
    CalleeSaveRegsInst(std::vector<std::pair<RegLoc*, RegBase*>> to_save_regs, HAsmBlock *parent)
        : callee_save_regs_(to_save_regs), HAsmInst(AsmOpID::callee_save_regs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<RegLoc*, RegBase*>> callee_save_regs_;
};

class CalleeStartStackFrameInst: public HAsmInst {
public:
    CalleeStartStackFrameInst(int stack_size, HAsmBlock *parent) 
        : stack_size_(stack_size), HAsmInst(AsmOpID::callee_start_stack_frame, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    int stack_size_;
};

class CalleeArgsMoveInst: public HAsmInst {
public:
    CalleeArgsMoveInst(std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs, std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs, HAsmBlock *parent) 
        : to_move_iargs_(to_move_iargs), to_move_fargs_(to_move_fargs), HAsmInst(AsmOpID::callee_args_move, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs_;
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs_;
};

class CalleeRestoreRegsInst: public HAsmInst {
public:
    CalleeRestoreRegsInst(std::vector<std::pair<RegLoc*, RegBase*>> to_load_regs, HAsmBlock *parent)
        : callee_load_regs_(to_load_regs), HAsmInst(AsmOpID::callee_restore_regs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<RegLoc*, RegBase*>> callee_load_regs_;   
};

class CalleeEndStackFrameInst: public HAsmInst {
public:
    CalleeEndStackFrameInst(int stack_size, HAsmBlock *parent) 
        : stack_size_(stack_size), HAsmInst(AsmOpID::callee_end_stack_frame, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    int stack_size_;
};

class CallerSaveRegsInst: public HAsmInst {
public:
    CallerSaveRegsInst(std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs, HAsmBlock *parent)
        : to_store_regs_(to_store_regs), HAsmInst(AsmOpID::caller_save_regs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs_;          
};

class CallerArgsMoveInst: public HAsmInst {
public:
    CallerArgsMoveInst(
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs, 
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs, 
        HAsmBlock *parent
    ) : to_move_fargs_(to_move_fargs), to_move_iargs_(to_move_iargs), HAsmInst(AsmOpID::caller_args_move, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs_;
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs_;
};

class CallerRestoreRegsInst: public HAsmInst {
public:
    CallerRestoreRegsInst(std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs, HAsmBlock *parent)
        : to_restore_regs_(to_restore_regs), HAsmInst(AsmOpID::caller_restore_regs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs_; 
};

class LdTmpRegsInst: public HAsmInst {
public:
    LdTmpRegsInst(std::vector<std::pair<RegLoc*, RegBase*>> to_load_tmp_regs, HAsmBlock *parent)
        : to_ld_tmp_regs_(to_load_tmp_regs), HAsmInst(AsmOpID::ld_tmp_regs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<RegLoc*, RegBase*>> to_ld_tmp_regs_;  
};

class StoreTmpResultInst: public HAsmInst {
public:
    StoreTmpResultInst(std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs, HAsmBlock *parent)
        : to_store_regs_(to_store_regs), HAsmInst(AsmOpID::store_tmp_result, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs_;  
};

class AllocTmpRegsWithSaveInitialOwnerInst: public HAsmInst {
public:
    AllocTmpRegsWithSaveInitialOwnerInst(std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs, std::vector<std::pair<RegLoc*, RegBase*>> to_ld_regs, HAsmBlock *parent)
        : to_store_regs_(to_store_regs), to_ld_regs_(to_ld_regs), HAsmInst(AsmOpID::alloc_tmp_regs_with_save_initial_owner, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs_;
    std::vector<std::pair<RegLoc*, RegBase*>> to_ld_regs_;  
};

class RestoreAllTmpRegsInst: public HAsmInst {
public:
    RestoreAllTmpRegsInst(std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs, HAsmBlock *parent)
        : to_restore_regs_(to_restore_regs), HAsmInst(AsmOpID::restore_all_tmp_regs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs_; 
};

class PhiDataMoveInst: public HAsmInst {
public:
    PhiDataMoveInst(
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_ilocs, 
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_flocs, 
        HAsmBlock *parent):
        to_move_ilocs_(to_move_ilocs), to_move_flocs_(to_move_flocs), HAsmInst(AsmOpID::phi_data_move, parent) {}

    std::string get_asm_code();
    std::string print();
private:
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_ilocs_;
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_flocs_; 
};

class CallerSaveCallResultInst: public HAsmInst {
public:
    CallerSaveCallResultInst(HAsmLoc *dst, Reg *src, HAsmBlock *parent):
        dst_(dst), src_(src), HAsmInst(AsmOpID::caller_save_call_result, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmLoc *dst_;
    Reg *src_;
};

class CalleeSaveCallResultInst: public HAsmInst {
public:
    CalleeSaveCallResultInst(RegLoc *dst, HAsmVal *src, HAsmBlock *parent):
        dst_(dst), src_(src), HAsmInst(AsmOpID::callee_save_call_result, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    RegLoc *dst_;
    HAsmVal *src_;
};

class ExpandStackSpaceInst: public HAsmInst {
public:
    ExpandStackSpaceInst(int expand_size, HAsmBlock *parent) : expand_size_(expand_size), HAsmInst(AsmOpID::expand_stack_space, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    int expand_size_;
};

class ShrinkStackSpaceInst: public HAsmInst {
public:
    ShrinkStackSpaceInst(int shrink_size, HAsmBlock *parent) : shrink_size_(shrink_size), HAsmInst(AsmOpID::shrink_stack_space, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    int shrink_size_;
};


class HAsmAddInst: public HAsmInst {
public:
    HAsmAddInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::add, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};


class HAsmSubwInst: public HAsmInst {
public:
    HAsmSubwInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::subw, parent) {}

    std::string get_asm_code();
    std::string print();
private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmMulwInst: public HAsmInst {
public:
    HAsmMulwInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::mulw, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmMul64Inst: public HAsmInst {
public:
    HAsmMul64Inst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::mul64, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmDivwInst: public HAsmInst {
public:
    HAsmDivwInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::divw, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmRemwInst: public HAsmInst {
public:
    HAsmRemwInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::remw, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmSrawInst: public HAsmInst {
public:
    HAsmSrawInst(Reg *dst, HAsmVal *rs1, Const *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::sraw, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    Const *rs2_;
};

class HAsmSllwInst: public HAsmInst {
public:
    HAsmSllwInst(Reg *dst, HAsmVal *rs1, Const *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::sllw, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    Const *rs2_;
};

class HAsmSrlwInst: public HAsmInst {
public:
    HAsmSrlwInst(Reg *dst, HAsmVal *rs1, Const *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::srlw, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    Const *rs2_;
};


class HAsmSraInst: public HAsmInst {
public:
    HAsmSraInst(Reg *dst, HAsmVal *rs1, Const *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::sra, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    Const *rs2_;
};

class HAsmSllInst: public HAsmInst {
public:
    HAsmSllInst(Reg *dst, HAsmVal *rs1, Const *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::sll, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    Const *rs2_;
};

class HAsmSrlInst: public HAsmInst {
public:
    HAsmSrlInst(Reg *dst, HAsmVal *rs1, Const *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::srl, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    Const *rs2_;
};

class HAsmLandInst: public HAsmInst {
public:
    HAsmLandInst(Reg *dst, Reg *rs1, Const *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::land, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    Reg *rs1_;
    Const *rs2_;
};

class HAsmFaddsInst: public HAsmInst {
public:
    HAsmFaddsInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::fadds, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmFsubsInst: public HAsmInst {
public:
    HAsmFsubsInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::fsubs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmFmulsInst: public HAsmInst {
public:
    HAsmFmulsInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::fmuls, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmFdivsInst: public HAsmInst {
public:
    HAsmFdivsInst(Reg *dst, HAsmVal *rs1, HAsmVal *rs2, HAsmBlock *parent) : 
        dst_(dst), rs1_(rs1), rs2_(rs2), HAsmInst(AsmOpID::fdivs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *rs1_;
    HAsmVal *rs2_;
};

class HAsmFcvtwsInst: public HAsmInst {
public:
    HAsmFcvtwsInst(Reg *dst, HAsmVal *src, HAsmBlock *parent) : 
        dst_(dst), src_(src), HAsmInst(AsmOpID::fcvtws, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *src_;
};

class HAsmFcvtswInst: public HAsmInst {
public:
    HAsmFcvtswInst(Reg *dst, HAsmVal *src, HAsmBlock *parent) : 
        dst_(dst), src_(src), HAsmInst(AsmOpID::fcvtsw, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *src_;
};

class HAsmZextInst: public HAsmInst {
public:
    HAsmZextInst(Reg *dst, Reg *src, HAsmBlock *parent) : 
        dst_(dst), src_(src), HAsmInst(AsmOpID::zext, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    Reg *src_;
};

class HAsmSnezInst: public HAsmInst {
public:
    HAsmSnezInst(Reg *dst, HAsmVal *cond, HAsmBlock *parent) : 
        dst_(dst), cond_(cond), HAsmInst(AsmOpID::snez, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *cond_;
};

class HAsmSeqzInst: public HAsmInst {
public:
    HAsmSeqzInst(Reg *dst, HAsmVal *cond, HAsmBlock *parent) : 
        dst_(dst), cond_(cond), HAsmInst(AsmOpID::seqz, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *cond_;
};

class HAsmFeqsInst: public HAsmInst {
public:
    HAsmFeqsInst(Reg *dst, HAsmVal *cond1, HAsmVal *cond2, HAsmBlock *parent) : 
        dst_(dst), cond1_(cond1), cond2_(cond2), HAsmInst(AsmOpID::feqs, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *cond1_;
    HAsmVal *cond2_;
};

class HAsmFlesInst: public HAsmInst {
public:
    HAsmFlesInst(Reg *dst, HAsmVal *cond1, HAsmVal *cond2, HAsmBlock *parent) : 
        dst_(dst), cond1_(cond1), cond2_(cond2), HAsmInst(AsmOpID::fles, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *cond1_;
    HAsmVal *cond2_;
};

class HAsmFltsInst: public HAsmInst {
public:
    HAsmFltsInst(Reg *dst, HAsmVal *cond1, HAsmVal *cond2, HAsmBlock *parent) : 
        dst_(dst), cond1_(cond1), cond2_(cond2), HAsmInst(AsmOpID::flts, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *cond1_;
    HAsmVal *cond2_;
};

class HAsmFgesInst: public HAsmInst {
public:
    HAsmFgesInst(Reg *dst, HAsmVal *cond1, HAsmVal *cond2, HAsmBlock *parent) : 
        dst_(dst), cond1_(cond1), cond2_(cond2), HAsmInst(AsmOpID::fges, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *cond1_;
    HAsmVal *cond2_;
};

class HAsmFgtsInst: public HAsmInst {
public:
    HAsmFgtsInst(Reg *dst, HAsmVal *cond1, HAsmVal *cond2, HAsmBlock *parent) : 
        dst_(dst), cond1_(cond1), cond2_(cond2), HAsmInst(AsmOpID::fgts, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *cond1_;
    HAsmVal *cond2_;
};

class HAsmFnesInst: public HAsmInst {
public:
    HAsmFnesInst(Reg *dst, HAsmVal *cond1, HAsmVal *cond2, HAsmBlock *parent) : 
        dst_(dst), cond1_(cond1), cond2_(cond2), HAsmInst(AsmOpID::fnes, parent) {}

    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    HAsmVal *cond1_;
    HAsmVal *cond2_;
};

class HAsmLwInst: public HAsmInst {
public:
    HAsmLwInst(Reg *dst, Label *label, HAsmBlock *parent): 
        dst_(dst), label_(label), is_label_(true), HAsmInst(AsmOpID::lw, parent) {}

    HAsmLwInst(Reg *dst, Reg *base, HAsmVal *offset, HAsmBlock *parent): 
        dst_(dst), base_(base), offset_(offset), is_label_(false), HAsmInst(AsmOpID::lw, parent) {}
    bool is_label() { return is_label_; }
    std::string get_asm_code();
    std::string print();

private:
    Reg *dst_;
    Reg *base_;
    HAsmVal *offset_;
    Label *label_;
    bool is_label_;

};

class HAsmFlwInst: public HAsmInst {
public:
    HAsmFlwInst(Reg *dst, Label *label, HAsmBlock *parent): 
        dst_(dst), label_(label), is_label_(true), HAsmInst(AsmOpID::flw, parent) {}

    HAsmFlwInst(Reg *dst, Reg *base, HAsmVal *offset, HAsmBlock *parent): 
        dst_(dst), base_(base), offset_(offset), is_label_(false), HAsmInst(AsmOpID::flw, parent) {}
    bool is_label() { return is_label_; }
    std::string get_asm_code();
    std::string print();
    
private:
    Reg *dst_;
    Reg *base_;
    HAsmVal *offset_;
    Label *label_;
    bool is_label_;
};

class HAsmSwInst: public HAsmInst {
public:
    HAsmSwInst(HAsmVal *val, Label *label, HAsmBlock *parent): 
        val_(val), label_(label), is_label_(true), HAsmInst(AsmOpID::sw, parent) {}

    HAsmSwInst(HAsmVal *val, Reg *base, HAsmVal *offset, HAsmBlock *parent): 
        val_(val), base_(base), offset_(offset), is_label_(false), HAsmInst(AsmOpID::sw, parent) {}
    bool is_label() { return is_label_; }
    std::string get_asm_code();
    std::string print();

private:
    HAsmVal *val_;
    Reg *base_;
    HAsmVal *offset_;
    Label *label_;
    bool is_label_;
};

class HAsmFswInst: public HAsmInst {
public:
    HAsmFswInst(HAsmVal *val, Label *label, HAsmBlock *parent): 
        val_(val), label_(label), is_label_(true), HAsmInst(AsmOpID::fsw, parent) {}

    HAsmFswInst(HAsmVal *val, Reg *base, HAsmVal *offset, HAsmBlock *parent): 
        val_(val), base_(base), offset_(offset), is_label_(false), HAsmInst(AsmOpID::fsw, parent) {}
    bool is_label() { return is_label_; }
    std::string get_asm_code();
    std::string print();

private:
    HAsmVal *val_;
    Reg *base_;
    HAsmVal *offset_;
    Label *label_;
    bool is_label_;
};

class HAsmMemsetInst: public HAsmInst {
public:
    HAsmMemsetInst(RegBase *base, int total_size, int step_size, bool is_fp, HAsmBlock *parent):
        base_(base), total_size_(total_size), step_size_(step_size), is_fp_(is_fp), HAsmInst(AsmOpID::memset, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    RegBase *base_;
    int total_size_;
    int step_size_;
    bool is_fp_;
};

class HAsmCallInst: public HAsmInst {
public:
    HAsmCallInst(Label *label, HAsmBlock *parent):
        label_(label), HAsmInst(AsmOpID::call, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    Label *label_;
};

class HAsmLaInst: public HAsmInst {
public:
    HAsmLaInst(Reg *dst, Label *label, HAsmBlock *parent):
        dst_(dst), label_(label), HAsmInst(AsmOpID::la, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    Reg *dst_; 
    Label *label_;    
};

class HAsmMvInst: public HAsmInst {
public:
    HAsmMvInst(Reg *dst, Reg *src, HAsmBlock *parent):
        dst_(dst), src_(src), HAsmInst(AsmOpID::mv, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    Reg *dst_; 
    Reg *src_;  
};

class HAsmBeqInst: public HAsmInst {
public:
    HAsmBeqInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::beq, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmBneInst: public HAsmInst {
public:
    HAsmBneInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::bne, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmBgeInst: public HAsmInst {
public:
    HAsmBgeInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::bge, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmBltInst: public HAsmInst {
public:
    HAsmBltInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::blt, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmFBeqInst: public HAsmInst {
public:
    HAsmFBeqInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::fbeq, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmFBgeInst: public HAsmInst {
public:
    HAsmFBgeInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::fbge, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmFBgtInst: public HAsmInst {
public:
    HAsmFBgtInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::fbgt, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmFBleInst: public HAsmInst {
public:
    HAsmFBleInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::fble, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmFBltInst: public HAsmInst {
public:
    HAsmFBltInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::fblt, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmFBneInst: public HAsmInst {
public:
    HAsmFBneInst(HAsmVal *lval, HAsmVal *rval, Label *label, HAsmBlock *parent):
        lval_(lval), rval_(rval), label_(label), HAsmInst(AsmOpID::fbne, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    HAsmVal *lval_;
    HAsmVal *rval_;
    Label *label_;
};

class HAsmJInst: public HAsmInst {
public:
    HAsmJInst(Label *label, HAsmBlock *parent):
        label_(label), HAsmInst(AsmOpID::j, parent) {}
    std::string get_asm_code();
    std::string print();
private:
    Label *label_;
};

class HAsmRetInst: public HAsmInst {
public:
    HAsmRetInst(HAsmBlock *parent): HAsmInst(AsmOpID::ret, parent) {}
    std::string get_asm_code();
    std::string print();
};

#endif