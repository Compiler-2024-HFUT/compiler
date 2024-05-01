#include <iostream>
#include <vector>

#include "backend/CodeGen.hpp"
#include "backend/HAsm2Asm.hpp"

#include "backend/HAsmLoc.hpp"
#include "midend/Instruction.hpp"

//#include "logging.hpp"

#define ENABLE_REGALLOC_DEBUG

void CodeGen::module_gen() {
    std::string asm_header;
    globals_def_gen();
    auto regalloc_driver = new RegAllocDriver(m_->get_module());
    regalloc_driver->compute_reg_alloc();
    
    asm_header += HAsm2Asm::space +".file" + HAsm2Asm::space + + "\"" + m_->get_module()->getFileName() + "\"" + HAsm2Asm::newline;
    asm_header += HAsm2Asm::space + ".option" + HAsm2Asm::space + "pic" + HAsm2Asm::newline;
    m_->set_hasm_header(asm_header);

    for(auto func: m_->get_module()->getFunctions()) {
        if(func->isDeclaration()) 
            continue;
        ival2interval = regalloc_driver->get_ireg_alloc_in_func(func);
        fval2interval = regalloc_driver->get_freg_alloc_in_func(func);
        cur_func_ = m_->create_func(func);
        function_gen();
    }

}

void CodeGen::globals_def_gen() {
    for(auto var: m_->get_module()->getGlobalVariables()) {
        std::string global_var_def;
        std::string name = var->getName();
        bool is_const = var->isConst();
        auto initializer = var->getInit();
        bool is_initialied = (dynamic_cast<ConstantZero*>(initializer) == nullptr);
        auto const_array = dynamic_cast<ConstantArray*>(initializer);
        bool is_array = var->getType()->getPointerElementType()->isArrayType();
        int size = var->getType()->getPointerElementType()->getSize();
        global_variable_labels_table[var] = new Label(var->getName());
        if(is_initialied) {
            if(is_array) {
                global_var_def += HAsm2Asm::space + ".globl" + HAsm2Asm::space + name + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".data" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".align" + HAsm2Asm::space + std::to_string(var_align) + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".type" + HAsm2Asm::space + name + ", @object" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".size" + HAsm2Asm::space + name + ", " +std::to_string(size) + HAsm2Asm::newline;
                global_var_def += name + ":" + HAsm2Asm::newline;
                if(const_array->getType()->getArrayElementType()->isFloatType()) {
                    for(int i = 0; i < const_array->getSizeOfArray(); i++) {
                        global_var_def += HAsm2Asm::space + ".word" + HAsm2Asm::space + std::to_string(*(uint32_t*)&((static_cast<ConstantFP*>(const_array->getElementValue(i)))->getValue())) + HAsm2Asm::newline;
                    }
                } else {
                    for(int i = 0; i < const_array->getSizeOfArray(); i++) {
                        global_var_def += HAsm2Asm::space + ".word" + HAsm2Asm::space + std::to_string((static_cast<ConstantInt*>(const_array->getElementValue(i)))->getValue()) + HAsm2Asm::newline;
                    }
                }
            } else {
                global_var_def += HAsm2Asm::space + ".globl" + HAsm2Asm::space + name + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".section" + HAsm2Asm::space + ".sdata, \"aw\"" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".align" + HAsm2Asm::space + std::to_string(var_align) + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".type" + HAsm2Asm::space + name + ", @object" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".size" + HAsm2Asm::space + name + ", " + std::to_string(size) + HAsm2Asm::newline;
                global_var_def += name + ":" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".word";
                if(initializer->getType()->isFloatType()) {
                    global_var_def += HAsm2Asm::space + std::to_string(*(uint32_t*)&(static_cast<ConstantFP*>(initializer)->getValue())) + HAsm2Asm::newline;
                } else {
                    global_var_def += HAsm2Asm::space + std::to_string(static_cast<ConstantInt*>(initializer)->getValue()) + HAsm2Asm::newline;
                }
            }
        } else {
            if(is_array) {
                global_var_def += HAsm2Asm::space + ".globl" + HAsm2Asm::space + name + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".bss" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".align" + HAsm2Asm::space + std::to_string(var_align) + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".type" + HAsm2Asm::space + name + ", @object" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".size" + HAsm2Asm::space + name +", " + std::to_string(size) + HAsm2Asm::newline;
                global_var_def += name + ":" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".zero" + HAsm2Asm::space + std::to_string(size) + HAsm2Asm::newline;
            } else {
                global_var_def += HAsm2Asm::space + ".globl" + HAsm2Asm::space + name + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".section" + HAsm2Asm::space + ".sdata, \"aw\"" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".align" + HAsm2Asm::space + std::to_string(var_align) + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".type" + HAsm2Asm::space + name + ", @object" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".size" + HAsm2Asm::space + name + ", " + std::to_string(size) + HAsm2Asm::newline;
                global_var_def += name + ":" + HAsm2Asm::newline;
                global_var_def += HAsm2Asm::space + ".word";
                if(initializer->getType()->isFloatType()) {
                    global_var_def += HAsm2Asm::space + std::to_string(*(uint32_t*)&(static_cast<ConstantFP*>(initializer)->getValue())) + HAsm2Asm::newline;
                } else {
                    global_var_def += HAsm2Asm::space + std::to_string(static_cast<ConstantInt*>(initializer)->getValue()) + HAsm2Asm::newline;
                }
            }
        }
        m_->add_to_data_section(global_var_def);
    }
}

void CodeGen::function_gen() {
    //& for debug info
  /*  #ifdef ENABLE_REGALLOC_DEBUG
        std::cout << "************* " << cur_func_->get_function()->getName() << " ****************" << std::endl;
        for(auto &[val,interval]: ival2interval) {
            LOG(INFO) << "value: " << val->getName() << ", " << "reg: " << Ireg2name[interval->reg_id];
        }
        for(auto &[val,interval]: fval2interval) {
            LOG(INFO) << "value: " << val->getName() << ", " << "reg: " << Freg2name[interval->reg_id];
        }
    #endif
*/              //debug,我删掉了
    auto func = cur_func_->get_function();

    //& record stack info and used tmp regs for inst gen
    cur_tmp_reg_saved_stack_offset = 0;
    caller_trans_args_stack_offset = 0;
    caller_saved_regs_stack_offset = 0;

    cur_tmp_iregs.clear();          //~ 当前借用的临时寄存器
    cur_tmp_fregs.clear();          //~ 当前借用的临时寄存器
    tmp_iregs_loc.clear();          //~ 保存临时寄存器原本值的地址
    tmp_fregs_loc.clear();          //~ 保存临时寄存器原本值的地址

    free_locs_for_tmp_regs_saved.clear();

    linearizing_and_labeling_bbs();

    int stack_size = stack_space_allocation();

    std::string func_header;
    func_header += HAsm2Asm::space + ".align" + HAsm2Asm::space + std::to_string(func_align) + HAsm2Asm::newline;
    func_header += HAsm2Asm::space + ".globl" + HAsm2Asm::space + func->getName() + HAsm2Asm::newline;
    func_header += HAsm2Asm::space + ".type" + HAsm2Asm::space + func->getName() + ",@function" + HAsm2Asm::newline;

    cur_func_->set_func_header(func_header);

    cur_bb_ = cur_func_->create_bb(func->getEntryBlock(), bb2label[func->getEntryBlock()]);

    callee_stack_prologue(stack_size);

    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs = callee_iargs_move(cur_func_->get_function());
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs = callee_fargs_move(cur_func_->get_function());

    if(!to_move_iargs.empty() || !to_move_fargs.empty())
        cur_bb_->create_callee_args_move(to_move_iargs, to_move_fargs);

    for(auto bb: linear_bbs) {
        if(bb != func->getEntryBlock()) 
            cur_bb_ = cur_func_->create_bb(bb, bb2label[bb]);
        bb_gen();
    }

    callee_stack_epilogue(stack_size);

    cur_bb_->create_ret();
}

void CodeGen::linearizing_and_labeling_bbs() {
    bb2label.clear();
    linear_bbs.clear();
    auto func = cur_func_->get_function();
    std::list<BasicBlock*> linear_bbs_of_func = func->getBasicBlocks();
    
    BasicBlock *ret_bb;
    Label* new_label;
    std::string label_str;
    
    for(auto bb: linear_bbs_of_func) {
        if(bb == func->getEntryBlock() && bb->getTerminator()->isRet()) {
            bb2label.insert({bb, new Label("")});
            linear_bbs.push_back(bb);
            return ;
        } else if(bb == func->getEntryBlock()) {
            bb2label.insert({bb, new Label("")});
        } else if(bb != func->getEntryBlock() && !bb->getTerminator()->isRet()) {
            label_str = func->getName() + "_" + bb->getName();
            new_label = new Label(label_str);
            bb2label.insert({bb, new_label});
        } else {
            ret_bb = bb;
            continue;
        }
        linear_bbs.push_back(bb);
    }
    label_str = func->getName() + "_" + "ret";
    new_label = new Label(label_str);
    bb2label.insert({ret_bb, new_label});
    linear_bbs.push_back(ret_bb);
}

//& stack HAsm2Asm::space alloc
int CodeGen::stack_space_allocation() {
    int total_size = 0;
    int iargs_size = 0;
    int fargs_size = 0;

    auto func = cur_func_->get_function();

    used_iregs_pair.first.clear();
    used_iregs_pair.second.clear();
    used_fregs_pair.first.clear();
    used_fregs_pair.second.clear();

    val2stack.clear();

    //! val2stack记录了通过栈传递给callee的参数,在callee中可能被分配到栈上(此时记录无用但不影响正确性)，也可能被分配到寄存器中
    if(func->getIArgs().size() > 8) {
        int i = 0;
        for(auto arg: func->getIArgs()) {
            if(i >= 8) {
                int size_of_arg_type = arg->getType()->getSize();
                val2stack[static_cast<Value*>(arg)] = new RegBase(reg_fp, iargs_size);
                iargs_size += ((size_of_arg_type + 7) / 8) * 8;
            }
            i++;
        }
    }

    if(func->getFArgs().size() > 8) {
        int i = 0;
        for(auto arg: func->getFArgs()) {
            if(i >= 8) {
                int size_of_arg_type = arg->getType()->getSize();
                val2stack[static_cast<Value*>(arg)] = new RegBase(reg_fp, iargs_size + fargs_size);
                fargs_size += ((size_of_arg_type + 7) / 8) * 8;
            } 
            i++;
        }
    }

    //& build reg -> values map or val -> stack map
    for(auto iter: ival2interval) {
        Value *val_iter = iter.first;
        Interval *interval_iter = iter.second;
        if(interval_iter->reg_id >= 0) {
            auto iter = callee_saved_iregs.find(interval_iter->reg_id);
            if(iter != callee_saved_iregs.end()) {
                used_iregs_pair.second.insert(interval_iter->reg_id);
            } else {
                used_iregs_pair.first.insert(interval_iter->reg_id);
            }
        }
    }

    for(auto iter: fval2interval) {
        Value *val_iter = iter.first;
        Interval *interval_iter = iter.second;
        if(interval_iter->reg_id >= 0) {
            auto iter = callee_saved_fregs.find(interval_iter->reg_id);
            if(iter != callee_saved_fregs.end()) {
                used_fregs_pair.second.insert(interval_iter->reg_id);
            } else {
                used_fregs_pair.first.insert(interval_iter->reg_id);
            }
        }
    }

    //& 总是保存fp,ra,s1寄存器
    used_iregs_pair.second.insert(reg_ra);
    used_iregs_pair.second.insert(reg_fp);

    total_size += reg_size * (used_iregs_pair.second.size() + used_fregs_pair.second.size());

    //& 为栈上的指针参数分配空间

    for(auto iter: ival2interval) {
        Value *val_iter = iter.first;
        Interval *interval_iter = iter.second;
        if(interval_iter->reg_id < 0) {
            auto arg = dynamic_cast<Argument*>(val_iter);
            if(arg && val2stack[static_cast<Value*>(arg)] != nullptr) 
                continue;
            int size_of_val_type = val_iter->getType()->getSize();
            total_size += ((size_of_val_type + 7) / 8) * 8;
            val2stack[val_iter] = new RegBase(reg_fp, -total_size);
        }
    }

    for(auto iter: fval2interval) {
        Value *val_iter = iter.first;
        Interval *interval_iter = iter.second;
        if(interval_iter->reg_id < 0) {
            auto arg = dynamic_cast<Argument*>(val_iter);
            if(arg && val2stack[static_cast<Value*>(arg)] != nullptr) 
                continue;
            int size_of_val_type = val_iter->getType()->getSize();
            total_size += ((size_of_val_type + 7) / 8) * 8;
            val2stack[val_iter] = new RegBase(reg_fp, -total_size);
        }
    }

    //& handle alloc inst
    for(auto &inst: func->getEntryBlock()->getInstructions()) {
        auto alloc = dynamic_cast<AllocaInst*>(inst);
        if(!alloc)
            continue;
        int size_of_alloc_type = alloc->getAllocaType()->getSize();
        total_size += ((size_of_alloc_type + 3) / 4) * 4;
        val2stack[static_cast<Value*>(alloc)] = new RegBase(reg_fp, -total_size);
    }

    //& for debug info
    #ifdef ENABLE_REGALLOC_DEBUG
        for(auto &[val, loc]: val2stack) {
            std::cout << val->getName() << ": " << loc->get_asm_code() << std::endl; 
        }
    #endif

    return ((total_size + 7) / 8) * 8;
}

void CodeGen::callee_stack_prologue(int stack_size) {
    int cur_offset = 0;

    std::vector<std::pair<RegLoc*, RegBase*>> to_save_regs;

    if(!used_iregs_pair.second.empty()) {
        for(auto iter = used_iregs_pair.second.begin(); iter != used_iregs_pair.second.end(); iter++) {
            cur_offset -= reg_size;
            to_save_regs.push_back(std::make_pair(new RegLoc(*iter, false), new RegBase(reg_sp, cur_offset)));
        }
    }
    if(!used_fregs_pair.second.empty()) {
        for(auto iter = used_fregs_pair.second.begin(); iter != used_fregs_pair.second.end(); iter++) {
            cur_offset -= reg_size;
            to_save_regs.push_back(std::make_pair(new RegLoc(*iter, true), new RegBase(reg_sp, cur_offset)));
        }
    }

    cur_bb_->create_callee_save_regs(to_save_regs);
    cur_bb_->create_callee_start_stack_frame(stack_size);
}

//! 初赛测试中未出现寄存器移动loop的情况
std::vector<std::pair<HAsmLoc*, HAsmLoc*>> CodeGen::callee_iargs_move(Function *func) {
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_locs;
    auto iargs_vector = func->getIArgs();
    int first_iargs_num = iargs_vector.size() > 8 ?   8 : iargs_vector.size();

    std::map<int, bool> is_args_moved;

    for(int i = 0; i < first_iargs_num; ++i) {
        is_args_moved[i] = false;
    }

    std::list<std::pair<Argument*, int>> iargs_dependency_chain;

    //& solved first 8(if have) int or ptr arguments
    while(true) {
        int i = 0;
        for(auto &[arg, is_moved]: is_args_moved) {
            if(!is_moved) 
                break;
            i++;
        }
        //& all moved
        if(i == first_iargs_num)
            break;

        while(true) {
            if(ival2interval.find(iargs_vector[i]) != ival2interval.end()) {
                int target_reg_id = ival2interval[iargs_vector[i]]->reg_id;
                if(is_args_moved[i]) {
                    for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                        auto iarg = riter->first;
                        to_move_locs.push_back(std::make_pair(new RegLoc(ival2interval[iarg]->reg_id, false), new RegLoc(arg_reg_base + riter->second, false)));
                        is_args_moved[riter->second] = true;
                    }
                    iargs_dependency_chain.clear();
                    break;
                } else if(target_reg_id < 0) {
                    int base_reg_id = val2stack[iargs_vector[i]]->get_reg_id();
                    int offset = val2stack[iargs_vector[i]]->get_offset();
                    to_move_locs.push_back(std::make_pair(new RegBase(base_reg_id, offset), new RegLoc(arg_reg_base + i, false)));
                    is_args_moved[i] = true;
                    if(!iargs_dependency_chain.empty()) {
                        for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                            auto iarg = riter->first;
                            to_move_locs.push_back(std::make_pair(new RegLoc(ival2interval[iarg]->reg_id, false), new RegLoc(arg_reg_base + riter->second, false)));
                            is_args_moved[riter->second] = true;
                        }
                        iargs_dependency_chain.clear();
                    } 
                    break;
                } else if(target_reg_id - arg_reg_base == i) {
                    is_args_moved[i] = true;
                    break;
                } else if(target_reg_id - arg_reg_base >= first_iargs_num || target_reg_id - arg_reg_base < 0) {
                    to_move_locs.push_back(std::make_pair(new RegLoc(target_reg_id, false), new RegLoc(arg_reg_base + i, false)));
                    is_args_moved[i] = true;
                    if(!iargs_dependency_chain.empty()) {
                        for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                            auto iarg = riter->first;
                            to_move_locs.push_back(std::make_pair(new RegLoc(ival2interval[iarg]->reg_id, false), new RegLoc(arg_reg_base + riter->second, false)));
                            is_args_moved[riter->second] = true;
                        }
                        iargs_dependency_chain.clear();
                    }
                    break;
                } else {
                    iargs_dependency_chain.push_back({iargs_vector[i], i});
                    i = target_reg_id - arg_reg_base;
                    if(iargs_vector[i] == iargs_dependency_chain.begin()->first) {
                        //& found loop
                        to_move_locs.push_back(std::make_pair(new RegLoc(reg_s1, false), new RegLoc(arg_reg_base + iargs_dependency_chain.rbegin()->second, false)));
                        for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                            if(riter->first == iargs_dependency_chain.rbegin()->first)
                                continue;
                            auto iarg = riter->first;
                            to_move_locs.push_back(std::make_pair(new RegLoc(ival2interval[iarg]->reg_id, false), new RegLoc(arg_reg_base + riter->second, false)));
                            is_args_moved[riter->second] = true;
                        }
                        to_move_locs.push_back(std::make_pair(new RegLoc(ival2interval[iargs_dependency_chain.rbegin()->first]->reg_id, false), new RegLoc(reg_s1, false)));
                        is_args_moved[iargs_dependency_chain.rbegin()->second] = true;
                        iargs_dependency_chain.clear();
                        break;
                    }
                }
            } else {
                is_args_moved[i] = true;
                if(!iargs_dependency_chain.empty()) {
                    for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                        auto iarg = riter->first;
                        to_move_locs.push_back(std::make_pair(new RegLoc(ival2interval[iarg]->reg_id, false), new RegLoc(arg_reg_base + riter->second, false)));
                        is_args_moved[riter->second] = true;
                    }
                    iargs_dependency_chain.clear();
                }
                break;
            }
        }
    }

    //& solved the other(if have) int or ptr arguments
    for(int i = first_iargs_num; i < iargs_vector.size(); i++) {
        if(ival2interval.find(iargs_vector[i]) != ival2interval.end()) {
            int target_reg_id = ival2interval[iargs_vector[i]]->reg_id;
            if(target_reg_id < 0) {
                continue;
            } else {
                to_move_locs.push_back(std::make_pair(new RegLoc(target_reg_id, false), new RegBase(reg_fp, reg_size * (i - first_iargs_num))));
            }
        } 
    }

    return to_move_locs;
}

//! 初赛测试中未出现寄存器移动loop的情况
std::vector<std::pair<HAsmLoc*, HAsmLoc*>> CodeGen::callee_fargs_move(Function *func) {
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_locs;

    int iargs_num = func->getIArgs().size();
    iargs_num = iargs_num > 8 ? (iargs_num-8) : 0;

    auto fargs_vector = func->getFArgs();
    int first_fargs_num = fargs_vector.size() > 8 ?   8 : fargs_vector.size();

    std::map<int, bool> is_args_moved;

    for(int i = 0; i < first_fargs_num; ++i) {
        is_args_moved[i] = false;
    }

    std::list<std::pair<Argument*, int>> fargs_dependency_chain;

    //& solved first 8 float arguments
    while(true) {
        int i = 0;
        for(auto &[arg, is_moved]: is_args_moved) {
            if(!is_moved) 
                break;
            i++;
        }
        //& all moved
        if(i == first_fargs_num)
            break;

        while(true) {
            if(fval2interval.find(fargs_vector[i]) != fval2interval.end()) {
                int target_reg_id = fval2interval[fargs_vector[i]]->reg_id;
                if(is_args_moved[i]) {
                    for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                        auto farg = riter->first;
                        to_move_locs.push_back(std::make_pair(new RegLoc(fval2interval[farg]->reg_id, true), new RegLoc(arg_reg_base + riter->second, true)));
                        is_args_moved[riter->second] = true;
                    }
                    fargs_dependency_chain.clear();
                    break;
                } else if(target_reg_id < 0) {
                    int base_reg_id = val2stack[fargs_vector[i]]->get_reg_id();
                    int offset = val2stack[fargs_vector[i]]->get_offset();
                    to_move_locs.push_back(std::make_pair(new RegBase(base_reg_id, offset), new RegLoc(arg_reg_base + i, true)));
                    is_args_moved[i] = true;
                    if(!fargs_dependency_chain.empty()) {
                        for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                            auto farg = riter->first;
                            to_move_locs.push_back(std::make_pair(new RegLoc(fval2interval[farg]->reg_id, true), new RegLoc(arg_reg_base + riter->second, true)));
                            is_args_moved[riter->second] = true;
                        }
                        fargs_dependency_chain.clear();
                    } 
                    break;
                } else if(target_reg_id - arg_reg_base == i) {
                    is_args_moved[i] = true;
                    break;
                } else if(target_reg_id - arg_reg_base >= first_fargs_num || target_reg_id - arg_reg_base < 0) {
                    to_move_locs.push_back(std::make_pair(new RegLoc(target_reg_id, true), new RegLoc(arg_reg_base + i, true)));
                    is_args_moved[i] = true;
                    int target = i;
                    if(!fargs_dependency_chain.empty()) {
                        for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                            auto farg = riter->first;
                            to_move_locs.push_back(std::make_pair(new RegLoc(fval2interval[farg]->reg_id, true), new RegLoc(arg_reg_base + riter->second, true)));
                            target = riter->second;
                            is_args_moved[riter->second] = true;
                        }
                        fargs_dependency_chain.clear();
                    }
                    break;
                } else {
                    fargs_dependency_chain.push_back({fargs_vector[i], i});
                    i = target_reg_id - arg_reg_base;
                    if(fargs_vector[i] == fargs_dependency_chain.begin()->first) {
                        //& found loop    
                        to_move_locs.push_back(std::make_pair(new RegLoc(reg_fs1, true), new RegLoc(arg_reg_base + fargs_dependency_chain.rbegin()->second, true)));
                        for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                            if(riter->first == fargs_dependency_chain.rbegin()->first)
                                continue;
                            auto farg = riter->first;
                            to_move_locs.push_back(std::make_pair(new RegLoc(fval2interval[farg]->reg_id, true), new RegLoc(arg_reg_base + riter->second, true)));
                            is_args_moved[riter->second] = true;
                        }
                        to_move_locs.push_back(std::make_pair(new RegLoc(fval2interval[fargs_dependency_chain.rbegin()->first]->reg_id, true), new RegLoc(reg_fs1, true)));
                        is_args_moved[fargs_dependency_chain.rbegin()->second] = true;
                        fargs_dependency_chain.clear();
                        break;
                    }
                }
            } else {
                is_args_moved[i] = true;
                if(!fargs_dependency_chain.empty()) {
                    for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                        auto farg = riter->first;
                        to_move_locs.push_back(std::make_pair(new RegLoc(fval2interval[farg]->reg_id, true), new RegLoc(arg_reg_base + riter->second, true)));
                        is_args_moved[riter->second] = true;
                    }
                    fargs_dependency_chain.clear();
                }
                break;
            }
        }
    }

    //& solved the other(if have) float arguments
    for(int i = first_fargs_num; i < fargs_vector.size(); i++) {
        if(fval2interval.find(fargs_vector[i]) != fval2interval.end()) {
            int target_reg_id = fval2interval[fargs_vector[i]]->reg_id;
            if(target_reg_id < 0) {
                continue;
            } else {
                to_move_locs.push_back(std::make_pair(new RegLoc(target_reg_id, true), new RegBase(reg_fp, reg_size * (iargs_num + i - first_fargs_num))));
            }
        } 
    }
    return to_move_locs;
}

void CodeGen::bb_gen() {
    Instruction *br_inst = nullptr;
    for(auto &inst: cur_bb_->get_bb()->getInstructions()) {
        if(inst->isTerminator()) {
            br_inst = inst;
            break;
        }
        ld_tmp_regs_for_inst(inst);
        if(inst->isCall()) {
            auto call_inst = dynamic_cast<CallInst*>(inst);
            caller_reg_store(cur_bb_->get_bb()->getParent(), call_inst);

            std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_fargs = caller_fargs_move(call_inst);
            std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_iargs = caller_iargs_move(call_inst);

            if(!to_move_fargs.empty() || !to_move_iargs.empty())
                cur_bb_->create_caller_args_move(to_move_fargs, to_move_iargs);

            int extra_stack_offset = caller_trans_args_stack_offset + cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset; 
            
            if(extra_stack_offset != 0) 
                cur_bb_->create_expand_stack_space(extra_stack_offset);
            
            instr_gen(call_inst);
            
            if(extra_stack_offset != 0) 
                cur_bb_->create_shrink_stack_space(-extra_stack_offset);
            
            caller_reg_restore(cur_bb_->get_bb()->getParent(), call_inst);
            
            caller_trans_args_stack_offset = 0;
        } else if(!inst->isPhi()) {
            alloc_tmp_regs_for_inst(inst);
            instr_gen(inst);
            store_tmp_reg_for_inst(inst);
        } 
    }
    ld_tmp_regs_for_inst(br_inst);

    if(br_inst->isRet()) {
        instr_gen(br_inst);
    } else {
        phi_union(br_inst);
    }
}

void CodeGen::callee_stack_epilogue(int stack_size) {

    std::vector<std::pair<RegLoc*, RegBase*>> to_load_regs;
    
    cur_bb_->create_callee_end_stack_frame(stack_size);
    
    int num_of_all_restore_regs = used_iregs_pair.second.size() + used_fregs_pair.second.size();
    int cur_offset = - reg_size * num_of_all_restore_regs;
    if(!used_fregs_pair.second.empty()) {
        for(auto iter = used_fregs_pair.second.rbegin(); iter != used_fregs_pair.second.rend(); iter++) {
            to_load_regs.push_back(std::make_pair(new RegLoc(*iter, true), new RegBase(reg_sp, cur_offset)));
            cur_offset += reg_size; 
        }
    }
    
    if(!used_iregs_pair.second.empty()) {
        for(auto iter = used_iregs_pair.second.rbegin(); iter != used_iregs_pair.second.rend(); iter++) {
            to_load_regs.push_back(std::make_pair(new RegLoc(*iter, false), new RegBase(reg_sp, cur_offset)));
            cur_offset += reg_size; 
        }
    }
    cur_bb_->create_callee_restore_regs(to_load_regs);
}

void CodeGen::ld_tmp_regs_for_inst(Instruction *inst) {
    if(inst->isAlloca() || inst->isPhi())
        return ;

    std::set<int> to_del_tmp_iregs_set;
    std::set<int> to_ld_tmp_iregs_set;
    std::set<int> to_del_tmp_fregs_set;
    std::set<int> to_ld_tmp_fregs_set;

    std::vector<std::pair<RegLoc*, RegBase*>> to_ld_tmp_regs;
    
    for(auto opr: inst->getOperands()) {
        if(dynamic_cast<Constant*>(opr) ||
        dynamic_cast<BasicBlock*>(opr) ||
        dynamic_cast<GlobalVariable*>(opr) ||
        dynamic_cast<AllocaInst*>(opr) ||
        dynamic_cast<Function*>(opr)) {
            continue;
        }
        if(opr->getType()->isFloatType()) {
            int opr_reg = fval2interval[opr]->reg_id;
            if(opr_reg >= 0) {
                if(cur_tmp_fregs.find(opr_reg) != cur_tmp_fregs.end()) {
                    to_ld_tmp_fregs_set.insert(opr_reg);
                    to_del_tmp_fregs_set.insert(opr_reg);
                }
            } 
        } else {
            int opr_reg = ival2interval[opr]->reg_id;
            if(opr_reg >= 0) {
                if(cur_tmp_iregs.find(opr_reg) != cur_tmp_iregs.end()) {
                    to_ld_tmp_iregs_set.insert(opr_reg);
                    to_del_tmp_iregs_set.insert(opr_reg);
                }
            } 
        }
    }

    if(!inst->isVoid()) {
        if(inst->getType()->isFloatType()) {
            int inst_reg_id = fval2interval[inst]->reg_id;
            if(inst_reg_id >= 0) {
                if(cur_tmp_fregs.find(inst_reg_id) != cur_tmp_fregs.end()) {
                    to_del_tmp_fregs_set.insert(inst_reg_id);
                }
            }
        } else {
            int inst_reg_id = ival2interval[inst]->reg_id;
            if(inst_reg_id >= 0) {
                if(cur_tmp_iregs.find(inst_reg_id) != cur_tmp_iregs.end()) {
                    to_del_tmp_iregs_set.insert(inst_reg_id);
                }
            }
        }
    }

    for(auto ld_reg: to_ld_tmp_iregs_set) {        
        RegBase* regbase = tmp_iregs_loc[ld_reg];
        to_ld_tmp_regs.push_back(std::make_pair(new RegLoc(ld_reg, false), regbase));
    }

    for(auto ld_reg: to_ld_tmp_fregs_set) {
        RegBase* regbase = tmp_fregs_loc[ld_reg];
        to_ld_tmp_regs.push_back(std::make_pair(new RegLoc(ld_reg, true), regbase));
    }

    if(! to_ld_tmp_regs.empty())
        cur_bb_->create_ld_tmp_regs(to_ld_tmp_regs);

    for(auto del_reg: to_del_tmp_iregs_set) {
        auto del_loc = tmp_iregs_loc[del_reg];
        free_locs_for_tmp_regs_saved.insert(del_loc);
        cur_tmp_iregs.erase(del_reg);
        tmp_iregs_loc.erase(del_reg);
    }

    for(auto del_reg: to_del_tmp_fregs_set) {
        auto del_loc = tmp_fregs_loc[del_reg];
        free_locs_for_tmp_regs_saved.insert(del_loc);
        cur_tmp_fregs.erase(del_reg);
        tmp_fregs_loc.erase(del_reg);
    }
    return ;
}

void CodeGen::alloc_tmp_regs_for_inst(Instruction *inst) {
    istore_list.clear();
    fstore_list.clear();
    use_tmp_regs_interval.clear();
    std::set<int> inst_ireg_id_set;
    std::set<int> inst_freg_id_set;
    std::set<int> used_tmp_iregs;
    std::set<int> used_tmp_fregs;
    to_store_ivals.clear();
    to_store_fvals.clear();

    std::set<Value*> to_ld_ival_set;
    std::set<Value*> to_ld_fval_set;

    std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs;
    std::vector<std::pair<RegLoc*, RegBase*>> to_ld_regs;

    for(auto opr: inst->getOperands()) {
        if(dynamic_cast<Constant*>(opr) ||
        dynamic_cast<BasicBlock*>(opr) ||
        dynamic_cast<GlobalVariable*>(opr) ||
        dynamic_cast<AllocaInst*>(opr)) {
            continue;
        }
        if(opr->getType()->isFloatType()) {
            if(fval2interval[opr]->reg_id >= 0) {
                inst_freg_id_set.insert(fval2interval[opr]->reg_id);
            } 
        } else {
            if(ival2interval[opr]->reg_id >= 0) {
                inst_ireg_id_set.insert(ival2interval[opr]->reg_id);
            } 
        }
    }

    //& finding a register to store the result for an instruction
    if(!inst->isVoid() && !dynamic_cast<AllocaInst*>(inst)) {
        if(inst->getType()->isFloatType()) {
            auto reg_interval = fval2interval[inst];
            if(reg_interval->reg_id < 0) {
                if(!cur_tmp_fregs.empty()) {
                    reg_interval->reg_id = *cur_tmp_fregs.begin();
                    cur_tmp_fregs.erase(reg_interval->reg_id);
                    used_tmp_fregs.insert(reg_interval->reg_id);
                } else {
                    for(auto freg: all_available_freg_ids) {
                        if(inst_freg_id_set.find(freg) == inst_freg_id_set.end()) {
                            reg_interval->reg_id = freg;
                            fstore_list.insert(freg);
                            break;
                        }
                    }
                }
                use_tmp_regs_interval.insert(reg_interval);
                to_store_fvals.insert(inst);
            } 
            inst_freg_id_set.insert(reg_interval->reg_id);
            if(reg_interval->reg_id < 0) 
                std::cout<<"在为指令生成代码时分配临时寄存器出现异常"<<std::endl;  //这里有个if，必须有语句，所以加里这句话
               // LOG(ERROR) << "在为指令生成代码时分配临时寄存器出现异常";
        } else {
            auto reg_interval = ival2interval[inst];
            if(reg_interval->reg_id < 0) {
                if(!cur_tmp_iregs.empty()) {
                    reg_interval->reg_id = *cur_tmp_iregs.begin();
                    cur_tmp_iregs.erase(reg_interval->reg_id);
                    used_tmp_iregs.insert(reg_interval->reg_id);
                } else {
                    for(auto ireg: all_available_ireg_ids) {
                        if(inst_ireg_id_set.find(ireg) == inst_ireg_id_set.end()) {
                            reg_interval->reg_id = ireg;
                            istore_list.insert(ireg);
                            break;
                        }
                    }
                }
                use_tmp_regs_interval.insert(reg_interval);
                to_store_ivals.insert(inst);
            } 
            inst_ireg_id_set.insert(reg_interval->reg_id);
            if(reg_interval->reg_id < 0) 
             std::cout<<"在为指令生成代码时分配临时寄存器出现异常"<<std::endl;  //这里有个if，必须有语句，所以加里这句话
               // LOG(ERROR) << "在为指令生成代码时分配临时寄存器出现异常";
        }
    }

    //& finding registers for the operands of an instruction
    for(auto opr: inst->getOperands()) {
        if(dynamic_cast<Constant*>(opr) ||
        dynamic_cast<BasicBlock*>(opr) ||
        dynamic_cast<GlobalVariable*>(opr) ||
        dynamic_cast<AllocaInst*>(opr)) {
            continue;
        }

        if(opr->getType()->isFloatType()) {
            auto reg_interval = fval2interval[opr];
            if(reg_interval->reg_id < 0) {
                if(!cur_tmp_fregs.empty()) {
                    reg_interval->reg_id = *cur_tmp_fregs.begin();
                    cur_tmp_fregs.erase(reg_interval->reg_id);
                    used_tmp_fregs.insert(reg_interval->reg_id);
                    inst_freg_id_set.insert(reg_interval->reg_id);
                } else {
                    for(auto freg: all_available_freg_ids) {
                        if(inst_freg_id_set.find(freg) == inst_freg_id_set.end()) {
                            reg_interval->reg_id = freg;
                            fstore_list.insert(freg);
                            inst_freg_id_set.insert(freg);
                            break;
                        }
                    }
                }
                to_ld_fval_set.insert(opr);
                use_tmp_regs_interval.insert(reg_interval);
            } 
            if(reg_interval->reg_id < 0) 
              std::cout<<"在为指令生成代码时分配临时寄存器出现异常"<<std::endl;  //这里有个if，必须有语句，所以加里这句话
               // LOG(ERROR) << "在为指令生成代码时分配临时寄存器出现异常";
        } else {
            auto reg_interval = ival2interval[opr];
            if(reg_interval->reg_id < 0) {
                if(!cur_tmp_iregs.empty()) {
                    reg_interval->reg_id = *cur_tmp_iregs.begin();
                    cur_tmp_iregs.erase(reg_interval->reg_id);
                    used_tmp_iregs.insert(reg_interval->reg_id);
                    inst_ireg_id_set.insert(reg_interval->reg_id);
                } else {
                    for(auto ireg: all_available_ireg_ids) {
                        if(inst_ireg_id_set.find(ireg) == inst_ireg_id_set.end()) {
                            reg_interval->reg_id = ireg;
                            istore_list.insert(ireg);
                            inst_ireg_id_set.insert(ireg);
                            break;
                        }
                    }
                }
                to_ld_ival_set.insert(opr);
                use_tmp_regs_interval.insert(reg_interval);
            } 
            if(reg_interval->reg_id < 0) 
              std::cout<<"在为指令生成代码时分配临时寄存器出现异常"<<std::endl;  //这里有个if，必须有语句，所以加里这句话
            //    LOG(ERROR) << "在为指令生成代码时分配临时寄存器出现异常";
        }
    }

    //& store the origin values in tmp used regs
    for(auto reg_id: istore_list) {
        if(free_locs_for_tmp_regs_saved.empty()) {
            cur_tmp_reg_saved_stack_offset -= 8;
            RegBase* loc = new RegBase(reg_sp, cur_tmp_reg_saved_stack_offset);
            tmp_iregs_loc[reg_id] = loc;
            to_store_regs.push_back(std::make_pair(new RegLoc(reg_id, false), loc));
        } else {
            RegBase* loc = *free_locs_for_tmp_regs_saved.begin();
            free_locs_for_tmp_regs_saved.erase(free_locs_for_tmp_regs_saved.begin());
            tmp_iregs_loc[reg_id] = loc;
            to_store_regs.push_back(std::make_pair(new RegLoc(reg_id, false), loc));
        }
        cur_tmp_iregs.insert(reg_id);
    }

    for(auto reg_id: fstore_list) {
        if(free_locs_for_tmp_regs_saved.empty()) {
            cur_tmp_reg_saved_stack_offset -= 8;
            RegBase* loc = new RegBase(reg_sp, cur_tmp_reg_saved_stack_offset);
            tmp_fregs_loc[reg_id] = loc;
            to_store_regs.push_back(std::make_pair(new RegLoc(reg_id, true), loc));
        } else {
            RegBase* loc = *free_locs_for_tmp_regs_saved.begin();
            free_locs_for_tmp_regs_saved.erase(free_locs_for_tmp_regs_saved.begin());
            tmp_fregs_loc[reg_id] = loc;
            to_store_regs.push_back(std::make_pair(new RegLoc(reg_id, true), loc));
        }
        cur_tmp_fregs.insert(reg_id);
    }

    for(auto tmp_freg: used_tmp_fregs) {
        cur_tmp_fregs.insert(tmp_freg);
    }

    for(auto tmp_ireg: used_tmp_iregs) {
        cur_tmp_iregs.insert(tmp_ireg);
    }

    //& load the vals on stack
    for(auto fval: to_ld_fval_set) {
        RegBase *reg_base = val2stack[fval];
        to_ld_regs.push_back(std::make_pair(new RegLoc(fval2interval[fval]->reg_id, true), reg_base));
    }

    for(auto ival: to_ld_ival_set) {
        RegBase *reg_base = val2stack[ival];
        to_ld_regs.push_back(std::make_pair(new RegLoc(ival2interval[ival]->reg_id, false), reg_base));
    }

    if(! to_ld_regs.empty() || !to_store_regs.empty())
        cur_bb_->create_alloc_tmp_regs_with_save_initial_owner(to_store_regs, to_ld_regs);
}

void CodeGen::store_tmp_reg_for_inst(Instruction *inst) {
    std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs;

    if(!to_store_ivals.empty()) {
        for(auto ival: to_store_ivals) {
            RegBase *regbase = val2stack[ival];
            to_store_regs.push_back(std::make_pair(new RegLoc(ival2interval[ival]->reg_id, false), regbase));
        }   
        to_store_ivals.clear();
    } 
    if(!to_store_fvals.empty()) {
        for(auto fval: to_store_fvals) {
            RegBase *regbase = val2stack[fval];
            to_store_regs.push_back(std::make_pair(new RegLoc(fval2interval[fval]->reg_id, true), regbase));
        }
        to_store_fvals.clear();
    }

    for(auto inter: use_tmp_regs_interval) {
        inter->reg_id = -1;
    }

    to_store_ivals.clear();
    to_store_fvals.clear();
    use_tmp_regs_interval.clear();

    if(! to_store_regs.empty())
        cur_bb_->create_store_tmp_result(to_store_regs);
}

void CodeGen::tmp_regs_restore() {
    
    std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs;

    for(auto &[ireg_id, loc]: tmp_iregs_loc) {
        to_restore_regs.push_back(std::make_pair(new RegLoc(ireg_id, false), loc));
    }

    for(auto &[freg_id, loc]: tmp_fregs_loc) {
        to_restore_regs.push_back(std::make_pair(new RegLoc(freg_id, true), loc));
    }
    tmp_iregs_loc.clear();
    tmp_fregs_loc.clear();
    cur_tmp_iregs.clear();
    cur_tmp_fregs.clear();
    free_locs_for_tmp_regs_saved.clear();
    cur_tmp_reg_saved_stack_offset = 0;

    if(! to_restore_regs.empty())
        cur_bb_->create_restore_all_tmp_regs(to_restore_regs);

    return ;
}

void CodeGen::caller_reg_store(Function* func, CallInst* call) {
    caller_saved_ireg_locs.clear();
    caller_saved_freg_locs.clear();
    caller_save_iregs.clear();
    caller_save_fregs.clear();

    std::vector<std::pair<RegLoc*, RegBase*>> to_store_regs;

    int call_inst_reg = -1;
    bool is_float_call = call->getType()->isFloatType();
    bool is_void_call = call->getType()->isVoidType();
    bool is_int_call = !is_float_call && !is_void_call;

    if(!is_void_call) {
        if(is_float_call && fval2interval.find(call) != fval2interval.end()) {
            call_inst_reg = fval2interval[call]->reg_id;
        } else if(ival2interval.find(call) != ival2interval.end()) {
            call_inst_reg = ival2interval[call]->reg_id;
        }
    }
      
    for(auto ireg: used_iregs_pair.first) {
        if(is_int_call && ireg == call_inst_reg)
            continue;
        caller_save_iregs.push_back(ireg);
    }
    for(auto freg: used_fregs_pair.first) {
        if(is_float_call && freg == call_inst_reg)
            continue;
        caller_save_fregs.push_back(freg);
    }
    for(auto reg: caller_save_iregs) {
        caller_saved_regs_stack_offset -= 8;
        auto regbase = new RegBase(reg_sp, cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset);
        caller_saved_ireg_locs[reg] = regbase;
        to_store_regs.push_back(std::make_pair(new RegLoc(reg, false), regbase));
    }
    for(auto reg: caller_save_fregs) {
        caller_saved_regs_stack_offset -= 8;
        auto regbase = new RegBase(reg_sp, cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset);
        caller_saved_freg_locs[reg] = regbase;
        to_store_regs.push_back(std::make_pair(new RegLoc(reg, true), regbase));
    }
    
    if(! to_store_regs.empty())
        cur_bb_->create_caller_save_regs(to_store_regs);
}


void CodeGen::caller_reg_restore(Function* func, CallInst* call) {
    std::vector<std::pair<RegLoc*, RegBase*>> to_restore_regs;

    for(auto &[freg, loc]: caller_saved_freg_locs) {
        to_restore_regs.push_back(std::make_pair(new RegLoc(freg, true), loc));    
    }
    for(auto &[ireg, loc]: caller_saved_ireg_locs) {
        to_restore_regs.push_back(std::make_pair(new RegLoc(ireg, false), loc)); 
    }
    caller_saved_regs_stack_offset = 0;

    if(! to_restore_regs.empty())
        cur_bb_->create_caller_restore_regs(to_restore_regs);
    return ;
}


//! 初赛测试中未出现一个寄存器向多个寄存器中移动的情况
std::vector<std::pair<HAsmLoc*, HAsmLoc*>> CodeGen::caller_iargs_move(CallInst *call) {
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_locs;

    std::vector<Value*> iargs;
    for(auto arg: call->getOperands()) {
        if(dynamic_cast<Function*>(arg))
            continue;
        if(!arg->getType()->isFloatType()) 
            iargs.push_back(arg);
    }

    int num_of_iargs = iargs.size() > 8 ? 8 : iargs.size();

    std::map<int, bool> is_args_moved;

    std::map<int, std::set<int>> reg2iargnos;

    std::list<std::pair<Value*, int>> reg_dependency_chain;


    for(int i = 0; i < num_of_iargs; i++) {
        is_args_moved[i] = false;
        if(ival2interval.find(iargs[i]) != ival2interval.end()) {
            int reg_id = ival2interval[iargs[i]]->reg_id;
            if(reg_id >= 0) {
                if(reg2iargnos.find(reg_id) == reg2iargnos.end()) {
                    reg2iargnos.insert({reg_id, {i}});
                } else {
                    reg2iargnos[reg_id].insert(i);
                }
            }
        }
    }

    int extra_stack_offset =  cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset;

    //& 首先处理需要store到栈上的参数
    for(int i = iargs.size() - 1; i >= num_of_iargs; i--) {
        caller_trans_args_stack_offset -= reg_size;
        if(ival2interval.find(iargs[i]) != ival2interval.end()) {
            auto reg_id = ival2interval[iargs[i]]->reg_id;
            if(reg_id >= 0) {
                to_move_locs.push_back(std::make_pair(new RegBase(reg_sp, extra_stack_offset + caller_trans_args_stack_offset), new RegLoc(reg_id, false)));
            } else {
                to_move_locs.push_back(std::make_pair(new RegBase(reg_sp, extra_stack_offset + caller_trans_args_stack_offset), val2stack[iargs[i]]));
            }
        } else {
            auto const_int = dynamic_cast<ConstantInt*>(iargs[i]);
            if(const_int) {
                to_move_locs.push_back(std::make_pair(new RegBase(reg_sp, extra_stack_offset + caller_trans_args_stack_offset), new ConstPool(const_int->getValue())));
            } else {
                //LOG(ERROR) << "汇编代码生成发生错误";
            }
        }
    }

    while(true) {
        int i = 0;
        //& 寻找一个尚未移动的参数
        for(auto &[arg, is_moved]: is_args_moved) {
            if(!is_moved) 
                break;
            i++;
        }
        //& all moved
        if(i == is_args_moved.size()) 
            break;
        while(true) {
            if(reg2iargnos.find(i + arg_reg_base) != reg2iargnos.end()) {
                if(reg2iargnos[i+arg_reg_base].find(i) != reg2iargnos[i+arg_reg_base].end()) {
                    if(reg2iargnos[i+arg_reg_base].size() == 1) {
                        is_args_moved[i] = true;
                        reg2iargnos[i+arg_reg_base].erase(i);
                        reg2iargnos.erase(i+arg_reg_base);
                    } else {
                        for(auto tmp_argno: reg2iargnos[i+arg_reg_base]) {
                            if(tmp_argno != i) {
                                i = tmp_argno;
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                } else {
                    reg_dependency_chain.push_back({iargs[i], i});
                    i = *reg2iargnos[i+arg_reg_base].begin();
                    if(i == reg_dependency_chain.begin()->second) {
                        //& found loop
                        //& check if or not no branches (a single circle)
                        bool is_single_circle = true; 
                        for(auto iter = reg_dependency_chain.begin(); iter != reg_dependency_chain.end(); iter++) {
                            if(reg2iargnos[iter->second + arg_reg_base].size() > 1) {
                                int next_argno;
                                if(iter->first == reg_dependency_chain.rbegin()->first) {
                                    next_argno = reg_dependency_chain.begin()->second;
                                } else {
                                    next_argno = (++iter)->second;
                                    iter--;
                                }
                                for(auto tmp_argno: reg2iargnos[iter->second + arg_reg_base]) {
                                    if(tmp_argno != next_argno) {
                                        i = tmp_argno;
                                        break;
                                    } else {
                                        continue;
                                    }
                                }
                                reg_dependency_chain.clear();
                                is_single_circle = false;
                                break;
                            }
                        }
                        if(is_single_circle) {
                            to_move_locs.push_back(std::make_pair(new RegLoc(reg_s1, false), new RegLoc(ival2interval[reg_dependency_chain.rbegin()->first]->reg_id, false)));
                            for(auto riter= reg_dependency_chain.rbegin(); riter != reg_dependency_chain.rend(); riter++) {
                                if(riter->first == reg_dependency_chain.rbegin()->first)
                                    continue;
                                int arg_no = riter->second;
                                int src_reg_id = ival2interval[riter->first]->reg_id;
                                to_move_locs.push_back(std::make_pair(new RegLoc(arg_no+arg_reg_base, false), new RegLoc(src_reg_id, false)));
                                is_args_moved[arg_no] = true;        
                                reg2iargnos[src_reg_id].erase(arg_no);
                                if(reg2iargnos[src_reg_id].empty())
                                    reg2iargnos.erase(src_reg_id);
                            }
                            to_move_locs.push_back(std::make_pair(new RegLoc(reg_dependency_chain.rbegin()->second + arg_reg_base, false), new RegLoc(reg_s1, false)));
                            int arg_no = reg_dependency_chain.rbegin()->second;
                            int src_reg_id = ival2interval[reg_dependency_chain.rbegin()->first]->reg_id;
                            is_args_moved[arg_no] = true; 
                            reg2iargnos[src_reg_id].erase(arg_no);
                            if(reg2iargnos[src_reg_id].empty())
                                reg2iargnos.erase(src_reg_id);
                            reg_dependency_chain.clear();
                            break;
                        } 
                    }
                }
            } else {
                if(ival2interval.find(iargs[i]) == ival2interval.end()) {
                    to_move_locs.push_back(std::make_pair(new RegLoc(i+arg_reg_base, false), new ConstPool(dynamic_cast<ConstantInt*>(iargs[i])->getValue())));
                    is_args_moved[i] = true;
                    break;
                } else {
                    int src_reg_id = ival2interval[iargs[i]]->reg_id;
                    if(src_reg_id < 0) {
                        RegBase *regbase = val2stack[iargs[i]];
                        to_move_locs.push_back(std::make_pair(new RegLoc(i+arg_reg_base, false), regbase));
                        is_args_moved[i] = true;
                        break;
                    } else {
                        to_move_locs.push_back(std::make_pair(new RegLoc(i+arg_reg_base, false), new RegLoc(src_reg_id, false)));
                        is_args_moved[i] = true;
                        reg2iargnos[src_reg_id].erase(i);
                        if(reg2iargnos[src_reg_id].empty())
                            reg2iargnos.erase(src_reg_id);
                        if(!reg_dependency_chain.empty()) {
                            for(auto riter = reg_dependency_chain.rbegin(); riter != reg_dependency_chain.rend(); riter++) {
                                auto iarg = riter->first;
                                auto iargno = riter->second;
                                if(ival2interval.find(iargs[iargno]) == ival2interval.end()) {
                                    to_move_locs.push_back(std::make_pair(new RegLoc(iargno+arg_reg_base, false), new ConstPool(dynamic_cast<ConstantInt*>(iarg)->getValue())));
                                    is_args_moved[iargno] = true;
                                    break;
                                } else {
                                    int src_reg_id = ival2interval[iarg]->reg_id;
                                    if(src_reg_id < 0) {
                                        RegBase *regbase = val2stack[iarg];
                                        to_move_locs.push_back(std::make_pair(new RegLoc(iargno+arg_reg_base, false), regbase));
                                        is_args_moved[iargno] = true;
                                        break;
                                    } else {
                                        if(reg2iargnos.find(iargno + arg_reg_base) != reg2iargnos.end()) 
                                            break;
                                        to_move_locs.push_back(std::make_pair(new RegLoc(iargno+arg_reg_base, false), new RegLoc(src_reg_id, false)));
                                        is_args_moved[iargno] = true;
                                        reg2iargnos[src_reg_id].erase(iargno);
                                        if(reg2iargnos[src_reg_id].empty())
                                            reg2iargnos.erase(src_reg_id);
                                    }
                                }
                            }
                            reg_dependency_chain.clear();
                        }
                        break;
                    }
                }
            }
        }
    }
    return to_move_locs;
}


//! 初赛测试中未出现一个寄存器向多个寄存器中移动的情况
std::vector<std::pair<HAsmLoc*, HAsmLoc*>> CodeGen::caller_fargs_move(CallInst *call) {
    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_locs;

    std::vector<Value*> fargs;
    for(auto arg: call->getOperands()) {
        if(dynamic_cast<Function*>(arg))
            continue;
        if(arg->getType()->isFloatType()) 
            fargs.push_back(arg);
    }

    int num_of_fargs = fargs.size() > 8 ? 8 : fargs.size();

    std::map<int, bool> is_args_moved;

    std::map<int, std::set<int>> reg2fargnos;

    std::list<std::pair<Value*, int>> reg_dependency_chain;


    for(int i = 0; i < num_of_fargs; i++) {
        is_args_moved[i] = false;
        if(fval2interval.find(fargs[i]) != fval2interval.end()) {
            int reg_id = fval2interval[fargs[i]]->reg_id;
            if(reg_id >= 0) {
                if(reg2fargnos.find(reg_id) == reg2fargnos.end()) {
                    reg2fargnos.insert({reg_id, {i}});
                } else {
                    reg2fargnos[reg_id].insert(i);
                }
            }
        }
    }

    int extra_stack_offset =  cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset;

    //& 首先处理需要store到栈上的参数
    for(int i = fargs.size() - 1; i >= num_of_fargs; i--) {
        caller_trans_args_stack_offset -= reg_size;
        if(fval2interval.find(fargs[i]) != fval2interval.end()) {
            auto reg_id = fval2interval[fargs[i]]->reg_id;
            if(reg_id >= 0) {
                to_move_locs.push_back(std::make_pair(new RegBase(reg_sp, extra_stack_offset + caller_trans_args_stack_offset), new RegLoc(reg_id, true)));
            } else {
                to_move_locs.push_back(std::make_pair(new RegBase(reg_sp, extra_stack_offset + caller_trans_args_stack_offset), val2stack[fargs[i]]));
            }
        } else {
            auto const_fp = dynamic_cast<ConstantFP*>(fargs[i]);
            if(const_fp) {
                to_move_locs.push_back(std::make_pair(new RegBase(reg_sp, extra_stack_offset + caller_trans_args_stack_offset), new ConstPool(const_fp->getValue())));
            } else {

               // LOG(ERROR) << "汇编代码生成发生错误";
            }
        }
    }

    while(true) {
        int i = 0;
        //& 寻找一个尚未移动的参数
        for(auto &[arg, is_moved]: is_args_moved) {
            if(!is_moved) 
                break;
            i++;
        }
        //& all moved
        if(i == is_args_moved.size()) 
            break;
        while(true) {
            if(reg2fargnos.find(i + arg_reg_base) != reg2fargnos.end()) {
                if(reg2fargnos[i+arg_reg_base].find(i) != reg2fargnos[i+arg_reg_base].end()) {
                    if(reg2fargnos[i+arg_reg_base].size() == 1) {
                        is_args_moved[i] = true;
                        reg2fargnos[i+arg_reg_base].erase(i);
                        reg2fargnos.erase(i+arg_reg_base);
                    } else {
                        for(auto tmp_argno: reg2fargnos[i+arg_reg_base]) {
                            if(tmp_argno != i) {
                                i = tmp_argno;
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                } else {
                    reg_dependency_chain.push_back({fargs[i], i});
                    i = *reg2fargnos[i+arg_reg_base].begin();
                    if(i == reg_dependency_chain.begin()->second) {
                        //& found loop
                        //& check if or not no branches (a single circle)
                        bool is_single_circle = true; 
                        for(auto iter = reg_dependency_chain.begin(); iter != reg_dependency_chain.end(); iter++) {
                            if(reg2fargnos[iter->second + arg_reg_base].size() > 1) {
                                int next_argno;
                                if(iter->first == reg_dependency_chain.rbegin()->first) {
                                    next_argno = reg_dependency_chain.begin()->second;
                                } else {
                                    next_argno = (++iter)->second;
                                    iter--;
                                }
                                for(auto tmp_argno: reg2fargnos[iter->second + arg_reg_base]) {
                                    if(tmp_argno != next_argno) {
                                        i = tmp_argno;
                                        break;
                                    } else {
                                        continue;
                                    }
                                }
                                reg_dependency_chain.clear();
                                is_single_circle = false;
                                break;
                            }
                        }
                        if(is_single_circle) {
                            to_move_locs.push_back(std::make_pair(new RegLoc(reg_fs1, true), new RegLoc(fval2interval[reg_dependency_chain.rbegin()->first]->reg_id, true)));
                            for(auto riter= reg_dependency_chain.rbegin(); riter != reg_dependency_chain.rend(); riter++) {
                                if(riter->first == reg_dependency_chain.rbegin()->first)
                                    continue;
                                int arg_no = riter->second;
                                int src_reg_id = fval2interval[riter->first]->reg_id;
                                to_move_locs.push_back(std::make_pair(new RegLoc(arg_no+arg_reg_base, true), new RegLoc(src_reg_id, true)));
                                is_args_moved[arg_no] = true;        
                                reg2fargnos[src_reg_id].erase(arg_no);
                                if(reg2fargnos[src_reg_id].empty())
                                    reg2fargnos.erase(src_reg_id);
                            }
                            to_move_locs.push_back(std::make_pair(new RegLoc(reg_dependency_chain.rbegin()->second + arg_reg_base, true), new RegLoc(reg_fs1, true)));
                            int arg_no = reg_dependency_chain.rbegin()->second;
                            int src_reg_id = fval2interval[reg_dependency_chain.rbegin()->first]->reg_id;
                            is_args_moved[arg_no] = true; 
                            reg2fargnos[src_reg_id].erase(arg_no);
                            if(reg2fargnos[src_reg_id].empty())
                                reg2fargnos.erase(src_reg_id);
                            reg_dependency_chain.clear();
                            break;
                        } 
                    }
                }
            } else {
                if(fval2interval.find(fargs[i]) == fval2interval.end()) {
                    auto const_fp =  dynamic_cast<ConstantFP*>(fargs[i]);
                    to_move_locs.push_back(std::make_pair(new RegLoc(i + arg_reg_base, true), new ConstPool(const_fp->getValue())));
                    is_args_moved[i] = true;
                    break;
                } else {
                    int src_reg_id = fval2interval[fargs[i]]->reg_id;
                    if(src_reg_id < 0) {
                        RegBase *regbase = val2stack[fargs[i]];
                        to_move_locs.push_back(std::make_pair(new RegLoc(i + arg_reg_base, true), regbase));
                        is_args_moved[i] = true;
                        break;
                    } else {
                        to_move_locs.push_back(std::make_pair(new RegLoc(i + arg_reg_base, true), new RegLoc(src_reg_id, true)));
                        is_args_moved[i] = true;
                        reg2fargnos[src_reg_id].erase(i);
                        if(reg2fargnos[src_reg_id].empty())
                            reg2fargnos.erase(src_reg_id);
                        if(!reg_dependency_chain.empty()) {
                            for(auto riter = reg_dependency_chain.rbegin(); riter != reg_dependency_chain.rend(); riter++) {
                                auto farg = riter->first;
                                auto fargno = riter->second;
                                if(fval2interval.find(fargs[fargno]) == fval2interval.end()) {
                                    auto const_fp =  dynamic_cast<ConstantFP*>(farg);
                                    to_move_locs.push_back(std::make_pair(new RegLoc(fargno + arg_reg_base, true), new ConstPool(const_fp->getValue())));
                                    is_args_moved[fargno] = true;
                                    break;
                                } else {
                                    int src_reg_id = fval2interval[farg]->reg_id;
                                    if(src_reg_id < 0) {
                                        RegBase *regbase = val2stack[farg];
                                        to_move_locs.push_back(std::make_pair(new RegLoc(fargno + arg_reg_base, true), regbase));
                                        is_args_moved[fargno] = true;
                                        break;
                                    } else {
                                        if(reg2fargnos.find(fargno + arg_reg_base) != reg2fargnos.end()) 
                                            break;
                                        to_move_locs.push_back(std::make_pair(new RegLoc(fargno + arg_reg_base, true), new RegLoc(src_reg_id, true)));
                                        is_args_moved[fargno] = true;
                                        reg2fargnos[src_reg_id].erase(fargno);
                                        if(reg2fargnos[src_reg_id].empty())
                                            reg2fargnos.erase(src_reg_id);
                                    }
                                }
                            }
                            reg_dependency_chain.clear();
                        }
                        break;
                    }
                }
            }
        }
    }
    return to_move_locs;
}

void CodeGen::phi_union(Instruction *br_inst) {
    
    PhiDataMoveInst *succ_move_inst = nullptr;
    PhiDataMoveInst *fail_move_inst = nullptr;
    PhiDataMoveInst **move_inst;

    HAsmInst *succ_br_inst = nullptr;
    HAsmInst *fail_br_inst = nullptr;
    

    std::vector<HAsmLoc*> phi_itargets;
    std::vector<HAsmLoc*> phi_isrcs;
    std::vector<HAsmLoc*> phi_ftargets;
    std::vector<HAsmLoc*> phi_fsrcs;

    //& 保证寄存器地址的唯一性
    std::map<int, HAsmLoc*> ireg2loc;
    std::map<int, HAsmLoc*> freg2loc;

    bool is_cmpbr = false;
    bool is_fcmpbr = false;

    BranchInst *br = dynamic_cast<BranchInst*>(br_inst); 
    CmpBrInst *cmpbr = dynamic_cast<CmpBrInst*>(br_inst);
    FCmpBrInst *fcmpbr = dynamic_cast<FCmpBrInst*>(br_inst);

    BasicBlock *succ_bb = nullptr;
    BasicBlock *fail_bb = nullptr;

    bool have_succ_move = false;
    bool have_fail_move = false;

    tmp_regs_restore();

    if(fcmpbr) {
        is_fcmpbr = true;   
        succ_bb = dynamic_cast<BasicBlock*>(fcmpbr->getOperand(2));
        fail_bb = dynamic_cast<BasicBlock*>(fcmpbr->getOperand(3));
    } else if(cmpbr) {
        is_cmpbr = true;
        succ_bb = dynamic_cast<BasicBlock*>(cmpbr->getOperand(2));
        fail_bb = dynamic_cast<BasicBlock*>(cmpbr->getOperand(3));
    } else {
        if(br_inst->getNumOperands() == 1) {
            succ_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(0));
        } else {
            succ_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(1));
            fail_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(2));
        }
    }

    for(auto sux: cur_bb_->get_bb()->getSuccBasicBlocks()) {
        bool is_succ_bb;
        if(sux == succ_bb) {
            is_succ_bb = true;
            move_inst = &succ_move_inst;
        } else {
            is_succ_bb = false;
            move_inst = &fail_move_inst;
        }

        phi_isrcs.clear();
        phi_fsrcs.clear();
        phi_itargets.clear();
        phi_ftargets.clear();

        for(auto inst: sux->getInstructions()) {
            if(!inst->isPhi()) {
                break;
            }
            Value *lst_val = nullptr;
            if(inst->getType()->isFloatType()) {
                int target_reg_id = fval2interval[inst]->reg_id;
                HAsmLoc *target_loc_ptr = nullptr;
                if(target_reg_id >= 0) {
                    if(freg2loc.find(target_reg_id) == freg2loc.end()) 
                        freg2loc.insert({target_reg_id, new RegLoc(target_reg_id, true)});
                    target_loc_ptr = freg2loc[target_reg_id];
                } else {
                    target_loc_ptr = val2stack[inst];
                }
                for(auto opr: inst->getOperands()) {
                    if(dynamic_cast<BasicBlock*>(opr)) {
                        auto this_bb = dynamic_cast<BasicBlock*>(opr);
                        if(this_bb != cur_bb_->get_bb())
                            continue;
                        if(dynamic_cast<ConstantFP*>(lst_val)) {
                            auto const_val = dynamic_cast<ConstantFP*>(lst_val);
                            auto src = new ConstPool(const_val->getValue());
                            phi_fsrcs.push_back(src);
                            phi_ftargets.push_back(target_loc_ptr);
                        } else {
                            int src_reg_id = fval2interval[lst_val]->reg_id;
                            if(src_reg_id >= 0) {
                                if(freg2loc.find(src_reg_id) == freg2loc.end())
                                    freg2loc.insert({src_reg_id, new RegLoc(src_reg_id, true)});
                                auto src = freg2loc[src_reg_id];
                                phi_fsrcs.push_back(src);
                                phi_ftargets.push_back(target_loc_ptr);
                            } else {
                                auto src = val2stack[lst_val];
                                phi_fsrcs.push_back(src);
                                phi_ftargets.push_back(target_loc_ptr);
                            }
                        }
                    } else {
                        if(opr == nullptr)
                          std::cout<<"err"<<std::endl;  //这里有个if，必须有语句，所以加里这句话
                           // LOG(ERROR) << "err";
                        lst_val = opr;
                    }
                }
            } else {
                int target_reg_id = ival2interval[inst]->reg_id;
                HAsmLoc *target_loc_ptr = nullptr;
                if(target_reg_id >= 0) {
                    if(ireg2loc.find(target_reg_id) == ireg2loc.end()) 
                        ireg2loc.insert({target_reg_id, new RegLoc(target_reg_id, false)});
                    target_loc_ptr = ireg2loc[target_reg_id];
                } else {
                    target_loc_ptr = val2stack[inst];
                }
                for(auto opr: inst->getOperands()) {
                    if(dynamic_cast<BasicBlock*>(opr)) {
                        auto this_bb = dynamic_cast<BasicBlock*>(opr);
                        if(this_bb != cur_bb_->get_bb()) 
                            continue;
                        if(dynamic_cast<ConstantInt*>(lst_val)) {
                            auto const_val = dynamic_cast<ConstantInt*>(lst_val);
                            auto src = new ConstPool(const_val->getValue());
                            phi_isrcs.push_back(src);
                            phi_itargets.push_back(target_loc_ptr);
                        } else {
                            int src_reg_id = ival2interval[lst_val]->reg_id;
                            if(src_reg_id >= 0) {
                                if(ireg2loc.find(src_reg_id) == ireg2loc.end()) 
                                    ireg2loc.insert({src_reg_id, new RegLoc(src_reg_id, false)});
                                auto src = ireg2loc[src_reg_id];
                                phi_isrcs.push_back(src);
                                phi_itargets.push_back(target_loc_ptr);
                            } else {
                                auto src = val2stack[lst_val];
                                phi_isrcs.push_back(src);
                                phi_itargets.push_back(target_loc_ptr);
                            }
                        }
                    } else {
                        lst_val = opr;
                    }
                }
            }


        }

        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_ilocs;
        std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_flocs;

        if(!phi_isrcs.empty()) {
            to_move_ilocs = idata_move(phi_isrcs, phi_itargets);

            if(is_succ_bb) {
                have_succ_move = true;
            } else {
                have_fail_move = true;
            }
        }
        if(!phi_fsrcs.empty()) {
            to_move_flocs = fdata_move(phi_fsrcs, phi_ftargets);
            if(is_succ_bb) {
                have_succ_move = true;
            } else {
                have_fail_move = true;
            }
        }

        if(! to_move_ilocs.empty() || ! to_move_flocs.empty()) {
            *move_inst = cur_bb_->create_phi_data_move(to_move_ilocs, to_move_flocs);
            cur_bb_->pop_inst();
        }
    }

    if(fcmpbr) {
        is_fcmpbr = true;   
        auto cond1 = fcmpbr->getOperand(0);
        auto cond2 = fcmpbr->getOperand(1);
        auto cmp_op = fcmpbr->getCmpOp();
        succ_bb = dynamic_cast<BasicBlock*>(fcmpbr->getOperand(2));
        fail_bb = dynamic_cast<BasicBlock*>(fcmpbr->getOperand(3));
        auto const_cond1 = dynamic_cast<ConstantFP*>(cond1);
        auto const_cond2 = dynamic_cast<ConstantFP*>(cond2);

        switch(cmp_op) {
            case CmpOp::EQ: {
                    //& 为了避免修改控制流可能造成的问题，不采取化简
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_fbeq(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbeq(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbeq(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fbeq(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbeq(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg_id < 0 && fval2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbeq(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fbeq(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbeq(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbeq(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();
                }
                break;

            case CmpOp::GE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_fbge(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbge(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbge(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fbge(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbge(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg_id < 0 && fval2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbge(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fbge(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbge(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbge(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();
                }
                break;
            case CmpOp::GT: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_fbgt(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbgt(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbgt(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fbgt(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbgt(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg_id < 0 && fval2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbgt(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fbgt(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbgt(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbgt(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();
                }
                break; 

            case CmpOp::LE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_fble(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fble(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fble(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fble(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fble(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg_id < 0 && fval2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fble(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fble(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fble(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fble(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();
                }
                break;
            
            case CmpOp::LT: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_fblt(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fblt(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fblt(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fblt(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fblt(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg_id < 0 && fval2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fblt(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fblt(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fblt(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fblt(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();
                }
                break;

            case CmpOp::NE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_fbne(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbne(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbne(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fbne(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbne(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg_id < 0 && fval2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbne(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_fbne(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(fval2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_fbne(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_fbne(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();
                }
                break; 

            default:
                break;
        }
    } else if(cmpbr) {
        is_cmpbr = true;
        auto cond1 = cmpbr->getOperand(0);
        auto cond2 = cmpbr->getOperand(1);
        auto cmp_op = cmpbr->getCmpOp();
        succ_bb = dynamic_cast<BasicBlock*>(cmpbr->getOperand(2));
        fail_bb = dynamic_cast<BasicBlock*>(cmpbr->getOperand(3));
        auto const_cond1 = dynamic_cast<ConstantInt*>(cond1);
        auto const_cond2 = dynamic_cast<ConstantInt*>(cond2);

        switch(cmp_op) {
            case CmpOp::EQ: {
                    //& 为了避免修改控制流可能造成的问题，不采取化简
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_beq(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                  
                    } else if(const_cond1) {
                        if(ival2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_beq(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_beq(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(ival2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_beq(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_beq(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(ival2interval[cond1]->reg_id < 0 && ival2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_beq(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(ival2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_beq(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(ival2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_beq(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_beq(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();  
                }
                break;

            case CmpOp::GE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_bge(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                  
                    } else if(const_cond1) {
                        if(ival2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_bge(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_bge(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(ival2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_bge(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_bge(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(ival2interval[cond1]->reg_id < 0 && ival2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_bge(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(ival2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_bge(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(ival2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_bge(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_bge(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();  
                }
                break;
            case CmpOp::GT: 
               // LOG(ERROR) << "phi union出现异常";
                break; 

            case CmpOp::LE: 
               // LOG(ERROR) << "phi union出现异常";
                break;
            
            case CmpOp::LT: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_blt(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                  
                    } else if(const_cond1) {
                        if(ival2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_blt(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_blt(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(ival2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_blt(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_blt(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(ival2interval[cond1]->reg_id < 0 && ival2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_blt(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(ival2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_blt(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(ival2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_blt(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_blt(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();  
                }
                break;

            case CmpOp::NE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = cur_bb_->create_bne(new Const(const_cond1->getValue()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                        cur_bb_->pop_inst();                  
                    } else if(const_cond1) {
                        if(ival2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_bne(new Const(const_cond1->getValue()), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_bne(new Const(const_cond1->getValue()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else if(const_cond2) {
                        if(ival2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_bne(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_bne(get_asm_reg(cond1), new Const(const_cond2->getValue()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    } else {
                        if(ival2interval[cond1]->reg_id < 0 && ival2interval[cond2]->reg_id < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_bne(new Mem(regbase1->get_reg_id(), regbase1->get_offset()), new Mem(regbase2->get_reg_id(), regbase2->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(ival2interval[cond1]->reg_id < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = cur_bb_->create_bne(new Mem(regbase->get_reg_id(), regbase->get_offset()), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else if(ival2interval[cond2]->reg_id < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = cur_bb_->create_bne(get_asm_reg(cond1), new Mem(regbase->get_reg_id(), regbase->get_offset()), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        } else {
                            succ_br_inst = cur_bb_->create_bne(get_asm_reg(cond1), get_asm_reg(cond2), bb2label[succ_bb]);
                            cur_bb_->pop_inst();
                        }
                    }
                    fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
                    cur_bb_->pop_inst();  
                }
                break;

            default:
                break;
        }
    } else {
        if(br_inst->getNumOperands() == 1) {
            succ_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(0));
            succ_br_inst = cur_bb_->create_j(bb2label[succ_bb]);
            cur_bb_->pop_inst();
        } else {
            succ_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(1));
            fail_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(2));
            auto cond = br_inst->getOperand(0);
            auto const_cond = dynamic_cast<ConstantInt*>(cond);
            if(const_cond) {
                succ_br_inst = cur_bb_->create_bne(new Const(const_cond->getValue()), new Reg(reg_zero, false), bb2label[succ_bb]);
                cur_bb_->pop_inst();
            } else if(ival2interval[cond]->reg_id < 0) {
                auto regbase = val2stack[cond];
                succ_br_inst = cur_bb_->create_bne(new Mem(regbase->get_reg_id(), regbase->get_offset()), new Reg(reg_zero, false), bb2label[succ_bb]);
                cur_bb_->pop_inst();
            } else {
                succ_br_inst = cur_bb_->create_bne(get_asm_reg(cond), new Reg(reg_zero, false), bb2label[succ_bb]);
                cur_bb_->pop_inst();
            }
            fail_br_inst = cur_bb_->create_j(bb2label[fail_bb]);
            cur_bb_->pop_inst();
        }
    }

    if(is_fcmpbr) {
        if(have_succ_move) 
        std::cout<<"出现未预期情况"<<std::endl;
           // LOG(ERROR) << "出现未预期情况";
        if(succ_br_inst)
            cur_bb_->push_inst(succ_br_inst);
        if(fail_move_inst)
            cur_bb_->push_inst(fail_move_inst);
        if(fail_br_inst)
            cur_bb_->push_inst(fail_br_inst);
        
    } else if(is_cmpbr) {
        if(have_succ_move) 
        std::cout<<"出现未预期情况"<<std::endl;

        if(succ_br_inst)
            cur_bb_->push_inst(succ_br_inst);
        if(fail_move_inst)
            cur_bb_->push_inst(fail_move_inst);
        if(fail_br_inst)
            cur_bb_->push_inst(fail_br_inst);

    } else {
        if(br_inst->getNumOperands() == 1) {
            if(succ_move_inst)
                cur_bb_->push_inst(succ_move_inst);
            if(succ_br_inst)
                cur_bb_->push_inst(succ_br_inst);

        } else {
            if(have_succ_move) 
            std::cout<<"出现未预期情况"<<std::endl;
                //LOG(ERROR) << "出现未预期情况";

            if(succ_br_inst)
                cur_bb_->push_inst(succ_br_inst);
            if(fail_move_inst)
                cur_bb_->push_inst(fail_move_inst);
            if(fail_br_inst)
                cur_bb_->push_inst(fail_br_inst);
        }
    }
}

std::vector<std::pair<HAsmLoc*, HAsmLoc*>> CodeGen::idata_move(std::vector<HAsmLoc*>&srcs, std::vector<HAsmLoc*>&dsts) {
 
    std::map<int, bool> is_data_moved;
    std::list<int> loc_dependency_chain;
    std::map<HAsmLoc*, std::set<int>> src2dstnos;

    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_locs;

    for(int i = 0; i < srcs.size(); i++) {
        is_data_moved[i] = false;
        if(src2dstnos.find(srcs[i]) == src2dstnos.end()) {
            src2dstnos.insert({srcs[i], {i}});
        } else {
            src2dstnos[srcs[i]].insert(i);
        }
    }

    while(true) {
        int i = 0;
        //& 寻找一个尚未移动的数据
        for(auto &[data, is_moved]: is_data_moved) {
            if(!is_moved)
                break;
            i++;
        }

        //& all moved
        if(i == is_data_moved.size())
            break;

        while(true) {
            if(src2dstnos.find(dsts[i]) != src2dstnos.end()) {
                if(src2dstnos[dsts[i]].find(i) != src2dstnos[dsts[i]].end()) {
                    if(src2dstnos[dsts[i]].size() == 1) {
                        is_data_moved[i] = true;
                        src2dstnos[dsts[i]].erase(i);
                        src2dstnos.erase(dsts[i]);
                    } else {
                        for(auto tmp_dstno: src2dstnos[dsts[i]]) {
                            if(tmp_dstno != i) {
                                i = tmp_dstno;
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                } else {
                    loc_dependency_chain.push_back(i);
                    i = *src2dstnos[dsts[i]].begin();
                    if(i == *loc_dependency_chain.begin()) {
                        //& found loop
                        //& check if or not no branches (a single circle)
                        bool is_single_circle = true;
                        for(auto iter = loc_dependency_chain.begin(); iter != loc_dependency_chain.end(); iter++) {
                            if(src2dstnos[dsts[*iter]].size() > 1) {
                                int next_dstno;
                                if(*iter == *loc_dependency_chain.rbegin()) {
                                    next_dstno = *loc_dependency_chain.begin();
                                } else {
                                    next_dstno = *(++iter);
                                    iter--;
                                }
                                for(auto tmp_dstno: src2dstnos[dsts[*iter]]) {
                                    if(tmp_dstno != next_dstno) {
                                        i = tmp_dstno;
                                        break;
                                    } else {
                                        continue;
                                    }
                                }
                                loc_dependency_chain.clear();
                                is_single_circle = false;
                                break;
                            }
                        }
                        if(is_single_circle) {
                            to_move_locs.push_back(std::make_pair(new RegLoc(reg_s1, false) , srcs[*loc_dependency_chain.rbegin()]));
                            for(auto riter = loc_dependency_chain.rbegin(); riter != loc_dependency_chain.rend(); riter++) {
                                if(*riter == *loc_dependency_chain.rbegin())
                                    continue;
                                to_move_locs.push_back(std::make_pair(dsts[*riter] , srcs[*riter]));
                                is_data_moved[*riter] = true;
                                src2dstnos[srcs[*riter]].erase(*riter);
                                if(src2dstnos[srcs[*riter]].empty()) {
                                    src2dstnos.erase(srcs[*riter]);
                                }  
                            }
                            to_move_locs.push_back(std::make_pair(dsts[*loc_dependency_chain.rbegin()] , new RegLoc(reg_s1, false)));
                            int tmp_no = *loc_dependency_chain.rbegin();
                            is_data_moved[tmp_no] = true;
                            src2dstnos[srcs[tmp_no]].erase(tmp_no);
                            if(src2dstnos[srcs[tmp_no]].empty()) {
                                src2dstnos.erase(srcs[tmp_no]);
                            }
                            loc_dependency_chain.clear();
                            break;
                        }
                    }
                }
            } else {
                to_move_locs.push_back(std::make_pair(dsts[i], srcs[i]));
                is_data_moved[i] = true;
                src2dstnos[srcs[i]].erase(i);
                if(src2dstnos[srcs[i]].empty()) {
                    src2dstnos.erase(srcs[i]);
                }
                if(!loc_dependency_chain.empty()) {
                    for(auto riter = loc_dependency_chain.rbegin(); riter != loc_dependency_chain.rend(); riter++) {
                        if(src2dstnos.find(dsts[*riter]) != src2dstnos.end()) 
                            break;
                        to_move_locs.push_back(std::make_pair(dsts[*riter], srcs[*riter]));
                        is_data_moved[*riter] = true;
                        src2dstnos[srcs[*riter]].erase(*riter);
                        if(src2dstnos[srcs[*riter]].empty()) {
                            src2dstnos.erase(srcs[*riter]);
                        }   
                    }
                    loc_dependency_chain.clear();
                }
                break;
            }
        }
    }

    return to_move_locs;
}

std::vector<std::pair<HAsmLoc*, HAsmLoc*>> CodeGen::fdata_move(std::vector<HAsmLoc*>&srcs, std::vector<HAsmLoc*>&dsts) {
 
    std::map<int, bool> is_data_moved;
    std::list<int> loc_dependency_chain;
    std::map<HAsmLoc*, std::set<int>> src2dstnos;

    std::vector<std::pair<HAsmLoc*, HAsmLoc*>> to_move_locs;

    for(int i = 0; i < srcs.size(); i++) {
        is_data_moved[i] = false;
        if(src2dstnos.find(srcs[i]) == src2dstnos.end()) {
            src2dstnos.insert({srcs[i], {i}});
        } else {
            src2dstnos[srcs[i]].insert(i);
        }
    }

    while(true) {
        int i = 0;
        //& 寻找一个尚未移动的数据
        for(auto &[data, is_moved]: is_data_moved) {
            if(!is_moved)
                break;
            i++;
        }

        //& all moved
        if(i == is_data_moved.size())
            break;

        while(true) {
            if(src2dstnos.find(dsts[i]) != src2dstnos.end()) {
                if(src2dstnos[dsts[i]].find(i) != src2dstnos[dsts[i]].end()) {
                    if(src2dstnos[dsts[i]].size() == 1) {
                        is_data_moved[i] = true;
                        src2dstnos[dsts[i]].erase(i);
                        src2dstnos.erase(dsts[i]);
                    } else {
                        for(auto tmp_dstno: src2dstnos[dsts[i]]) {
                            if(tmp_dstno != i) {
                                i = tmp_dstno;
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                } else {
                    loc_dependency_chain.push_back(i);
                    i = *src2dstnos[dsts[i]].begin();
                    if(i == *loc_dependency_chain.begin()) {
                        //& found loop
                        //& check if or not no branches (a single circle)
                        bool is_single_circle = true;
                        for(auto iter = loc_dependency_chain.begin(); iter != loc_dependency_chain.end(); iter++) {
                            if(src2dstnos[dsts[*iter]].size() > 1) {
                                int next_dstno;
                                if(*iter == *loc_dependency_chain.rbegin()) {
                                    next_dstno = *loc_dependency_chain.begin();
                                } else {
                                    next_dstno = *(++iter);
                                    iter--;
                                }
                                for(auto tmp_dstno: src2dstnos[dsts[*iter]]) {
                                    if(tmp_dstno != next_dstno) {
                                        i = tmp_dstno;
                                        break;
                                    } else {
                                        continue;
                                    }
                                }
                                loc_dependency_chain.clear();
                                is_single_circle = false;
                                break;
                            }
                        }
                        if(is_single_circle) {
                            to_move_locs.push_back(std::make_pair(new RegLoc(reg_fs1, true), srcs[*loc_dependency_chain.rbegin()]));
                            for(auto riter = loc_dependency_chain.rbegin(); riter != loc_dependency_chain.rend(); riter++) {
                                if(*riter == *loc_dependency_chain.rbegin())
                                    continue;
                                to_move_locs.push_back(std::make_pair(dsts[*riter], srcs[*riter]));
                                is_data_moved[*riter] = true;
                                src2dstnos[srcs[*riter]].erase(*riter);
                                if(src2dstnos[srcs[*riter]].empty()) {
                                    src2dstnos.erase(srcs[*riter]);
                                }  
                            }
                            to_move_locs.push_back(std::make_pair(dsts[*loc_dependency_chain.rbegin()], new RegLoc(reg_s1, true)));
                            int tmp_no = *loc_dependency_chain.rbegin();
                            is_data_moved[tmp_no] = true;
                            src2dstnos[srcs[tmp_no]].erase(tmp_no);
                            if(src2dstnos[srcs[tmp_no]].empty()) {
                                src2dstnos.erase(srcs[tmp_no]);
                            }
                            loc_dependency_chain.clear();
                            break;
                        }
                    }
                }
            } else {
                to_move_locs.push_back(std::make_pair(dsts[i], srcs[i]));
                is_data_moved[i] = true;
                src2dstnos[srcs[i]].erase(i);
                if(src2dstnos[srcs[i]].empty()) {
                    src2dstnos.erase(srcs[i]);
                }
                if(!loc_dependency_chain.empty()) {
                    for(auto riter = loc_dependency_chain.rbegin(); riter != loc_dependency_chain.rend(); riter++) {
                        if(src2dstnos.find(dsts[*riter]) != src2dstnos.end()) 
                            break;
                        to_move_locs.push_back(std::make_pair(dsts[*riter], srcs[*riter]));
                        is_data_moved[*riter] = true;
                        src2dstnos[srcs[*riter]].erase(*riter);
                        if(src2dstnos[srcs[*riter]].empty()) {
                            src2dstnos.erase(srcs[*riter]);
                        }   
                    }
                    loc_dependency_chain.clear();
                }
                break;
            }
        }
    }

    return to_move_locs;
}

Reg *CodeGen::get_asm_reg(Value *val) {
    if(val->getType()->isFloatType()) {
        auto iter = fval2interval.find(val);
        if(iter != fval2interval.end()) {
            return new Reg(iter->second->reg_id, true);
        } else {
           // LOG(ERROR) << "该值不存在活跃区间";
        }
    } else {
        auto iter = ival2interval.find(val);
        if(iter != ival2interval.end()) {
            return new Reg(iter->second->reg_id, false);
        } else {
           // LOG(ERROR) << "该值不存在活跃区间";
        }
    }
    return nullptr;
}

void CodeGen::instr_gen(Instruction * inst) {
    auto instr_type = inst->getInstrType();

    switch(instr_type) {
        case Instruction::add: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_add(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_add(get_asm_reg(inst), get_asm_reg(op2), new Const(const_op1->getValue()));
                } else if(const_op2) {
                    cur_bb_->create_add(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_add(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;
        case Instruction::sub: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_subw(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_subw(get_asm_reg(inst), new Const(const_op1->getValue()), get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_subw(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_subw(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;
        case Instruction::mul: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_mulw(get_asm_reg(inst), new Const(const_op1->getValue()),  new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_mulw(get_asm_reg(inst), new Const(const_op1->getValue()), get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_mulw(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_mulw(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;
        case Instruction::mul64: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op1 && const_op2) {
                    std::cout<<"无法处理"<<std::endl;
                    //LOG(ERROR) << "无法处理";
                } else if(const_op1) {
                    cur_bb_->create_mul64(get_asm_reg(inst), new Const(const_op1->getValue()), get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_mul64(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_mul64(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;
        case Instruction::sdiv: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_divw(get_asm_reg(inst), new Const(const_op1->getValue()),  new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_divw(get_asm_reg(inst), new Const(const_op1->getValue()),  get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_divw(get_asm_reg(inst), get_asm_reg(op1),  new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_divw(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;

        case Instruction::srem: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_remw(get_asm_reg(inst), new Const(const_op1->getValue()),  new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_remw(get_asm_reg(inst), new Const(const_op1->getValue()),  get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_remw(get_asm_reg(inst), get_asm_reg(op1),  new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_remw(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;

        case Instruction::asr: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op2 == nullptr)
                std::cout<<"出现未预期情况"<<std::endl;
                    //LOG(ERROR) << "出现未预期情况";
                if(const_op1) {
                    cur_bb_->create_sraw(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_sraw(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                }
            }
            break;

        case Instruction::shl: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op2 == nullptr)
                    std::cout<<"出现未预期情况"<<std::endl;
                    //LOG(ERROR) << ;
                if(const_op1) {
                    cur_bb_->create_sllw(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_sllw(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                }
            }
            break;
        case Instruction::lsr: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op2 == nullptr)
                    std::cout<< "出现未预期情况"<<std::endl;
                  //  LOG(ERROR) <<;
                if(const_op1) {
                    cur_bb_->create_srlw(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_srlw(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                }
            }
            break;

        case Instruction::asr64: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op2 == nullptr)
                    std::cout<<"出现未预期情况"<<std::endl;
                  //  LOG(ERROR) << ;
                if(const_op1) {
                    cur_bb_->create_sra(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_sra(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                }
            }
            break;

        case Instruction::shl64: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op2 == nullptr)
                    std::cout<<"出现未预期情况"<<std::endl;
                    //LOG(ERROR) << ;
                if(const_op1) {
                    cur_bb_->create_sll(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_sll(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                }
            }
            break;
        case Instruction::lsr64: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantInt*>(op1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op2 == nullptr)
                    std::cout<<"出现未预期情况"<<std::endl;
                   // LOG(ERROR) << ;
                if(const_op1) {
                    cur_bb_->create_srl(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_srl(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                }
            }
            break;

        case Instruction::land: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op2 = dynamic_cast<ConstantInt*>(op2);
                if(const_op2 == nullptr)
                    std::cout<<"出现未预期情况"<<std::endl;
                   // LOG(ERROR) << ;
                cur_bb_->create_land(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
            }
            break;

        case Instruction::fadd: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantFP*>(op1);
                auto const_op2 = dynamic_cast<ConstantFP*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_fadds(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_fadds(get_asm_reg(inst), new Const(const_op1->getValue()), get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_fadds(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_fadds(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;
        case Instruction::fsub: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantFP*>(op1);
                auto const_op2 = dynamic_cast<ConstantFP*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_fsubs(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_fsubs(get_asm_reg(inst), new Const(const_op1->getValue()), get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_fsubs(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_fsubs(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;
        case Instruction::fmul: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantFP*>(op1);
                auto const_op2 = dynamic_cast<ConstantFP*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_fmuls(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_fmuls(get_asm_reg(inst), new Const(const_op1->getValue()), get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_fmuls(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_fmuls(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;
        case Instruction::fdiv: {
                auto op1 = inst->getOperand(0);
                auto op2 = inst->getOperand(1);
                auto const_op1 = dynamic_cast<ConstantFP*>(op1);
                auto const_op2 = dynamic_cast<ConstantFP*>(op2);
                if(const_op1 && const_op2) {
                    cur_bb_->create_fdivs(get_asm_reg(inst), new Const(const_op1->getValue()), new Const(const_op2->getValue()));
                } else if(const_op1) {
                    cur_bb_->create_fdivs(get_asm_reg(inst), new Const(const_op1->getValue()), get_asm_reg(op2));
                } else if(const_op2) {
                    cur_bb_->create_fdivs(get_asm_reg(inst), get_asm_reg(op1), new Const(const_op2->getValue()));
                } else {
                    cur_bb_->create_fdivs(get_asm_reg(inst), get_asm_reg(op1), get_asm_reg(op2));
                }
            }
            break;
        case Instruction::fptosi:{
                auto src = inst->getOperand(0);
                auto const_src = dynamic_cast<ConstantFP*>(src);
                if(const_src) {
                    cur_bb_->create_fcvtws(get_asm_reg(inst), new Const(const_src->getValue()));
                } else {
                    cur_bb_->create_fcvtws(get_asm_reg(inst), get_asm_reg(src));
                }
            }
            break;
        case Instruction::sitofp: {
                auto src = inst->getOperand(0);
                auto const_src = dynamic_cast<ConstantInt*>(src);
                if(const_src) {
                    cur_bb_->create_fcvtsw(get_asm_reg(inst), new Const(const_src->getValue()));
                } else {
                    cur_bb_->create_fcvtsw(get_asm_reg(inst), get_asm_reg(src));
                }
            }
            break;
        case Instruction::zext: 
            cur_bb_->create_zext(get_asm_reg(inst), get_asm_reg(inst->getOperand(0)));
            break;
        //& 由于sysY文法中关系表达式只存在于条件判断中，在IR中紧接着就会有跳转指令,经过LIR后应该不再存在单独cmp或fcmp指令
        case Instruction::cmp: {
                auto cmp_inst = dynamic_cast<CmpInst*>(inst);
                auto cmp_op = cmp_inst->getCmpOp();
                auto cond1 = cmp_inst->getOperand(0);
                auto cond2 = cmp_inst->getOperand(1);
                auto const_cond1 = dynamic_cast<ConstantInt*>(cond1);
                auto const_cond2 = dynamic_cast<ConstantInt*>(cond2);
                
                if(const_cond2 && const_cond2->getValue() == 0) {
                    switch(cmp_op) {
                        case CmpOp::EQ:
                            if(const_cond1) {
                                cur_bb_->create_seqz(get_asm_reg(inst), new Const(const_cond1->getValue()));
                            } else {
                                cur_bb_->create_seqz(get_asm_reg(inst), get_asm_reg(cond1));
                            }
                            break;
                        case CmpOp::NE:
                            if(const_cond1) {
                                cur_bb_->create_snez(get_asm_reg(inst), new Const(const_cond1->getValue()));
                            } else {
                                cur_bb_->create_snez(get_asm_reg(inst), get_asm_reg(cond1));
                            }
                            break;
                        default:
                          //  LOG(ERROR) << "汇编代码生成出现异常";
                            break;
                    }
                } else {
                    //LOG(ERROR) << "汇编代码生成出现异常";
                }
            }
            break;
            

        case Instruction::fcmp: {
                auto cond1 = inst->getOperand(0);
                auto cond2 = inst->getOperand(1);
                auto cmp_op = (dynamic_cast<FCmpInst*>(inst))->getCmpOp();
                auto const_cond1 = dynamic_cast<ConstantFP*>(cond1);
                auto const_cond2 = dynamic_cast<ConstantFP*>(cond2);
                switch (cmp_op) {
                    case CmpOp::EQ: {
                            if(const_cond1 && const_cond2) {
                                cur_bb_->create_feqs(get_asm_reg(inst), new Const(const_cond1->getValue()), new Const(const_cond2->getValue()));
                            } else if(const_cond1) {
                                cur_bb_->create_feqs(get_asm_reg(inst), new Const(const_cond1->getValue()), get_asm_reg(cond2));
                            } else if(const_cond2) {
                                cur_bb_->create_feqs(get_asm_reg(inst), get_asm_reg(cond1), new Const(const_cond2->getValue()));
                            } else {
                                cur_bb_->create_feqs(get_asm_reg(inst), get_asm_reg(cond1), get_asm_reg(cond2));
                            }
                        }
                        break;
                    case CmpOp::GE: {
                            if(const_cond1 && const_cond2) {
                                cur_bb_->create_fges(get_asm_reg(inst), new Const(const_cond1->getValue()), new Const(const_cond2->getValue()));
                            } else if(const_cond1) {
                                cur_bb_->create_fges(get_asm_reg(inst), new Const(const_cond1->getValue()), get_asm_reg(cond2));
                            } else if(const_cond2) {
                                cur_bb_->create_fges(get_asm_reg(inst), get_asm_reg(cond1), new Const(const_cond2->getValue()));
                            } else {
                                cur_bb_->create_fges(get_asm_reg(inst), get_asm_reg(cond1), get_asm_reg(cond2));
                            }
                        }
                        break;
                    case CmpOp::GT: {
                            if(const_cond1 && const_cond2) {
                                cur_bb_->create_fgts(get_asm_reg(inst), new Const(const_cond1->getValue()), new Const(const_cond2->getValue()));
                            } else if(const_cond1) {
                                cur_bb_->create_fgts(get_asm_reg(inst), new Const(const_cond1->getValue()), get_asm_reg(cond2));
                            } else if(const_cond2) {
                                cur_bb_->create_fgts(get_asm_reg(inst), get_asm_reg(cond1), new Const(const_cond2->getValue()));
                            } else {
                                cur_bb_->create_fgts(get_asm_reg(inst), get_asm_reg(cond1), get_asm_reg(cond2));
                            }
                        }
                        break;

                    case CmpOp::LE: {
                            if(const_cond1 && const_cond2) {
                                cur_bb_->create_fles(get_asm_reg(inst), new Const(const_cond1->getValue()), new Const(const_cond2->getValue()));
                            } else if(const_cond1) {
                                cur_bb_->create_fles(get_asm_reg(inst), new Const(const_cond1->getValue()), get_asm_reg(cond2));
                            } else if(const_cond2) {
                                cur_bb_->create_fles(get_asm_reg(inst), get_asm_reg(cond1), new Const(const_cond2->getValue()));
                            } else {
                                cur_bb_->create_fles(get_asm_reg(inst), get_asm_reg(cond1), get_asm_reg(cond2));
                            }
                        }
                        break;

                    case CmpOp::LT: {
                            if(const_cond1 && const_cond2) {
                                cur_bb_->create_flts(get_asm_reg(inst), new Const(const_cond1->getValue()), new Const(const_cond2->getValue()));
                            } else if(const_cond1) {
                                cur_bb_->create_flts(get_asm_reg(inst), new Const(const_cond1->getValue()), get_asm_reg(cond2));
                            } else if(const_cond2) {
                                cur_bb_->create_flts(get_asm_reg(inst), get_asm_reg(cond1), new Const(const_cond2->getValue()));
                            } else {
                                cur_bb_->create_flts(get_asm_reg(inst), get_asm_reg(cond1), get_asm_reg(cond2));
                            }
                        }
                        break;

                    case CmpOp::NE: {
                            if(const_cond1 && const_cond2) {
                                cur_bb_->create_fnes(get_asm_reg(inst), new Const(const_cond1->getValue()), new Const(const_cond2->getValue()));
                            } else if(const_cond1) {
                                cur_bb_->create_fnes(get_asm_reg(inst), new Const(const_cond1->getValue()), get_asm_reg(cond2));
                            } else if(const_cond2) {
                                cur_bb_->create_fnes(get_asm_reg(inst), get_asm_reg(cond1), new Const(const_cond2->getValue()));
                            } else {
                                cur_bb_->create_fnes(get_asm_reg(inst), get_asm_reg(cond1), get_asm_reg(cond2));
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            break;

        case Instruction::load: {
                auto global_addr = dynamic_cast<GlobalVariable*>(inst->getOperand(0));
                if(global_addr) {
                    if(global_addr->getType()->getPointerElementType()->isFloatType()) {
                        cur_bb_->create_flw(get_asm_reg(inst), global_variable_labels_table[global_addr]);
                    } else {
                        cur_bb_->create_lw(get_asm_reg(inst), global_variable_labels_table[global_addr]);
                    }
                } else {
                    //LOG(ERROR) << "汇编代码生成出现异常";
                }
            }
            break;
        case Instruction::loadoffset: {
                auto loadoffset_inst = dynamic_cast<LoadOffsetInst*>(inst);
                if(! dynamic_cast<GetElementPtrInst*>(loadoffset_inst->getOperand(0))) {
                    //LOG(ERROR) << "汇编代码生成出现未预期情况";
                }
                auto offset = loadoffset_inst->getOffset();
                auto const_offset = dynamic_cast<ConstantInt*>(offset);
                if(loadoffset_inst->getLoadType()->isFloatType()) {
                    if(const_offset) {
                        cur_bb_->create_flw(get_asm_reg(inst), get_asm_reg(inst->getOperand(0)), new Const(const_offset->getValue()));
                    } else {
                        cur_bb_->create_flw(get_asm_reg(inst), get_asm_reg(inst->getOperand(0)), get_asm_reg(offset));
                    }
                } else {
                    if(const_offset) {
                        cur_bb_->create_lw(get_asm_reg(inst), get_asm_reg(inst->getOperand(0)), new Const(const_offset->getValue()));
                    } else {
                        cur_bb_->create_lw(get_asm_reg(inst), get_asm_reg(inst->getOperand(0)), get_asm_reg(offset));
                    }
                }
            }
            break;
        
        //& 由于mem2reg和LIR，load或store指令只会出现在load或store一个全局变量的值时
        case Instruction::store: {
                auto global_addr = dynamic_cast<GlobalVariable*>(inst->getOperand(1));
                auto store_inst = dynamic_cast<StoreInst*>(inst);
                auto const_int_src = dynamic_cast<ConstantInt*>(store_inst->getOperand(0));
                auto const_float_src = dynamic_cast<ConstantFP*>(store_inst->getOperand(0));
                if(global_addr) {
                    if(global_addr->getType()->getPointerElementType()->isFloatType()) {
                        if(const_float_src) {
                            cur_bb_->create_fsw(new Const(const_float_src->getValue()), global_variable_labels_table[global_addr]);
                        } else {
                            cur_bb_->create_fsw(get_asm_reg(inst->getOperand(0)), global_variable_labels_table[global_addr]);
                        }
                    } else {
                        if(const_int_src) {
                            cur_bb_->create_sw(new Const(const_int_src->getValue()), global_variable_labels_table[global_addr]);
                        } else {
                            cur_bb_->create_sw(get_asm_reg(inst->getOperand(0)), global_variable_labels_table[global_addr]);
                        }
                    }                  
                } else {
                    //LOG(ERROR) << "汇编代码生成出现异常";
                }
            }
            break;

        case Instruction::memset: {
                auto memset_inst = dynamic_cast<MemsetInst*>(inst);
                auto alloc_addr = dynamic_cast<AllocaInst*>(inst->getOperand(0));
                auto element_type = alloc_addr->getAllocaType()->getArrayElementType();
                int type_size = element_type->getSize();
                auto addr = val2stack[alloc_addr];
                if(element_type->isFloatType()) {
                    cur_bb_->create_memset(addr, alloc_addr->getAllocaType()->getSize(), type_size, true);
                } else if(element_type->isIntegerType()) {
                    cur_bb_->create_memset(addr, alloc_addr->getAllocaType()->getSize(), type_size, false);
                } else {
                   // LOG(ERROR) << "未预期情况发生";
                }
            }
            break;
        case Instruction::storeoffset: {
                auto storeoffset_inst = dynamic_cast<StoreOffsetInst*>(inst);
                auto offset = storeoffset_inst->getOffset();
                auto const_offset = dynamic_cast<ConstantInt*>(offset);
                auto const_int_src = dynamic_cast<ConstantInt*>(storeoffset_inst->getOperand(0));
                auto const_float_src = dynamic_cast<ConstantFP*>(storeoffset_inst->getOperand(0));
                if(storeoffset_inst->getStoreType()->isFloatType()) {
                    if(const_float_src) {
                        if(const_offset) {
                            cur_bb_->create_fsw(new Const(const_float_src->getValue()), get_asm_reg(inst->getOperand(1)), new Const(const_offset->getValue()));
                        } else {
                            cur_bb_->create_fsw(new Const(const_float_src->getValue()), get_asm_reg(inst->getOperand(1)), get_asm_reg(offset));
                        }
                    } else {
                        if(const_offset) {
                            cur_bb_->create_fsw(get_asm_reg(inst->getOperand(0)), get_asm_reg(inst->getOperand(1)), new Const(const_offset->getValue()));    
                        } else {
                            cur_bb_->create_fsw(get_asm_reg(inst->getOperand(0)), get_asm_reg(inst->getOperand(1)), get_asm_reg(offset));
                        }
                    }
                } else {
                    if(const_int_src) {
                        if(const_offset) {
                            cur_bb_->create_sw(new Const(const_int_src->getValue()), get_asm_reg(inst->getOperand(1)), new Const(const_offset->getValue()));
                        } else {
                            //! 在addi中使用了s1寄存器
                            cur_bb_->create_sw(new Const(const_int_src->getValue()), get_asm_reg(inst->getOperand(1)), get_asm_reg(offset));
                        }
                    } else {
                        if(const_offset) {
                            cur_bb_->create_sw(get_asm_reg(inst->getOperand(0)), get_asm_reg(inst->getOperand(1)), new Const(const_offset->getValue()));
                        } else {
                            cur_bb_->create_sw(get_asm_reg(inst->getOperand(0)), get_asm_reg(inst->getOperand(1)), get_asm_reg(offset));
                        }
                    }
                }
            }
            break;
        case Instruction::br:
         //   LOG(ERROR) << "Unreachable";
            break; 

        case Instruction::cmpbr: 
          //  LOG(ERROR) << "Unreachable";
            break;

        case Instruction::fcmpbr: 
          //  LOG(ERROR) << "Unreachable";
            break;

        case Instruction::call: {
                cur_bb_->create_call(new Label(inst->getOperand(0)->getName()));
                auto func = dynamic_cast<Function*>(inst->getOperand(0));
                if(!func->getReturnType()->isVoidType()) {
                    if(func->getReturnType()->isFloatType()) {
                        if(fval2interval.find(inst) != fval2interval.end()) {
                            if(fval2interval[inst]->reg_id >= 0) {
                                cur_bb_->create_caller_save_call_result(new RegLoc(get_asm_reg(inst)->get_id(), get_asm_reg(inst)->is_float()), new Reg(reg_fa0, true));
                            } else {
                                cur_bb_->create_caller_save_call_result(val2stack[inst], new Reg(reg_fa0, true));
                            }
                        } 
                    } else {
                        if(ival2interval.find(inst) != ival2interval.end()) {
                            if(ival2interval[inst]->reg_id >= 0) {
                                cur_bb_->create_caller_save_call_result(new RegLoc(get_asm_reg(inst)->get_id(), get_asm_reg(inst)->is_float()), new Reg(reg_a0, false));
                            } else {
                                cur_bb_->create_caller_save_call_result(val2stack[inst], new Reg(reg_a0, false));
                            }
                        }
                    }
                } 
            }
            break;
        case Instruction::ret:
            if(!inst->getOperands().empty()) {
                auto ret_val = inst->getOperand(0);
                auto const_int_ret_val = dynamic_cast<ConstantInt*>(ret_val);
                auto const_float_ret_val = dynamic_cast<ConstantFP*>(ret_val);
                
                if(ret_val->getType()->isFloatType()) {
                    if(const_float_ret_val) {
                        cur_bb_->create_callee_save_call_result(new RegLoc(reg_fa0, true), new Const(const_float_ret_val->getValue()));
                    } else {
                        if(fval2interval[ret_val]->reg_id < 0) {
                            cur_bb_->create_callee_save_call_result(new RegLoc(reg_fa0, true), new Mem(val2stack[ret_val]->get_reg_id(), val2stack[ret_val]->get_offset()));
                        } else {
                            cur_bb_->create_callee_save_call_result(new RegLoc(reg_fa0, true), get_asm_reg(ret_val));
                        }     
                    }
                } else {
                    if(const_int_ret_val) {
                        cur_bb_->create_callee_save_call_result(new RegLoc(reg_a0, false), new Const(const_int_ret_val->getValue()));
                    } else {
                        if(ival2interval[ret_val]->reg_id < 0) {
                            cur_bb_->create_callee_save_call_result(new RegLoc(reg_a0, false), new Mem(val2stack[ret_val]->get_reg_id(), val2stack[ret_val]->get_offset()));
                        } else {
                            cur_bb_->create_callee_save_call_result(new RegLoc(reg_a0, false), get_asm_reg(ret_val));
                        }      
                    }
                }
            }
            break;
        //& phi已经被处理
        case Instruction::phi:
            break;

        //& alloca指令无需处理
        case Instruction::alloca:
            break;

        //& gep指令已经被split
        case Instruction::getelementptr: {
                auto base_addr = inst->getOperand(0);
                if(dynamic_cast<GlobalVariable*>(base_addr)) {
                    auto addr = global_variable_labels_table[dynamic_cast<GlobalVariable*>(base_addr)];
                    cur_bb_->create_la(get_asm_reg(inst), addr);
                } else if(dynamic_cast<AllocaInst*>(base_addr)) {
                    auto addr = val2stack[base_addr];
                    int offset = addr->get_offset();
                    auto reg_id = addr->get_reg_id();
                    cur_bb_->create_add(get_asm_reg(inst), new Reg(reg_id, false), new Const(offset));
                } else if(dynamic_cast<Argument*>(base_addr)) {
                    cur_bb_->create_mv(get_asm_reg(inst), get_asm_reg(base_addr));
                } else {
                    cur_bb_->create_mv(get_asm_reg(inst), get_asm_reg(base_addr));
                }
            }
            break;
        default:
          //  LOG(ERROR) << "未实现";
            break;
    }
}