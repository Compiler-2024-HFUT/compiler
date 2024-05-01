#include "backend/HAsmInst.hpp"
#include "backend/HAsmLoc.hpp"
#include "backend/HAsm2Asm.hpp"
#include "backend/HAsmFunc.hpp"

//#include "logging.hh"

HAsmFunc *HAsmInst::get_hasm_func() { return parent_->get_parent(); }

std::string CalleeSaveRegsInst::get_asm_code() {
    std::string asm_code;

    for(auto save_reg_pair: callee_save_regs_) {
        auto regloc = save_reg_pair.first;
        auto regbase = save_reg_pair.second;
        if(regloc->is_float())
            asm_code += HAsm2Asm::fsw(new Reg(regloc->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
        else
            asm_code += HAsm2Asm::sd(new Reg(regloc->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
    }

    return asm_code;
}

std::string CalleeSaveRegsInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    code += "[ ";
    int num = callee_save_regs_.size();
    int i = 0;
    for(auto save_reg_pair: callee_save_regs_) {
        auto regloc = save_reg_pair.first;
        auto regbase = save_reg_pair.second;
        code += regloc->print();
        code += " -> ";
        code += regbase->print();
        if(i < num - 1) {
            code += ", ";
        }
        i++;
    }
    code += " ]";
    code += HAsm2Asm::newline;
    return code;
}

std::string CalleeStartStackFrameInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::mv(new Reg(reg_fp, false), new Reg(reg_sp, false));
    asm_code += HAsm2Asm::addi(new Reg(reg_sp, false), new Reg(reg_sp, false), -stack_size_);
    return asm_code;
}

std::string CalleeStartStackFrameInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + std::to_string(stack_size_) + HAsm2Asm::newline;
}

std::string CalleeArgsMoveInst::get_asm_code() {
    std::string asm_code;
    for(auto iarg_pair: to_move_iargs_) {
        auto target_loc = iarg_pair.first;
        auto src_loc = iarg_pair.second;
        if(dynamic_cast<RegLoc*>(target_loc) && dynamic_cast<RegLoc*>(src_loc)) {
            asm_code += HAsm2Asm::mv(new Reg(dynamic_cast<RegLoc*>(target_loc)->get_reg_id(), false), new Reg(dynamic_cast<RegLoc*>(src_loc)->get_reg_id(), false));
        } else if(dynamic_cast<RegBase*>(target_loc) && dynamic_cast<RegLoc*>(src_loc)) {
            auto regbase = dynamic_cast<RegBase*>(target_loc);
            auto offset = regbase->get_offset();
            if(offset > 2047 || offset < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::sd(new Reg(dynamic_cast<RegLoc*>(src_loc)->get_reg_id(), false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::sd(new Reg(dynamic_cast<RegLoc*>(src_loc)->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), offset);
            }
        } else if(dynamic_cast<RegLoc*>(target_loc) && dynamic_cast<RegBase*>(src_loc)) {
            auto regbase = dynamic_cast<RegBase*>(src_loc);
            auto offset = regbase->get_offset();
            if(offset > 2047 || offset < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(dynamic_cast<RegLoc*>(target_loc)->get_reg_id(), false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(dynamic_cast<RegLoc*>(target_loc)->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), offset);
            }
        } else {
          //  LOG(ERROR) << "出现未预期情况";
        }
    }
    
    for(auto farg_pair: to_move_fargs_) {
        auto target_loc = farg_pair.first;
        auto src_loc = farg_pair.second;
        if(dynamic_cast<RegLoc*>(target_loc) && dynamic_cast<RegLoc*>(src_loc)) {
            asm_code += HAsm2Asm::fmvs(new Reg(dynamic_cast<RegLoc*>(target_loc)->get_reg_id(), true), new Reg(dynamic_cast<RegLoc*>(src_loc)->get_reg_id(), true));
        } else if(dynamic_cast<RegBase*>(target_loc) && dynamic_cast<RegLoc*>(src_loc)) {
            auto regbase = dynamic_cast<RegBase*>(target_loc);
            auto offset = regbase->get_offset();
            if(offset > 2047 || offset < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::fsw(new Reg(dynamic_cast<RegLoc*>(src_loc)->get_reg_id(), true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::fsw(new Reg(dynamic_cast<RegLoc*>(src_loc)->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), offset);
            }
        } else if(dynamic_cast<RegLoc*>(target_loc) && dynamic_cast<RegBase*>(src_loc)) {
            auto regbase = dynamic_cast<RegBase*>(src_loc);
            auto offset = regbase->get_offset();
            if(offset > 2047 || offset < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(dynamic_cast<RegLoc*>(target_loc)->get_reg_id(), true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(dynamic_cast<RegLoc*>(target_loc)->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), offset);
            }
        } else {
           // LOG(ERROR) << "出现未预期情况";
        }
    }

    return asm_code;
}

std::string CalleeArgsMoveInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    int num, i;
    if(! to_move_iargs_.empty()) {
        code += "[ ";
        num = to_move_iargs_.size();
        i = 0;
        for(auto iarg_pair: to_move_iargs_) {
            auto target = iarg_pair.first;
            auto src = iarg_pair.second;
            code += src->print();
            code += " -> ";
            code += target->print();
            if(i < num - 1) {
                code += ", ";
            }
            i++;
        }
        code += " ]";
    }
    if(! to_move_iargs_.empty() && ! to_move_fargs_.empty()) {
        code += ", ";
    }
    if(! to_move_fargs_.empty()) {
        code += "[ ";
        num = to_move_fargs_.size();
        i = 0;
        for(auto farg_pair: to_move_fargs_) {
            auto target = farg_pair.first;
            auto src = farg_pair.second;
            code += src->print();
            code += " -> ";
            code += target->print();
            if(i < num - 1) {
                code += ", ";
            }
            i++;
        }
        code += "]";
    }
    code += HAsm2Asm::newline;
    return code;
}

std::string CalleeRestoreRegsInst::get_asm_code() {
    std::string asm_code;
    for(auto load_reg_pair: callee_load_regs_) {
        auto reg = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        if(reg->is_float())
            asm_code += HAsm2Asm::flw(new Reg(reg->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
        else
            asm_code += HAsm2Asm::ld(new Reg(reg->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
    }

    return asm_code;
}

std::string CalleeRestoreRegsInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    code += "[ ";
    int num = callee_load_regs_.size();
    int i = 0;
    for(auto load_reg_pair: callee_load_regs_) {
        auto regloc = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        code += regbase->print();
        code += " -> ";
        code += regloc->print();
        if(i < num - 1) {
            code += ", ";
        }
        i++;
    }
    code += " ]";
    code += HAsm2Asm::newline;
    return code;
}

std::string CalleeEndStackFrameInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::addi(new Reg(reg_sp, false), new Reg(reg_sp, false), stack_size_);
    return asm_code;
}

std::string CalleeEndStackFrameInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + std::to_string(stack_size_) + HAsm2Asm::newline;
}

std::string CallerSaveRegsInst::get_asm_code() {
    std::string asm_code;

    for(auto save_reg_pair: to_store_regs_) {
        auto regloc = save_reg_pair.first;
        auto regbase = save_reg_pair.second;
        if(regloc->is_float())
            asm_code += HAsm2Asm::fsw(new Reg(regloc->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
        else
            asm_code += HAsm2Asm::sd(new Reg(regloc->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
    }

    return asm_code;
}

std::string CallerSaveRegsInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    code += "[ ";
    int num = to_store_regs_.size();
    int i = 0;
    for(auto save_reg_pair: to_store_regs_) {
        auto regloc = save_reg_pair.first;
        auto regbase = save_reg_pair.second;
        code += regloc->print();
        code += " -> ";
        code += regbase->print();
        if(i < num - 1) {
            code += ", ";
        }
        i++;
    }
    code += " ]";
    code += HAsm2Asm::newline;
    return code;
}

std::string CallerArgsMoveInst::get_asm_code() {
    std::string asm_code;
    for(auto move_farg_pair: to_move_fargs_) {
        auto target = move_farg_pair.first;
        auto src = move_farg_pair.second;
        auto const_src = dynamic_cast<ConstPool*>(src);
        auto reg_src = dynamic_cast<RegLoc*>(src);
        auto regbase_src = dynamic_cast<RegBase*>(src); 
        if(const_src) {
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_src->get_fval()));
                asm_code += HAsm2Asm::fmvsx(new Reg(reg_target->get_reg_id(), true), new Reg(reg_s1, false));
            } else if(dynamic_cast<RegBase*>(target)) {
                asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_src->get_fval()));
                asm_code += HAsm2Asm::fmvsx(new Reg(reg_target->get_reg_id(), true), new Reg(reg_s1, false));
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        } else if(reg_src) {
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::fmvs(new Reg(reg_target->get_reg_id(), true), new Reg(reg_src->get_reg_id(), true));
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::fsw(new Reg(reg_src->get_reg_id(), true), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::fsw(new Reg(reg_src->get_reg_id(), true), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        } else {
            if(regbase_src->get_offset() > 2047 || regbase_src->get_offset() < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_src->get_reg_id(), false), regbase_src->get_offset()); 
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(regbase_src->get_reg_id(), false), regbase_src->get_offset()); 
            }
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::fmvs(new Reg(reg_target->get_reg_id(), true), new Reg(reg_fs1, true));
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        }
    }

    for(auto move_iarg_pair: to_move_iargs_) {
        auto target = move_iarg_pair.first;
        auto src = move_iarg_pair.second;
        auto const_src = dynamic_cast<ConstPool*>(src);
        auto reg_src = dynamic_cast<RegLoc*>(src);
        auto regbase_src = dynamic_cast<RegBase*>(src); 
        if(const_src) {
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::addi(new Reg(reg_target->get_reg_id(), false), new Reg(reg_zero, false), const_src->get_ival());
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(reg_zero, false), const_src->get_ival());
                    asm_code += HAsm2Asm::sd(new Reg(reg_s1, false), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(reg_zero, false), const_src->get_ival());
                    asm_code += HAsm2Asm::sd(new Reg(reg_s1, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        } else if(reg_src) {
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::mv(new Reg(reg_target->get_reg_id(), false), new Reg(reg_src->get_reg_id(), false));
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::sd(new Reg(reg_src->get_reg_id(), false), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::sd(new Reg(reg_src->get_reg_id(), false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        } else {
            if(regbase_src->get_offset() > 2047 || regbase_src->get_offset() < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_src->get_reg_id(), false), regbase_src->get_offset()); 
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(regbase_src->get_reg_id(), false), regbase_src->get_offset()); 
            }
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::mv(new Reg(reg_target->get_reg_id(), false), new Reg(reg_ra, false));
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::sd(new Reg(reg_ra, false), new Reg(reg_s1, false), 0);
                } else {
                    asm_code += HAsm2Asm::sd(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        }
    } 

    return asm_code;
}

std::string CallerArgsMoveInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    int num, i;
    if(! to_move_fargs_.empty()) {
        num = to_move_fargs_.size();
        i = 0;
        code += "[ ";
        for(auto farg_pair: to_move_fargs_) {
            auto target = farg_pair.first;
            auto src = farg_pair.second;
            code += src->print();
            code += " -> ";
            code += target->print();
            if(i < num - 1) {
                code += ", ";
            }
            i++;
        }
        code += " ]";
    }
    if(! to_move_fargs_.empty() && ! to_move_iargs_.empty()) {
        code += ", ";
    } 
    if(! to_move_iargs_.empty()) {
        code += "[ ";
        num = to_move_iargs_.size();
        i = 0;
        for(auto iarg_pair: to_move_iargs_) {
            auto target = iarg_pair.first;
            auto src = iarg_pair.second;
            code += src->print();
            code += " -> ";
            code += target->print();
            if(i < num - 1) {
                code += ", ";
            }
            i++;
        }
        code += "]";
    }
    code += HAsm2Asm::newline;
    return code;
}

std::string CallerRestoreRegsInst::get_asm_code() {
    std::string asm_code;
    for(auto load_reg_pair: to_restore_regs_) {
        auto reg = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        if(reg->is_float())
            asm_code += HAsm2Asm::flw(new Reg(reg->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
        else
            asm_code += HAsm2Asm::ld(new Reg(reg->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
    }

    return asm_code;
}

std::string CallerRestoreRegsInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    code += "[ ";
    int num = to_restore_regs_.size();
    int i = 0;
    for(auto load_reg_pair: to_restore_regs_) {
        auto regloc = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        code += regbase->print();
        code += " -> ";
        code += regloc->print();
        if(i < num - 1) {
            code += ", ";
        }
        i++;
    }
    code += " ]";
    code += HAsm2Asm::newline;
    return code;
}

std::string LdTmpRegsInst::get_asm_code() {
    std::string asm_code;
    for(auto load_reg_pair: to_ld_tmp_regs_) {
        auto reg = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        if(reg->is_float()) {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::flw(new Reg(reg->get_reg_id(), true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        } else {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::ld(new Reg(reg->get_reg_id(), false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        }
    }

    return asm_code;
}

std::string LdTmpRegsInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    code += "[ ";
    int num = to_ld_tmp_regs_.size();
    int i = 0;
    for(auto load_reg_pair: to_ld_tmp_regs_) {
        auto regloc = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        code += regbase->print();
        code += " -> ";
        code += regloc->print();
        if(i < num - 1) {
            code += ", ";
        }
        i++;
    }
    code += " ]";
    code += HAsm2Asm::newline;
    return code;
}

std::string StoreTmpResultInst::get_asm_code() {
    std::string asm_code;
    for(auto store_reg_pair: to_store_regs_) {
        auto reg = store_reg_pair.first;
        auto regbase = store_reg_pair.second;
        if(reg->is_float()) {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::fsw(new Reg(reg->get_reg_id(), true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::fsw(new Reg(reg->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        } else {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::sd(new Reg(reg->get_reg_id(), false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::sd(new Reg(reg->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        }
    }

    return asm_code;
}

std::string StoreTmpResultInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    code += "[ ";
    int num = to_store_regs_.size();
    int i = 0;
    for(auto save_reg_pair: to_store_regs_) {
        auto regloc = save_reg_pair.first;
        auto regbase = save_reg_pair.second;
        code += regloc->print();
        code += " -> ";
        code += regbase->print();
        if(i < num - 1) {
            code += ", ";
        }
        i++;
    }
    code += " ]";
    code += HAsm2Asm::newline;
    return code;
}

std::string AllocTmpRegsWithSaveInitialOwnerInst::get_asm_code() {
    std::string asm_code;
    for(auto store_reg_pair: to_store_regs_) {
        auto reg = store_reg_pair.first;
        auto regbase = store_reg_pair.second;
        if(reg->is_float()) {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::fsw(new Reg(reg->get_reg_id(), true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::fsw(new Reg(reg->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        } else {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::sd(new Reg(reg->get_reg_id(), false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::sd(new Reg(reg->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        }
    }

    for(auto load_reg_pair: to_ld_regs_) {
        auto reg = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        if(reg->is_float()) {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::flw(new Reg(reg->get_reg_id(), true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        } else {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::ld(new Reg(reg->get_reg_id(), false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        }
    }

    return asm_code;
}

std::string AllocTmpRegsWithSaveInitialOwnerInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    int num, i;
    if(! to_store_regs_.empty()) {
        code += "[ ";
        int num = to_store_regs_.size();
        int i = 0;
        for(auto save_reg_pair: to_store_regs_) {
            auto regloc = save_reg_pair.first;
            auto regbase = save_reg_pair.second;
            code += regloc->print();
            code += " -> ";
            code += regbase->print();
            if(i < num - 1) {
                code += ", ";
            }
            i++;
        }
        code += " ]";
    }

    if(! to_store_regs_.empty() && ! to_ld_regs_.empty()) {
        code += ", ";
    }
    if(! to_ld_regs_.empty()) {
        code += "[ ";
        num = to_store_regs_.size();
        i = 0;
        for(auto load_reg_pair: to_ld_regs_) {
            auto regloc = load_reg_pair.first;
            auto regbase = load_reg_pair.second;
            code += regbase->print();
            code += " -> ";
            code += regloc->print();
            if(i < num - 1) {
                code += ", ";
            }
            i++;
        }
        code += "]";
    }
    
    code += HAsm2Asm::newline;
    return code;
}

std::string RestoreAllTmpRegsInst::get_asm_code() {
    std::string asm_code;
    for(auto load_reg_pair: to_restore_regs_) {
        auto reg = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        if(reg->is_float()) {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::flw(new Reg(reg->get_reg_id(), true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg->get_reg_id(), true), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        } else {
            if(regbase->get_offset() < -2048 || regbase->get_offset() > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
                asm_code += HAsm2Asm::ld(new Reg(reg->get_reg_id(), false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg->get_reg_id(), false), new Reg(regbase->get_reg_id(), false), regbase->get_offset());
            }
        }
    }

    return asm_code;
}

std::string RestoreAllTmpRegsInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    code += "[ ";
    int num = to_restore_regs_.size();
    int i = 0;
    for(auto load_reg_pair: to_restore_regs_) {
        auto regloc = load_reg_pair.first;
        auto regbase = load_reg_pair.second;
        code += regbase->print();
        code += " -> ";
        code += regloc->print();
        if(i < num - 1) {
            code += ", ";
        }
        i++;
    }
    code += " ]";
    code += HAsm2Asm::newline;
    return code;
}

std::string PhiDataMoveInst::get_asm_code() {
    std::string asm_code;
    for(auto move_floc_pair: to_move_flocs_) {
        auto target = move_floc_pair.first;
        auto src = move_floc_pair.second;
        auto const_src = dynamic_cast<ConstPool*>(src);
        auto reg_src = dynamic_cast<RegLoc*>(src);
        auto regbase_src = dynamic_cast<RegBase*>(src); 
        if(const_src) {
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_src->get_fval()));
                asm_code += HAsm2Asm::fmvsx(new Reg(reg_target->get_reg_id(), true), new Reg(reg_s1, false));
            } else if(dynamic_cast<RegBase*>(target)) {
                asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_src->get_fval()));
                asm_code += HAsm2Asm::fmvsx(new Reg(reg_target->get_reg_id(), true), new Reg(reg_s1, false));
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        } else if(reg_src) {
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::fmvs(new Reg(reg_target->get_reg_id(), true), new Reg(reg_src->get_reg_id(), true));
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::fsw(new Reg(reg_src->get_reg_id(), true), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::fsw(new Reg(reg_src->get_reg_id(), true), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        } else {
            if(regbase_src->get_offset() > 2047 || regbase_src->get_offset() < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_src->get_reg_id(), false), regbase_src->get_offset()); 
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(regbase_src->get_reg_id(), false), regbase_src->get_offset()); 
            }
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::fmvs(new Reg(reg_target->get_reg_id(), true), new Reg(reg_fs1, true));
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        }
    }

    for(auto move_iloc_pair: to_move_ilocs_) {
        auto target = move_iloc_pair.first;
        auto src = move_iloc_pair.second;
        auto const_src = dynamic_cast<ConstPool*>(src);
        auto reg_src = dynamic_cast<RegLoc*>(src);
        auto regbase_src = dynamic_cast<RegBase*>(src); 
        if(const_src) {
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::addi(new Reg(reg_target->get_reg_id(), false), new Reg(reg_zero, false), const_src->get_ival());
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(reg_zero, false), const_src->get_ival());
                    asm_code += HAsm2Asm::sd(new Reg(reg_s1, false), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(reg_zero, false), const_src->get_ival());
                    asm_code += HAsm2Asm::sd(new Reg(reg_s1, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        } else if(reg_src) {
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::mv(new Reg(reg_target->get_reg_id(), false), new Reg(reg_src->get_reg_id(), false));
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::sd(new Reg(reg_src->get_reg_id(), false), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::sd(new Reg(reg_src->get_reg_id(), false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        } else {
            if(regbase_src->get_offset() > 2047 || regbase_src->get_offset() < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_src->get_reg_id(), false), regbase_src->get_offset()); 
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(regbase_src->get_reg_id(), false), regbase_src->get_offset()); 
            }
            auto reg_target = dynamic_cast<RegLoc*>(target);
            auto regbase_target = dynamic_cast<RegBase*>(target);
            if(reg_target) {
                asm_code += HAsm2Asm::mv(new Reg(reg_target->get_reg_id(), false), new Reg(reg_ra, false));
            } else if(dynamic_cast<RegBase*>(target)) {
                if(regbase_target->get_offset() > 2047 || regbase_target->get_offset() < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                    asm_code += HAsm2Asm::sd(new Reg(reg_ra, false), new Reg(reg_s1, false), 0);
                } else {
                    asm_code += HAsm2Asm::sd(new Reg(reg_ra, false), new Reg(regbase_target->get_reg_id(), false), regbase_target->get_offset());
                }
            } 
        }
    } 

    return asm_code;
}

std::string PhiDataMoveInst::print() {
    std::string code;
    code += HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space;
    int num, i;
    if(! to_move_flocs_.empty()) {
        code += "[ ";
        num = to_move_flocs_.size();
        i = 0;
        for(auto floc_pair: to_move_flocs_) {
            auto target = floc_pair.first;
            auto src = floc_pair.second;
            code += src->print();
            code += " -> ";
            code += target->print();
            if(i < num - 1) {
                code += ", ";
            }
            i++;
        }
        code += " ]";
    }
    if(! to_move_flocs_.empty() && ! to_move_ilocs_.empty()) {
        code += ", ";
    }

    if(! to_move_ilocs_.empty()) {
        code += "[ ";
        num = to_move_ilocs_.size();
        i = 0;
        for(auto iloc_pair: to_move_ilocs_) {
            auto target = iloc_pair.first;
            auto src = iloc_pair.second;
            code += src->print();
            code += " -> ";
            code += target->print();
            if(i < num - 1) {
                code += ", ";
            }
            i++;
        }
        code += "]";
    }
    
    code += HAsm2Asm::newline;
    return code;
}

std::string CallerSaveCallResultInst::get_asm_code() {
    std::string asm_code;
    if(src_->is_float()) {
        auto regbase_dst = dynamic_cast<RegBase*>(dst_);
        auto regloc_dst = dynamic_cast<RegLoc*>(dst_);
        if(regbase_dst) {
            if(regbase_dst->get_offset() > 2047 || regbase_dst->get_offset() < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_dst->get_reg_id(), false), regbase_dst->get_offset());
                asm_code += HAsm2Asm::fsw(new Reg(reg_fa0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::fsw(new Reg(reg_fa0, true), new Reg(regbase_dst->get_reg_id(), false), regbase_dst->get_offset());
            }
        } else {
            asm_code += HAsm2Asm::fmvs(new Reg(regloc_dst->get_reg_id(), true), new Reg(reg_fa0, true));
        }
    } else { 
        auto regbase_dst = dynamic_cast<RegBase*>(dst_);
        auto regloc_dst = dynamic_cast<RegLoc*>(dst_);
        if(regbase_dst) {
            if(regbase_dst->get_offset() > 2047 || regbase_dst->get_offset() < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(regbase_dst->get_reg_id(), false), regbase_dst->get_offset());
                asm_code += HAsm2Asm::sd(new Reg(reg_a0, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::sd(new Reg(reg_a0, false), new Reg(regbase_dst->get_reg_id(), false), regbase_dst->get_offset());
            }
        } else {
            asm_code += HAsm2Asm::mv(new Reg(regloc_dst->get_reg_id(), false), new Reg(reg_a0, false));
        }
    }
    return asm_code;
}

std::string CallerSaveCallResultInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + src_->print() + HAsm2Asm::newline;
}

std::string CalleeSaveCallResultInst::get_asm_code() {
    std::string asm_code;
    if(dst_->is_float()) {
        auto const_src = dynamic_cast<Const*>(src_);
        auto reg_src = dynamic_cast<Reg*>(src_);
        auto mem_src = dynamic_cast<Mem*>(src_);
        if(const_src) {
            asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_src->get_fval()));
            asm_code += HAsm2Asm::fmvsx(new Reg(reg_fa0, true), new Reg(reg_s1, false));
        } else if(mem_src) {
            if(mem_src->get_offset() > 2047 || mem_src->get_offset() < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_src->get_reg_id(), false), mem_src->get_offset());
                asm_code += HAsm2Asm::flw(new Reg(reg_fa0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fa0, true), new Reg(mem_src->get_reg_id(), false), mem_src->get_offset());
            }
        } else {
            asm_code += HAsm2Asm::fmvs(new Reg(reg_fa0, true), reg_src);
        }
    } else {
        auto const_src = dynamic_cast<Const*>(src_);
        auto reg_src = dynamic_cast<Reg*>(src_);
        auto mem_src = dynamic_cast<Mem*>(src_);
        if(const_src) {
            asm_code += HAsm2Asm::addi(new Reg(reg_a0, false), new Reg(reg_zero, false), const_src->get_ival());
        } else if(mem_src) {
            if(mem_src->get_offset() > 2047 || mem_src->get_offset() < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_src->get_reg_id(), false), mem_src->get_offset());
                asm_code += HAsm2Asm::ld(new Reg(reg_a0, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_a0, false), new Reg(mem_src->get_reg_id(), false), mem_src->get_offset());
            }
        } else {
            asm_code += HAsm2Asm::mv(new Reg(reg_a0, false), reg_src);
        }
    }
    return asm_code;
}

std::string CalleeSaveCallResultInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + src_->print() + HAsm2Asm::newline;
}

std::string ExpandStackSpaceInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::addi(new Reg(reg_sp, false), new Reg(reg_sp, false), expand_size_);
    return asm_code;
}

std::string ExpandStackSpaceInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + std::to_string(-expand_size_) + HAsm2Asm::newline;
}

std::string ShrinkStackSpaceInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::addi(new Reg(reg_sp, false), new Reg(reg_sp, false), shrink_size_);
    return asm_code;
}

std::string ShrinkStackSpaceInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + std::to_string(shrink_size_) + HAsm2Asm::newline;
}

std::string HAsmAddInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival() + const_rs2->get_ival());
    } else if(const_rs1) {
        asm_code += HAsm2Asm::addi(dst_, dynamic_cast<Reg*>(rs2_), const_rs1->get_ival());
    } else if(const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, dynamic_cast<Reg*>(rs1_), const_rs2->get_ival());
    } else {
        asm_code += HAsm2Asm::add(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmAddInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmSubwInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival() - const_rs2->get_ival());
    } else if(const_rs1) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival());
        asm_code += HAsm2Asm::subw(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, dynamic_cast<Reg*>(rs1_), - const_rs2->get_ival());
    } else {
        asm_code += HAsm2Asm::subw(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmSubwInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmMulwInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival() * const_rs2->get_ival());
    } else if(const_rs1) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival());
        asm_code += HAsm2Asm::mulw(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs2->get_ival());
        asm_code += HAsm2Asm::mulw(dst_, dynamic_cast<Reg*>(rs1_), dst_);
    } else {
        asm_code += HAsm2Asm::mulw(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmMulwInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmMul64Inst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        
        //LOG(ERROR) << "无法处理";
    } else if(const_rs1) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival());
        asm_code += HAsm2Asm::mul(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs2->get_ival());
        asm_code += HAsm2Asm::mul(dst_, dynamic_cast<Reg*>(rs1_), dst_);
    } else {
        asm_code += HAsm2Asm::mul(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmMul64Inst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmDivwInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival() / const_rs2->get_ival());
    } else if(const_rs1) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival());
        asm_code += HAsm2Asm::divw(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs2->get_ival());
        asm_code += HAsm2Asm::divw(dst_, dynamic_cast<Reg*>(rs1_), dst_);
    } else {
        asm_code += HAsm2Asm::divw(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmDivwInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmRemwInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival() % const_rs2->get_ival());
    } else if(const_rs1) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs1->get_ival());
        asm_code += HAsm2Asm::remw(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), const_rs2->get_ival());
        asm_code += HAsm2Asm::remw(dst_, dynamic_cast<Reg*>(rs1_), dst_);
    } else {
        asm_code += HAsm2Asm::remw(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmRemwInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmSrawInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs2 == nullptr){}
        //LOG(ERROR) << "出现未预期情况";
    if(const_rs1)
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), (const_rs1->get_ival()) >> (const_rs2->get_ival()));
    else 
        asm_code += HAsm2Asm::sraiw(dst_, dynamic_cast<Reg*>(rs1_), const_rs2->get_ival());

    return asm_code;
}

std::string HAsmSrawInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmSllwInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs2 == nullptr);
       // LOG(ERROR) << "出现未预期情况";
    if(const_rs1)
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), (const_rs1->get_ival()) << (const_rs2->get_ival()));
    else 
        asm_code += HAsm2Asm::slliw(dst_, dynamic_cast<Reg*>(rs1_), const_rs2->get_ival());

    return asm_code;
}

std::string HAsmSllwInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmSrlwInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs2 == nullptr);
       // LOG(ERROR) << "出现未预期情况";
    if(const_rs1)
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), (const_rs1->get_ival()) >> (const_rs2->get_ival()));
    else 
        asm_code += HAsm2Asm::srliw(dst_, dynamic_cast<Reg*>(rs1_), const_rs2->get_ival());

    return asm_code;
}

std::string HAsmSrlwInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmSraInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs2 == nullptr);
      //  LOG(ERROR) << "出现未预期情况";
    if(const_rs1)
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), (const_rs1->get_ival()) >> (const_rs2->get_ival()));
    else 
        asm_code += HAsm2Asm::srai(dst_, dynamic_cast<Reg*>(rs1_), const_rs2->get_ival());

    return asm_code;
}

std::string HAsmSraInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmSllInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs2 == nullptr);
       // LOG(ERROR) << "出现未预期情况";
    if(const_rs1)
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), (const_rs1->get_ival()) << (const_rs2->get_ival()));
    else 
        asm_code += HAsm2Asm::slli(dst_, dynamic_cast<Reg*>(rs1_), const_rs2->get_ival());

    return asm_code;
}

std::string HAsmSllInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmSrlInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs2 == nullptr);
       // LOG(ERROR) << "出现未预期情况";
    if(const_rs1)
        asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), (const_rs1->get_ival()) >> (const_rs2->get_ival()));
    else 
        asm_code += HAsm2Asm::srli(dst_, dynamic_cast<Reg*>(rs1_), const_rs2->get_ival());

    return asm_code;
}

std::string HAsmSrlInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmLandInst::get_asm_code() {
    std::string asm_code;
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs2 == nullptr);
        //LOG(ERROR) << "出现未预期情况";
    if(const_rs2->get_ival() > 2047 || const_rs2->get_ival() < -2048) {
        asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(reg_zero, false), const_rs2->get_ival());
        asm_code += HAsm2Asm::land(dst_, dynamic_cast<Reg*>(rs1_), new Reg(reg_ra, false));
    } else {
        asm_code += HAsm2Asm::andi(dst_, dynamic_cast<Reg*>(rs1_), const_rs2->get_ival());
    }   

    return asm_code;
}

std::string HAsmLandInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmFaddsInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        auto result = const_rs1->get_fval() + const_rs2->get_fval();
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&result));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
    } else if(const_rs1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_rs1->get_fval())));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fadds(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_rs2->get_fval())));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fadds(dst_, dynamic_cast<Reg*>(rs1_), dst_);
    } else {
        asm_code += HAsm2Asm::fadds(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmFaddsInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmFsubsInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        auto result = const_rs1->get_fval() + const_rs2->get_fval();
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&result));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
    } else if(const_rs1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_rs1->get_fval())));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fsubs(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_rs2->get_fval())));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fsubs(dst_, dynamic_cast<Reg*>(rs1_), dst_);
    } else {
        asm_code += HAsm2Asm::fsubs(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmFsubsInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmFmulsInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        auto result = const_rs1->get_fval() * const_rs2->get_fval();
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&result));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
    } else if(const_rs1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_rs1->get_fval())));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fmuls(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_rs2->get_fval())));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fmuls(dst_, dynamic_cast<Reg*>(rs1_), dst_);
    } else {
        asm_code += HAsm2Asm::fmuls(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmFmulsInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmFdivsInst::get_asm_code() {
    std::string asm_code;
    auto const_rs1 = dynamic_cast<Const*>(rs1_);
    auto const_rs2 = dynamic_cast<Const*>(rs2_);
    if(const_rs1 && const_rs2) {
        auto result = const_rs1->get_fval() / const_rs2->get_fval();
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&result));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
    } else if(const_rs1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_rs1->get_fval())));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fdivs(dst_, dst_, dynamic_cast<Reg*>(rs2_));
    } else if(const_rs2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_rs2->get_fval())));
        asm_code += HAsm2Asm::fmvsx(dst_, new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fdivs(dst_, dynamic_cast<Reg*>(rs1_), dst_);
    } else {
        asm_code += HAsm2Asm::fdivs(dst_, dynamic_cast<Reg*>(rs1_), dynamic_cast<Reg*>(rs2_));
    }
    return asm_code;
}

std::string HAsmFdivsInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + rs1_->print() + ", " + rs2_->print()+ HAsm2Asm::newline;
}

std::string HAsmFcvtwsInst::get_asm_code() {
    std::string asm_code;
    auto const_src = dynamic_cast<Const*>(src_);
    if(const_src) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)(&(const_src->get_fval())));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fcvtws(dst_, new Reg(reg_fs1, true));
    } else {
        asm_code += HAsm2Asm::fcvtws(dst_, dynamic_cast<Reg*>(src_));
    }
    
    return asm_code;
}

std::string HAsmFcvtwsInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + src_->print() + HAsm2Asm::newline;
}

std::string HAsmFcvtswInst::get_asm_code() {
    std::string asm_code;
    auto const_src = dynamic_cast<Const*>(src_);
    if(const_src) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_src->get_ival());
        asm_code += HAsm2Asm::fcvtsw(dst_, new Reg(reg_ra, false));
    } else {
        asm_code += HAsm2Asm::fcvtsw(dst_, dynamic_cast<Reg*>(src_));
    }
    
    return asm_code;
}

std::string HAsmFcvtswInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + src_->print() + HAsm2Asm::newline;
}

std::string HAsmZextInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::andi(dst_, src_, 1);
    return asm_code;
}

std::string HAsmZextInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + src_->print() + HAsm2Asm::newline;
}

std::string HAsmSnezInst::get_asm_code() {
    std::string asm_code;
    auto const_cond = dynamic_cast<Const*>(cond_);
    if(const_cond) {
        if(const_cond->get_ival() != 0) {
            asm_code += HAsm2Asm::seqz(dst_, new Reg(reg_zero, false));
        } else {
            asm_code += HAsm2Asm::snez(dst_, new Reg(reg_zero, false));
        }   
    } else {
        asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), dynamic_cast<Reg*>(cond_));
        asm_code += HAsm2Asm::snez(dst_, new Reg(reg_ra, false));
    }   
    return asm_code;
}

std::string HAsmSnezInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + cond_->print() + HAsm2Asm::newline;
}

std::string HAsmSeqzInst::get_asm_code() {
    std::string asm_code;
    auto const_cond = dynamic_cast<Const*>(cond_);
    if(const_cond) {
        if(const_cond->get_ival() == 0) {
            asm_code += HAsm2Asm::seqz(dst_, new Reg(reg_zero, false));
        } else {
            asm_code += HAsm2Asm::snez(dst_, new Reg(reg_zero, false));
        }   
    } else {
        asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), dynamic_cast<Reg*>(cond_));
        asm_code += HAsm2Asm::seqz(dst_, new Reg(reg_ra, false));
    }   
    return asm_code;
}

std::string HAsmSeqzInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + cond_->print() + HAsm2Asm::newline;
}

std::string HAsmFeqsInst::get_asm_code() {
    std::string asm_code;
    auto const_cond1 = dynamic_cast<Const*>(cond1_);
    auto const_cond2 = dynamic_cast<Const*>(cond2_);
    if(const_cond1 && const_cond2) {
        if(const_cond1->get_fval() == const_cond2->get_fval()) {
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 1);
        } else {    
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 0);
        }
    } else if(const_cond1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond1->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::feqs(dst_, new Reg(reg_fs1, true), dynamic_cast<Reg*>(cond2_));
    } else if(const_cond2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond2->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::feqs(dst_, dynamic_cast<Reg*>(cond1_), new Reg(reg_fs1, true));
    } else {
        asm_code += HAsm2Asm::feqs(dst_, dynamic_cast<Reg*>(cond1_), dynamic_cast<Reg*>(cond2_));
    }
    return asm_code; 
}

std::string HAsmFeqsInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + cond1_->print() + ", " + cond2_->print() + HAsm2Asm::newline;
}

std::string HAsmFlesInst::get_asm_code() {
    std::string asm_code;
    auto const_cond1 = dynamic_cast<Const*>(cond1_);
    auto const_cond2 = dynamic_cast<Const*>(cond2_);
    if(const_cond1 && const_cond2) {
        if(const_cond1->get_fval() <= const_cond2->get_fval()) {
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 1);
        } else {    
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 0);
        }
    } else if(const_cond1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond1->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fles(dst_, new Reg(reg_fs1, true), dynamic_cast<Reg*>(cond2_));
    } else if(const_cond2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond2->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fles(dst_, dynamic_cast<Reg*>(cond1_), new Reg(reg_fs1, true));
    } else {
        asm_code += HAsm2Asm::fles(dst_, dynamic_cast<Reg*>(cond1_), dynamic_cast<Reg*>(cond2_));
    }
    return asm_code;
}

std::string HAsmFlesInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + cond1_->print() + ", " + cond2_->print() + HAsm2Asm::newline;
}

std::string HAsmFltsInst::get_asm_code() {
    std::string asm_code;
    auto const_cond1 = dynamic_cast<Const*>(cond1_);
    auto const_cond2 = dynamic_cast<Const*>(cond2_);
    if(const_cond1 && const_cond2) {
        if(const_cond1->get_fval() < const_cond2->get_fval()) {
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 1);
        } else {    
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 0);
        }
    } else if(const_cond1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond1->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::flts(dst_, new Reg(reg_fs1, true), dynamic_cast<Reg*>(cond2_));
    } else if(const_cond2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond2->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::flts(dst_, dynamic_cast<Reg*>(cond1_), new Reg(reg_fs1, true));
    } else {
        asm_code += HAsm2Asm::flts(dst_, dynamic_cast<Reg*>(cond1_), dynamic_cast<Reg*>(cond2_));
    }
    return asm_code;
}

std::string HAsmFltsInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + cond1_->print() + ", " + cond2_->print() + HAsm2Asm::newline;
}

std::string HAsmFgesInst::get_asm_code() {
    std::string asm_code;
    auto const_cond1 = dynamic_cast<Const*>(cond1_);
    auto const_cond2 = dynamic_cast<Const*>(cond2_);
    if(const_cond1 && const_cond2) {
        if(const_cond1->get_fval() >= const_cond2->get_fval()) {
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 1);
        } else {    
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 0);
        }
    } else if(const_cond1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond1->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fles(dst_, dynamic_cast<Reg*>(cond2_), new Reg(reg_fs1, true));
    } else if(const_cond2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond2->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::fles(dst_, new Reg(reg_fs1, true), dynamic_cast<Reg*>(cond1_));
    } else {
        asm_code += HAsm2Asm::fles(dst_, dynamic_cast<Reg*>(cond2_), dynamic_cast<Reg*>(cond1_));
    }
    return asm_code;
}

std::string HAsmFgesInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + cond1_->print() + ", " + cond2_->print() + HAsm2Asm::newline;
}

std::string HAsmFgtsInst::get_asm_code() {
    std::string asm_code;
    auto const_cond1 = dynamic_cast<Const*>(cond1_);
    auto const_cond2 = dynamic_cast<Const*>(cond2_);
    if(const_cond1 && const_cond2) {
        if(const_cond1->get_fval() > const_cond2->get_fval()) {
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 1);
        } else {    
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 0);
        }
    } else if(const_cond1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond1->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::flts(dst_, dynamic_cast<Reg*>(cond2_), new Reg(reg_fs1, true));
    } else if(const_cond2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond2->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::flts(dst_, new Reg(reg_fs1, true), dynamic_cast<Reg*>(cond1_));
    } else {
        asm_code += HAsm2Asm::flts(dst_, dynamic_cast<Reg*>(cond2_), dynamic_cast<Reg*>(cond1_));
    }
    return asm_code;
}

std::string HAsmFgtsInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + cond1_->print() + ", " + cond2_->print() + HAsm2Asm::newline;
}

std::string HAsmFnesInst::get_asm_code() {
    std::string asm_code;
    auto const_cond1 = dynamic_cast<Const*>(cond1_);
    auto const_cond2 = dynamic_cast<Const*>(cond2_);
    if(const_cond1 && const_cond2) {
        if(const_cond1->get_fval() != const_cond2->get_fval()) {
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 1);
        } else {    
            asm_code += HAsm2Asm::addi(dst_, new Reg(reg_zero, false), 0);
        }
    } else if(const_cond1) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond1->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(cond2_));
        asm_code += HAsm2Asm::seqz(dst_, new Reg(reg_s1, false));
    } else if(const_cond2) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_cond2->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        asm_code += HAsm2Asm::feqs(dst_, dynamic_cast<Reg*>(cond1_), new Reg(reg_fs1, true));
        asm_code += HAsm2Asm::seqz(dst_, new Reg(reg_s1, false));
    } else {
        asm_code += HAsm2Asm::feqs(dst_, dynamic_cast<Reg*>(cond1_), dynamic_cast<Reg*>(cond2_));
        asm_code += HAsm2Asm::seqz(dst_, new Reg(reg_s1, false));
    }
    return asm_code;
}

std::string HAsmFnesInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + cond1_->print() + ", " + cond2_->print() + HAsm2Asm::newline;
}

std::string HAsmLwInst::get_asm_code() {
    std::string asm_code;
    if(is_label_) {
        asm_code += HAsm2Asm::la(new Reg(reg_ra, false), label_);
        asm_code += HAsm2Asm::lw(dst_, new Reg(reg_ra, false), 0);
    } else {
        auto const_offset = dynamic_cast<Const*>(offset_);
        if(const_offset) {
            int offset_val = const_offset->get_ival() * 4;
            if(offset_val > 2047 || offset_val < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), base_, offset_val);
                asm_code += HAsm2Asm::lw(dst_, new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::lw(dst_, base_, offset_val);
            }
        } else {
            asm_code += HAsm2Asm::slliw(new Reg(reg_ra, false), dynamic_cast<Reg*>(offset_), 2);
            asm_code += HAsm2Asm::add(new Reg(reg_ra, false), base_, new Reg(reg_ra, false));
            asm_code += HAsm2Asm::lw(dst_, new Reg(reg_ra, false), 0);
        }
    }
    return asm_code;
}

std::string HAsmLwInst::print() {
    if(is_label_) {
        return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + label_->print() + HAsm2Asm::newline;
    } else {
        return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + "[" + base_->print() + ", " + offset_->print() + "]" + HAsm2Asm::newline;
    }
}

std::string HAsmFlwInst::get_asm_code() {
    std::string asm_code;
    if(is_label_) {
        asm_code += HAsm2Asm::la(new Reg(reg_ra, false), label_);
        asm_code += HAsm2Asm::flw(dst_, new Reg(reg_ra, false), 0);
    } else {
        auto const_offset = dynamic_cast<Const*>(offset_);
        if(const_offset) {
            int offset_val = const_offset->get_ival() * 4;
            if(offset_val > 2047 || offset_val < -2048) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), base_, offset_val);
                asm_code += HAsm2Asm::flw(dst_, new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(dst_, base_, offset_val);
            }
        } else {
            asm_code += HAsm2Asm::slliw(new Reg(reg_ra, false), dynamic_cast<Reg*>(offset_), 2);
            asm_code += HAsm2Asm::add(new Reg(reg_ra, false), base_, new Reg(reg_ra, false));
            asm_code += HAsm2Asm::flw(dst_, new Reg(reg_ra, false), 0);
        }
    }
    return asm_code;
}

std::string HAsmFlwInst::print() {
    if(is_label_) {
        return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + label_->print() + HAsm2Asm::newline;
    } else {
        return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + "[" + base_->print() + ", " + offset_->print() + "]" + HAsm2Asm::newline;
    }
}

std::string HAsmSwInst::get_asm_code() {
    std::string asm_code;
    if(is_label_) {
        asm_code += HAsm2Asm::la(new Reg(reg_ra, false), label_);
        if(dynamic_cast<Const*>(val_)) {
            asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(reg_zero, false), dynamic_cast<Const*>(val_)->get_ival());
            asm_code += HAsm2Asm::sw(new Reg(reg_s1, false), new Reg(reg_ra, false), 0);
        } else {
            asm_code += HAsm2Asm::sw(dynamic_cast<Reg*>(val_), new Reg(reg_ra, false), 0);
        }
    } else {
        if(dynamic_cast<Const*>(val_)) {
            if(dynamic_cast<Const*>(offset_)) {
                int offset_value = dynamic_cast<Const*>(offset_)->get_ival() * 4;
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(reg_zero, false), dynamic_cast<Const*>(val_)->get_ival());
                if(offset_value > 2047 || offset_value < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), base_, offset_value);
                    asm_code += HAsm2Asm::sw(new Reg(reg_ra, false), new Reg(reg_s1, false), 0);
                } else {
                    asm_code += HAsm2Asm::sw(new Reg(reg_ra, false), base_, offset_value);
                }
            } else {
                asm_code += HAsm2Asm::slliw(new Reg(reg_ra, false), dynamic_cast<Reg*>(offset_), 2);
                asm_code += HAsm2Asm::add(new Reg(reg_ra, false), new Reg(reg_ra, false), base_);
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(reg_zero, false), dynamic_cast<Const*>(val_)->get_ival());
                asm_code += HAsm2Asm::sw(new Reg(reg_s1, false), new Reg(reg_ra, false), 0);
            }
        } else {
            if(dynamic_cast<Const*>(offset_)) {
                int offset_value = dynamic_cast<Const*>(offset_)->get_ival() * 4;
                if(offset_value > 2047 || offset_value < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), base_, offset_value);
                    asm_code += HAsm2Asm::sw(dynamic_cast<Reg*>(val_), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::sw(dynamic_cast<Reg*>(val_), base_, offset_value);
                }
            } else {
                asm_code += HAsm2Asm::slliw(new Reg(reg_ra, false), dynamic_cast<Reg*>(offset_), 2);
                asm_code += HAsm2Asm::add(new Reg(reg_ra, false), new Reg(reg_ra, false), base_);
                asm_code += HAsm2Asm::sw(dynamic_cast<Reg*>(val_), new Reg(reg_ra, false), 0);
            }
        }
    }
    return asm_code;
}

std::string HAsmSwInst::print() {
    if(is_label_) {
        return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + val_->print() + ", " + label_->print() + HAsm2Asm::newline;
    } else {
        return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + val_->print() + ", " + "[" + base_->print() + ", " + offset_->print() + "]" + HAsm2Asm::newline;
    }
}

std::string HAsmFswInst::get_asm_code() {
    std::string asm_code;
    if(is_label_) {
        asm_code += HAsm2Asm::la(new Reg(reg_ra, false), label_);
        if(dynamic_cast<Const*>(val_)) {
            asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(dynamic_cast<Const*>(val_)->get_fval()));
            asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
        } else {
            asm_code += HAsm2Asm::fsw(dynamic_cast<Reg*>(val_), new Reg(reg_ra, false), 0);
        }
    } else {
        if(dynamic_cast<Const*>(val_)) {
            asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(dynamic_cast<Const*>(val_)->get_fval()));
            asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
            if(dynamic_cast<Const*>(offset_)) {
                int offset_value = dynamic_cast<Const*>(offset_)->get_ival() * 4;
                if(offset_value > 2047 || offset_value < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), base_, offset_value);
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), base_, offset_value);
                }
            } else {
                asm_code += HAsm2Asm::slliw(new Reg(reg_ra, false), dynamic_cast<Reg*>(offset_), 2);
                asm_code += HAsm2Asm::add(new Reg(reg_ra, false), new Reg(reg_ra, false), base_);
                asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            }
        } else {
            if(dynamic_cast<Const*>(offset_)) {
                int offset_value = dynamic_cast<Const*>(offset_)->get_ival() * 4;
                if(offset_value > 2047 || offset_value < -2048) {
                    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), base_, offset_value);
                    asm_code += HAsm2Asm::fsw(dynamic_cast<Reg*>(val_), new Reg(reg_ra, false), 0);
                } else {
                    asm_code += HAsm2Asm::fsw(dynamic_cast<Reg*>(val_), base_, offset_value);
                }
            } else {
                asm_code += HAsm2Asm::slliw(new Reg(reg_ra, false), dynamic_cast<Reg*>(offset_), 2);
                asm_code += HAsm2Asm::add(new Reg(reg_ra, false), new Reg(reg_ra, false), base_);
                asm_code += HAsm2Asm::fsw(dynamic_cast<Reg*>(val_), new Reg(reg_ra, false), 0);
            }
        }
    }
    return asm_code;
}

std::string HAsmFswInst::print() {
    if(is_label_) {
        return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + val_->print() + ", " + label_->print() + HAsm2Asm::newline;
    } else {
        return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + val_->print() + ", " + "[" + base_->print() + ", " + offset_->print() + "]" + HAsm2Asm::newline;
    }
}

std::string HAsmMemsetInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(base_->get_reg_id(), false), base_->get_offset());
    if(is_fp_) {
        int offset = 0;
        int past_size = 0;
        asm_code += HAsm2Asm::fcvtsw(new Reg(reg_fs1, true), new Reg(reg_zero, false));
        while(past_size < total_size_) {
            if(offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(reg_ra, false), offset-step_size_);
                offset = step_size_;
            }
            asm_code += HAsm2Asm::fsw(new Reg(reg_fs1, true), new Reg(reg_ra, false), offset);
            offset += step_size_;
            past_size += step_size_;
        }
    } else {
        int offset = 0;
        int past_size = 0;
        while(past_size < total_size_) {
            if(offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(reg_ra, false), offset-step_size_);
                offset = step_size_;
            }
            asm_code += HAsm2Asm::sw(new Reg(reg_zero, false), new Reg(reg_ra, false), offset);
            offset += step_size_;
            past_size += step_size_;
        }
    }   
    return asm_code;
}

std::string HAsmMemsetInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + base_->print() + ", " +  std::to_string(total_size_) + HAsm2Asm::newline;
}

std::string HAsmCallInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::call(label_);
    return asm_code;
}

std::string HAsmCallInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + label_->print() + HAsm2Asm::newline;
}

std::string HAsmLaInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::la(dst_, label_);
    return asm_code;
}

std::string HAsmLaInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmMvInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::mv(dst_, src_);
    return asm_code;
}

std::string HAsmMvInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + dst_->print() + ", " + src_->print() + HAsm2Asm::newline;
}

std::string HAsmBeqInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_ival() == const_rval->get_ival()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_lval->get_ival());
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::beq(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::beq(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        }
    } else if(const_rval) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_rval->get_ival());
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::beq(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::beq(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        }
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::beq(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::beq(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::beq(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::beq(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        }
    }
    return asm_code;
}

std::string HAsmBeqInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmBneInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_ival() != const_rval->get_ival()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_lval->get_ival());
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::bne(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::bne(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        }
    } else if(const_rval) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_rval->get_ival());
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::bne(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::bne(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        }
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::bne(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::bne(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::bne(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::bne(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        }
    }
    return asm_code;
}

std::string HAsmBneInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmBgeInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_ival() >= const_rval->get_ival()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_lval->get_ival());
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::bge(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::bge(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        }
    } else if(const_rval) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_rval->get_ival());
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::bge(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::bge(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        }
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::bge(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::bge(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::bge(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::bge(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        }
    }
    return asm_code;
}

std::string HAsmBgeInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmBltInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_ival() < const_rval->get_ival()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_lval->get_ival());
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::blt(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::blt(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        }
    } else if(const_rval) {
        asm_code += HAsm2Asm::addiw(new Reg(reg_ra, false), new Reg(reg_zero, false), const_rval->get_ival());
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::blt(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::blt(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        }
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(reg_s1, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_s1, false), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), new Reg(reg_s1, false));
            asm_code += HAsm2Asm::blt(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::blt(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::ld(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), new Reg(reg_ra, false));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::blt(new Reg(reg_s1, false), new Reg(reg_ra, false), label_);
        } else {
            asm_code += HAsm2Asm::sextw(new Reg(reg_ra, false), dynamic_cast<Reg*>(lval_));
            asm_code += HAsm2Asm::sextw(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_));
            asm_code += HAsm2Asm::blt(new Reg(reg_ra, false), new Reg(reg_s1, false), label_);
        }
    }
    return asm_code;
}

std::string HAsmBltInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmFBeqInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_fval() == const_rval->get_fval()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_lval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else {
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(rval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else if(const_rval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_rval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs0, true), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), new Reg(reg_fs1, true));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(rval_));
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), dynamic_cast<Reg*>(rval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    }

    return asm_code;
}

std::string HAsmFBeqInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}


std::string HAsmFBgeInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_fval() >= const_rval->get_fval()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_lval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs0, true), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_), new Reg(reg_fs1, true));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else if(const_rval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_rval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else {
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(lval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs0, true), new Reg(reg_fs1, true));
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_), new Reg(reg_fs1, true));
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(lval_));
        } else {
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_), dynamic_cast<Reg*>(lval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    }

    return asm_code;
}

std::string HAsmFBgeInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmFBgtInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_fval() > const_rval->get_fval()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_lval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs0, true), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_), new Reg(reg_fs1, true));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else if(const_rval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_rval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else {
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(lval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs0, true), new Reg(reg_fs1, true));
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_), new Reg(reg_fs1, true));
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(lval_));
        } else {
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), dynamic_cast<Reg*>(rval_), dynamic_cast<Reg*>(lval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    }

    return asm_code;
}

std::string HAsmFBgtInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmFBleInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_fval() <= const_rval->get_fval()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_lval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else {
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(rval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else if(const_rval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_rval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs0, true), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), new Reg(reg_fs1, true));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(rval_));
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::fles(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), dynamic_cast<Reg*>(rval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    }

    return asm_code;
}

std::string HAsmFBleInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmFBltInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_fval() < const_rval->get_fval()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_lval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else {
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(rval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else if(const_rval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_rval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs0, true), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), new Reg(reg_fs1, true));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(rval_));
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::flts(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), dynamic_cast<Reg*>(rval_));
        }
        asm_code += HAsm2Asm::blt(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    }

    return asm_code;
}
std::string HAsmFBltInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}


std::string HAsmFBneInst::get_asm_code() {
    std::string asm_code;
    auto const_lval = dynamic_cast<Const*>(lval_);
    auto const_rval = dynamic_cast<Const*>(rval_);
    if(const_lval && const_rval) {
        if(const_lval->get_fval() != const_rval->get_fval()) {
            asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        } else {
            asm_code += HAsm2Asm::bne(new Reg(reg_zero, false), new Reg(reg_zero, false), label_);
        }
    } else if(const_lval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_lval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else {
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(rval_));
        }
        asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else if(const_rval) {
        asm_code += HAsm2Asm::li(new Reg(reg_s1, false), *(uint32_t*)&(const_rval->get_fval()));
        asm_code += HAsm2Asm::fmvsx(new Reg(reg_fs1, true), new Reg(reg_s1, false));
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs0, true), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), new Reg(reg_fs1, true));
        }
        asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    } else {
        auto mem_lval = dynamic_cast<Mem*>(lval_);
        auto mem_rval = dynamic_cast<Mem*>(rval_);
        if(mem_lval && mem_rval) {
            int offset1 = mem_lval->get_offset();
            int offset2 = mem_rval->get_offset();
            if(offset1 < -2048 || offset1 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset1);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset1);
            }
            if(offset2 < -2048 || offset2 > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset2);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs0, true), new Reg(mem_rval->get_reg_id(), false), offset2);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), new Reg(reg_fs0, true));
        } else if(mem_lval) {
            int offset = mem_lval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_lval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_lval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), new Reg(reg_fs1, true), dynamic_cast<Reg*>(rval_));
        } else if(mem_rval) {
            int offset = mem_rval->get_offset();
            if(offset < -2048 || offset > 2047) {
                asm_code += HAsm2Asm::addi(new Reg(reg_ra, false), new Reg(mem_rval->get_reg_id(), false), offset);
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(reg_ra, false), 0);
            } else {
                asm_code += HAsm2Asm::flw(new Reg(reg_fs1, true), new Reg(mem_rval->get_reg_id(), false), offset);
            }
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), new Reg(reg_fs1, true));
        } else {
            asm_code += HAsm2Asm::feqs(new Reg(reg_s1, false), dynamic_cast<Reg*>(lval_), dynamic_cast<Reg*>(rval_));
        }
        asm_code += HAsm2Asm::beq(new Reg(reg_zero, false), new Reg(reg_s1, false), label_);
    }

    return asm_code;
}

std::string HAsmFBneInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + lval_->print() + ", " + rval_->print() + ", " + label_->print() + HAsm2Asm::newline;
}

std::string HAsmJInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::j(label_);
    return asm_code;
}

std::string HAsmJInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::space + label_->print() + HAsm2Asm::newline;
}

std::string HAsmRetInst::get_asm_code() {
    std::string asm_code;
    asm_code += HAsm2Asm::ret();
    return asm_code;
}

std::string HAsmRetInst::print() {
    return HAsm2Asm::space + get_instr_op_name(get_hasminst_op()) + HAsm2Asm::newline;
}