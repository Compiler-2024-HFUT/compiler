#include "backend/AsmGen.hpp"


void AsmGen::visit(Module &node){ 
    //加载全局变量到全局变量表
    for(auto global_var: asm_unit->getModuleOfAsmUnit()->getGlobalVariables()){
        global_variable_labels_table[global_var] = new Label(global_var->getName());
    }
    //开始对汇编单元中的函数进行操作
    for(auto func:asm_unit->getModuleOfAsmUnit()->getFunctions()){
        if(!func->isDeclaration()){
            ival2interval = ivalue_interval_map[func];
            fval2interval = fvalue_interval_map[func];
            asm_unit->addSubroutine(func);
            subroutine = asm_unit->getSubroutine();
            func->accept(*this);
        }
    }
}

void AsmGen::visit(Function &node){
    auto func = &node;
    save_offset = 0;
    total_size = 0;
    iargs_size = 0;
    fargs_size = 0;

    val2stack.clear();
    //确定本函数用到的caller与callee寄存器，用于保存现场
    total_size =  setCallerAndCalleeRegs();


    //******************************选择某call指令前使用到的寄存器，以便与后面的保存现场和恢复现场配合，减少访存指令******************************
    ::std::vector<int> reg_iid_s={};
    ::std::vector<int> reg_fid_s={};

    //call表，用于存储上一个call，以便保存上一个call的返回寄存器
    ::std::vector<Instruction*> call_table ={};

    //记录用于传递参数的寄存器
    int iarg_num = func->getIArgs().size();
    auto iarg_vector = func->getIArgs();
    int farg_num = func->getFArgs().size();
    auto farg_vector = func->getFArgs();
    for(int p = 0; p<iarg_num; p++){
     auto check = ival2interval.find(iarg_vector[p]);
     if(check!=ival2interval.end())
         reg_iid_s.push_back(ival2interval[iarg_vector[p]]->reg);
    }
 
    for(int p = 0; p<farg_num; p++){
     auto check = fval2interval.find(farg_vector[p]);
     if(check!=fval2interval.end())
         reg_fid_s.push_back(fval2interval[farg_vector[p]]->reg);
    }
 
    //记录上条call指令传递结果的寄存器、gep指令的mov的目的寄存器以及该call指令前面指令的目的寄存器
    for(auto bb: func->getBasicBlocks()){
        for(auto inst: bb->getInstructions()){
            if(inst->isCall()){
             if(call_table.size()>0){
                 auto pre_call_inst = call_table.back();
                 auto callee_func = dynamic_cast<Function*>(pre_call_inst->getOperand(0));
                 if(!callee_func->getReturnType()->isVoidType()){
                     if(callee_func->getReturnType()->isFloatType()){
                         if(fval2interval.find(pre_call_inst)!=fval2interval.end() && fval2interval[pre_call_inst]){
                             int freg = fval2interval[pre_call_inst]->reg;
                             reg_fid_s.push_back(freg);
                         }
                     }
                     else{
                         if(ival2interval.find(pre_call_inst)!=ival2interval.end() && ival2interval[pre_call_inst]){
                              int ireg = ival2interval[pre_call_inst]->reg;
                             reg_iid_s.push_back(ireg);
                         }
                     }
                 }
             } 
                 //caller传参
                 ::std::vector<Value*> iargs;
                 ::std::vector<Value*> fargs;
                 ::std::vector<int> para_reg_iids ={};
                 ::std::vector<int> para_reg_fids ={};   
                 for(auto arg: inst->getOperands()) {
                     if(dynamic_cast<Function*>(arg))
                         continue;
                     if(!arg->getType()->isFloatType()) 
                         iargs.push_back(arg);
                     if(arg->getType()->isFloatType()) 
                         fargs.push_back(arg);
                 }
                 int caller_iarg_num = iargs.size();
                 int caller_farg_num = fargs.size();
 
                 for(int i=0; i<caller_iarg_num; i++){
                     auto check = ival2interval.find(iargs[i]);
                     if(check!=ival2interval.end()){
                         para_reg_iids.push_back(ival2interval[iargs[i]]->reg);
                     }
                 }
 
                 for(int i=0; i<caller_farg_num; i++){
                     auto check = fval2interval.find(fargs[i]);
                     if(check!=fval2interval.end()){
                         para_reg_fids.push_back(fval2interval[fargs[i]]->reg);
                     }
                 }
 
 
 
                call_define_ireg_map[inst] = reg_iid_s;
                call_define_freg_map[inst] = reg_fid_s;
 
                call_define_ireg_map[inst].insert(call_define_ireg_map[inst].end(), para_reg_iids.begin(), para_reg_iids.end());
                call_define_freg_map[inst].insert(call_define_freg_map[inst].end(), para_reg_fids.begin(), para_reg_fids.end());
                call_table.push_back(inst);
            }
         else if(inst->isGep()){
             auto base = inst->getOperand(0);
             auto global_base = dynamic_cast<GlobalVariable*>(base);
             auto alloca_base = dynamic_cast<AllocaInst*>(base);
             auto arg_base = dynamic_cast<Argument*>(base);
              if(!global_base && !alloca_base) {
                if(base->getType()->isFloatType()){
                    auto f_interval = fval2interval.find(base);
                    if(f_interval!=fval2interval.end()){
                        int freg_id_inst = static_cast<int>(f_interval->second->reg);
                        if(freg_id_inst >-1 && freg_id_inst <32)
                            reg_fid_s.push_back(freg_id_inst);
                    }
                }
                else{
                    auto i_interval = ival2interval.find(base);
                    if(i_interval!=ival2interval.end()){
                        int ireg_id_inst = static_cast<int>(i_interval->second->reg);
                        if(ireg_id_inst >-1 && ireg_id_inst <32)
                            reg_iid_s.push_back(ireg_id_inst);
                    }
                }
              } 
              //reg_iid_s.push_back(dynamic_cast<GReg*>( getAllocaReg(inst))->getID());
                if(inst->getType()->isFloatType()){
                    auto f_interval = fval2interval.find(inst);
                    if(f_interval!=fval2interval.end()){
                        int freg_id_inst = static_cast<int>(f_interval->second->reg);
                        if(freg_id_inst >-1 && freg_id_inst <32)
                            reg_fid_s.push_back(freg_id_inst);
                    }
                }
                else{
                    auto i_interval = ival2interval.find(inst);
                    if(i_interval!=ival2interval.end()){
                        int ireg_id_inst = static_cast<int>(i_interval->second->reg);
                        if(ireg_id_inst >-1 && ireg_id_inst <32)
                            reg_iid_s.push_back(ireg_id_inst);
                    }
                }
         }
            else{
                
              //  auto reg = getAllocaReg(inst);
              //  if(dynamic_cast<FReg*>(reg)){
              //      reg_fid_s.push_back(dynamic_cast<FReg*>(reg)->getID());
              //  }
              //  else if(dynamic_cast<GReg*>(reg)){
              //      reg_iid_s.push_back(dynamic_cast<GReg*>(reg)->getID());
              //  }
                if(inst->getType()->isFloatType()){
                    auto f_interval = fval2interval.find(inst);
                    if(f_interval!=fval2interval.end()){
                        int freg_id_inst = static_cast<int>(f_interval->second->reg);
                        if(freg_id_inst >-1 && freg_id_inst <32)
                            reg_fid_s.push_back(freg_id_inst);
                    }
                }
                else{
                    auto i_interval = ival2interval.find(inst);
                    if(i_interval!=ival2interval.end()){
                        int ireg_id_inst = static_cast<int>(i_interval->second->reg);
                        if(ireg_id_inst >-1 && ireg_id_inst <32)
                            reg_iid_s.push_back(ireg_id_inst);
                    }
                }

            }
        }
    }
    //******************************选择某call指令前使用到的寄存器，以便与后面的保存现场和恢复现场配合，减少访存指令******************************

    //为函数的参数分配内存空间
    if(func->getIArgs().size() > 8) 
        iargs_size+=allocateMemForIArgs();

    if(func->getFArgs().size() > 8) 
        fargs_size+=allocateMemForFArgs();


    //& record stack info and used tmp regs for inst gen
    cur_tmp_reg_saved_stack_offset = 0;
    caller_trans_args_stack_offset = 0;
    caller_saved_regs_stack_offset = 0;

    cur_tmp_iregs.clear();          //~ 当前借用的临时寄存器
    cur_tmp_fregs.clear();          //~ 当前借用的临时寄存器
    tmp_iregs_loc.clear();          //~ 保存临时寄存器原本值的地址
    tmp_fregs_loc.clear();          //~ 保存临时寄存器原本值的地址
    free_locs_for_tmp_regs_saved.clear();

    //*************************线性化BB并标号***************************
    //线性化：bb的顺序遵从：入口bb、过程bb、返回bb
 //      BasicBlock *ret_bb;
 //  Label* new_label;
 //  ::std::string label_str;

 //  bb2label.clear();
 //  linear_bbs.clear();
 //  bool fff = false;
 //  std::list<BasicBlock*> linear_bbs_of_func = func->getBasicBlocks();
 //  int mp = 0;
 //  for(auto bb: linear_bbs_of_func) {
 //      if(bb == func->getEntryBlock() && bb->getTerminator()->isRet()) {
 //          label_str = "";
 //          bb2label.insert({bb, new Label(label_str)});
 //          linear_bbs.push_back(bb);
 //          fff = true;
 //          break;
 //      } else if(bb == func->getEntryBlock()) {
 //          label_str = func->getName() + "_" + "entry";
 //          bb2label.insert({bb, new Label(label_str)});
 //      } else if(bb != func->getEntryBlock() && !bb->getTerminator()->isRet()) {
 //          label_str = func->getName() + "_" + ::std::to_string(mp++);
 //          new_label = new Label(label_str);
 //          bb2label.insert({bb, new_label});
 //      } else {
 //          ret_bb = bb;
 //          continue;
 //      }
 //      linear_bbs.push_back(bb);
 //  }
 //  if(!fff){
 //  label_str = func->getName() + "_" + "ret";
 //  new_label = new Label(label_str);
 //  bb2label.insert({ret_bb, new_label});
 //  linear_bbs.push_back(ret_bb);
 //  }

   bb2label.clear();
   linear_bbs.clear();
    ret_bb = nullptr;
    bool finish = false;
    int index = 0;
    ::std::string bb_label_name;
    Label* bb_label;
    for(auto bb: func->getBasicBlocks()){
        if(bb==func->getEntryBlock()){
            //空bb
            if(bb->getTerminator()->isRet()){
                bb_label_name = "";
                bb_label = new Label(bb_label_name);
                bb2label.insert({bb, bb_label});
                linear_bbs.push_back(bb);
                finish = true;
                break;
            }
            else{
                bb_label_name = func->getName() + "_" + "entry";
            }
        }
        else{
            if(!bb->getTerminator()->isRet()){
                bb_label_name = func->getName() + "_" + ::std::to_string(index++);
            }
            else{
                ret_bb = bb;
                continue;
            }
        }
        bb_label = new Label(bb_label_name);
        bb2label.insert({bb, bb_label});
        linear_bbs.push_back(bb);
    }

    if(!finish){
        bb_label_name = func->getName() + "_" + "ret";
        bb_label = new Label(bb_label_name);
        bb2label.insert({ret_bb, bb_label});
        linear_bbs.push_back(ret_bb);
    }

    //*************************线性化BB并标号***************************


    //*************************栈空间的开辟***************************


    //为函数的参数列表中超过8个的参数分配内存空间
//  if(func->getIArgs().size() > 8) {
//     iargs_size+=allocateMemForIArgs();
//  //          int i = 0;
//  //   for(auto arg: func->getIArgs()) {
//  //       if(i >= 8) {
//  //           int size_of_arg_type = arg->getType()->getSize();
//  //           val2stack[static_cast<Value*>(arg)] = new IRIA(static_cast<int>(RISCV::GPR::s0), iargs_size);
//  //           iargs_size += align_8(size_of_arg_type);
//  //       }
//  //       i++;
//  //   }
//
//
//
//// for(int i=8; i<subroutine->getFuncOfSubroutine()->getIArgs().size(); i++){
////     auto ipara = subroutine->getFuncOfSubroutine()->getIArgs()[i];
////     val2stack[static_cast<Value*>(ipara)] = new IRIA(static_cast<int>(RISCV::GPR::s0), iargs_size);
////     iargs_size += align_8(ipara->getType()->getSize());
//// }
//
//  }

 //  if(func->getFArgs().size() > 8) {
 //      fargs_size+=allocateMemForFArgs();
 // //             int i = 0;
 // //     for(auto arg: func->getFArgs()) {
 // //         if(i >= 8) {
 // //             int size_of_arg_type = arg->getType()->getSize();
 // //             val2stack[static_cast<Value*>(arg)] = new IRIA(static_cast<int>(RISCV::GPR::s0), iargs_size + fargs_size);
 // //             fargs_size +=  align_8(size_of_arg_type);
 // //         } 
 // //         i++;
 // //     }

 ////     for(int i=8; i<subroutine->getFuncOfSubroutine()->getFArgs().size(); i++){
 ////      auto fpara = subroutine->getFuncOfSubroutine()->getFArgs()[i];
 ////      val2stack[static_cast<Value*>(fpara)] = new IRIA(static_cast<int>(RISCV::GPR::s0), iargs_size+fargs_size);
 ////      fargs_size += align_8(fpara->getType()->getSize());
 ////  }
 //  }


    //为栈上指针分配内存空间
    total_size += allocateMemForIPointer();
    total_size += allocateMemForFPointer();




    //为alloca指令分配内存空间
    total_size += allocateMemForAlloca();


    //对齐地址
    int stack_size = align_8(total_size);
    //*************************栈空间的开辟***************************

    subroutine->addSequence(subroutine->getFuncOfSubroutine()->getEntryBlock(), bb2label[subroutine->getFuncOfSubroutine()->getEntryBlock()]);
    sequence = subroutine->getSequence();

 
    //*************************被调用函数的栈帧初始化***************************
 //   int cur_offset = 0;
//
 //   std::vector<std::pair<IRA*, IRIA*>> to_save_iregs;
 //   std::vector<std::pair<FRA*, IRIA*>> to_save_fregs;
//
 //   if(!used_iregs_pair.second.empty()) {
 //       for(auto iter = used_iregs_pair.second.begin(); iter != used_iregs_pair.second.end(); iter++) {
 //           cur_offset -= reg_size;
 //           to_save_iregs.push_back(std::make_pair(new IRA(*iter), new IRIA(static_cast<int>(RISCV::GPR::sp), cur_offset)));
 //       }
 //   }
 //   if(!used_fregs_pair.second.empty()) {
 //       for(auto iter = used_fregs_pair.second.begin(); iter != used_fregs_pair.second.end(); iter++) {
 //           cur_offset -= reg_size;
 //           to_save_fregs.push_back(std::make_pair(new FRA(*iter), new IRIA(static_cast<int>(RISCV::GPR::sp), cur_offset)));
 //       }
 //   }
    auto callee_save_iregs = getCalleeSaveIRegs();
    auto callee_save_fregs = getCalleeSaveFRegs();

    if(!callee_save_iregs.empty())
        sequence->createCalleeSaveRegs(callee_save_iregs);
    if(!callee_save_fregs.empty())
        sequence->createCalleeSaveRegs(callee_save_fregs);
    sequence->createCalleeStackFrameInitialize(stack_size);
    //*************************被调用函数的栈帧初始化***************************


    std::vector<std::pair<AddressMode*, AddressMode*>> to_move_iargs = callee_iargs_move(subroutine->getFuncOfSubroutine());
    std::vector<std::pair<AddressMode*, AddressMode*>> to_move_fargs = callee_fargs_move(subroutine->getFuncOfSubroutine());

    if(!to_move_iargs.empty() || !to_move_fargs.empty())
        sequence->createCalleeParaPass(to_move_iargs, to_move_fargs);

    for(auto bb: linear_bbs) {
        if(bb != subroutine->getFuncOfSubroutine()->getEntryBlock()) 
            subroutine->addSequence(bb, bb2label[bb]);
            sequence = subroutine->getSequence();
            bb->accept(*this);
    }

   //*************************被调用函数的栈帧收尾***************************
    
    std::vector<std::pair<IRA*, IRIA*>> to_load_iregs;
    std::vector<std::pair<FRA*, IRIA*>> to_load_fregs;
    
    sequence->createCalleeStackFrameClear(stack_size);
    
    int num_of_all_restore_regs = used_iregs_pair.second.size() + used_fregs_pair.second.size();
    save_offset = - reg_size * num_of_all_restore_regs;
    if(!used_fregs_pair.second.empty()) {
        for(auto iter = used_fregs_pair.second.rbegin(); iter != used_fregs_pair.second.rend(); iter++) {
            to_load_fregs.push_back(std::make_pair(new FRA(*iter), new IRIA(static_cast<int>(RISCV::GPR::sp), save_offset)));
            save_offset += reg_size; 
        }
    }
    
    if(!used_iregs_pair.second.empty()) {
        for(auto iter = used_iregs_pair.second.rbegin(); iter != used_iregs_pair.second.rend(); iter++) {
            to_load_iregs.push_back(std::make_pair(new IRA(*iter), new IRIA(static_cast<int>(RISCV::GPR::sp), save_offset)));
            save_offset += reg_size; 
        }
    }
    sequence->createCalleeRestoreRegs(to_load_iregs);
    sequence->createCalleeRestoreRegs(to_load_fregs);
    //*************************被调用函数的栈帧收尾***************************

    sequence->createRet();
}

void AsmGen::visit(BasicBlock &node){
    Instruction *br_inst = nullptr;
    for(auto &inst: sequence->getBBOfSeq()->getInstructions()) {
        if(inst->isTerminator()) {
            br_inst = inst;
            break;
        }

 
        //********************************************加载指令需要的临时寄存器***********************
        ld_tmp_regs_for_inst(inst);
        //********************************************加载指令需要的临时寄存器***********************

       if(inst->isCall()) {
            auto call_inst = dynamic_cast<CallInst*>(inst);
            
            //************************************调用前，保存caller寄存器************************
            caller_saved_ireg_locs.clear();
            caller_saved_freg_locs.clear();
            caller_save_iregs.clear();
            caller_save_fregs.clear();

            std::vector<std::pair<IRA*, IRIA*>> to_store_iregs;
            std::vector<std::pair<FRA*, IRIA*>> to_store_fregs;

            int call_inst_reg = -1;
            bool is_float_call = call_inst->getType()->isFloatType();
            bool is_void_call = call_inst->getType()->isVoidType();
            bool is_int_call = !is_float_call && !is_void_call;

            if(!is_void_call) {
                if(is_float_call && fval2interval.find(call_inst) != fval2interval.end()) {
                    call_inst_reg = fval2interval[call_inst]->reg;
                } else if(ival2interval.find(call_inst) != ival2interval.end()) {
                    call_inst_reg = ival2interval[call_inst]->reg;
                }
            }

            for(auto ireg: used_iregs_pair.first) {
                if(is_int_call && ireg == call_inst_reg)
                    continue;
               if(::std::find(call_define_ireg_map[inst].begin(), call_define_ireg_map[inst].end(), ireg) != call_define_ireg_map[inst].end()){
                 //   ::std::cout<<"ireg："<<ireg<<::std::endl;
                   caller_save_iregs.push_back(ireg);
                }
                
            }

            for(auto freg: used_fregs_pair.first) {
                if(is_float_call && freg == call_inst_reg)
                    continue;
               if(::std::find(call_define_freg_map[inst].begin(), call_define_freg_map[inst].end(), freg) != call_define_freg_map[inst].end()){
                 //   ::std::cout<<"freg："<<freg<<::std::endl;
                    caller_save_fregs.push_back(freg);
                }

            }
            for(auto reg: caller_save_iregs) {
                caller_saved_regs_stack_offset -= 8;
                auto regbase = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset);
                caller_saved_ireg_locs[reg] = regbase;
                to_store_iregs.push_back(std::make_pair(new IRA(reg), regbase));
            }
            for(auto reg: caller_save_fregs) {
                caller_saved_regs_stack_offset -= 8;
                auto regbase = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset);
                caller_saved_freg_locs[reg] = regbase;
                to_store_fregs.push_back(std::make_pair(new FRA(reg), regbase));
            }

            if(! to_store_iregs.empty())
                sequence->createCallerSaveRegs(to_store_iregs);

            if(! to_store_fregs.empty())
                sequence->createCallerSaveRegs(to_store_fregs);



            //************************************调用前，保存caller寄存器************************

            std::vector<std::pair<AddressMode*, AddressMode*>> to_move_fargs = caller_fargs_move(call_inst);
            std::vector<std::pair<AddressMode*, AddressMode*>> to_move_iargs = caller_iargs_move(call_inst);

            if(!to_move_fargs.empty() || !to_move_iargs.empty())
                sequence->createCallerParaPass( to_move_iargs, to_move_fargs);

            int extra_stack_offset = caller_trans_args_stack_offset + cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset; 
            
            if(extra_stack_offset != 0) 
                sequence->createCalleeStackFrameExpand(extra_stack_offset);
            
            call_inst->accept(*this);
            
            if(extra_stack_offset != 0) 
                sequence->createCalleeStackFrameShrink(-extra_stack_offset);
            
       
            //************************************调用后，恢复caller寄存器******************************
            std::vector<std::pair<IRA*, IRIA*>> to_restore_iregs;
            std::vector<std::pair<FRA*, IRIA*>> to_restore_fregs;

            for(auto &[freg, loc]: caller_saved_freg_locs) {
                to_restore_fregs.push_back(std::make_pair(new FRA(freg), loc));    
            }
            for(auto &[ireg, loc]: caller_saved_ireg_locs) {
                to_restore_iregs.push_back(std::make_pair(new IRA(ireg), loc)); 
            }
            caller_saved_regs_stack_offset = 0;

            if(! to_restore_fregs.empty())
                sequence->createCallerRestoreRegs(to_restore_fregs);


            if(! to_restore_iregs.empty())
                sequence->createCallerRestoreRegs(to_restore_iregs);
         

            //************************************调用后，恢复caller寄存器******************************

            
            caller_trans_args_stack_offset = 0;
        } else if(!inst->isPhi()) {
            
            //*************************************为指令分配临时寄存器************************
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

            std::vector<std::pair<IRA*, IRIA*>> to_store_iregs;
            std::vector<std::pair<IRA*, IRIA*>> to_ld_iregs;

            std::vector<std::pair<FRA*, IRIA*>> to_store_fregs;
            std::vector<std::pair<FRA*, IRIA*>> to_ld_fregs;

            for(auto opr: inst->getOperands()) {
                if(dynamic_cast<Constant*>(opr) ||
                dynamic_cast<BasicBlock*>(opr) ||
                dynamic_cast<GlobalVariable*>(opr) ||
                dynamic_cast<AllocaInst*>(opr)) {
                    continue;
                }
                if(opr->getType()->isFloatType()) {
                    if(fval2interval[opr]->reg >= 0) {
                        inst_freg_id_set.insert(fval2interval[opr]->reg);
                    } 
                } else {
                    if(ival2interval[opr]->reg >= 0) {
                        inst_ireg_id_set.insert(ival2interval[opr]->reg);
                    } 
                }
            }

            //& finding a register to store the result for an instruction
            if(!inst->isVoid() && !dynamic_cast<AllocaInst*>(inst)) {
                if(inst->getType()->isFloatType()) {
                    auto reg_interval = fval2interval[inst];
                    if(reg_interval->reg < 0) {
                        if(!cur_tmp_fregs.empty()) {
                            reg_interval->reg = *cur_tmp_fregs.begin();
                            cur_tmp_fregs.erase(reg_interval->reg);
                            used_tmp_fregs.insert(reg_interval->reg);
                        } else {
                            for(auto freg: all_alloca_fprs) {
                                if(inst_freg_id_set.find(freg) == inst_freg_id_set.end()) {
                                    reg_interval->reg = freg;
                                    fstore_list.insert(freg);
                                    break;
                                }
                            }
                        }
                        use_tmp_regs_interval.insert(reg_interval);
                        to_store_fvals.insert(inst);
                    } 
                    inst_freg_id_set.insert(reg_interval->reg);
                    if(reg_interval->reg < 0) {}
                     //   LOG(ERROR) << "在为指令生成代码时分配临时寄存器出现异常";
                } else {
                    auto reg_interval = ival2interval[inst];
                    if(reg_interval->reg < 0) {
                        if(!cur_tmp_iregs.empty()) {
                            reg_interval->reg = *cur_tmp_iregs.begin();
                            cur_tmp_iregs.erase(reg_interval->reg);
                            used_tmp_iregs.insert(reg_interval->reg);
                        } else {
                            for(auto ireg: all_alloca_gprs) {
                                if(inst_ireg_id_set.find(ireg) == inst_ireg_id_set.end()) {
                                    reg_interval->reg = ireg;
                                    istore_list.insert(ireg);
                                    break;
                                }
                            }
                        }
                        use_tmp_regs_interval.insert(reg_interval);
                        to_store_ivals.insert(inst);
                    } 
                    inst_ireg_id_set.insert(reg_interval->reg);
                    if(reg_interval->reg < 0) {}
                     //   LOG(ERROR) << "在为指令生成代码时分配临时寄存器出现异常";
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
                    if(reg_interval->reg < 0) {
                        if(!cur_tmp_fregs.empty()) {
                            reg_interval->reg = *cur_tmp_fregs.begin();
                            cur_tmp_fregs.erase(reg_interval->reg);
                            used_tmp_fregs.insert(reg_interval->reg);
                            inst_freg_id_set.insert(reg_interval->reg);
                        } else {
                            for(auto freg: all_alloca_fprs) {
                                if(inst_freg_id_set.find(freg) == inst_freg_id_set.end()) {
                                    reg_interval->reg = freg;
                                    fstore_list.insert(freg);
                                    inst_freg_id_set.insert(freg);
                                    break;
                                }
                            }
                        }
                        to_ld_fval_set.insert(opr);
                        use_tmp_regs_interval.insert(reg_interval);
                    } 
                    if(reg_interval->reg < 0) {}
                       // LOG(ERROR) << "在为指令生成代码时分配临时寄存器出现异常";
                } else {
                    auto reg_interval = ival2interval[opr];
                    if(reg_interval->reg < 0) {
                        if(!cur_tmp_iregs.empty()) {
                            reg_interval->reg = *cur_tmp_iregs.begin();
                            cur_tmp_iregs.erase(reg_interval->reg);
                            used_tmp_iregs.insert(reg_interval->reg);
                            inst_ireg_id_set.insert(reg_interval->reg);
                        } else {
                            for(auto ireg: all_alloca_gprs) {
                                if(inst_ireg_id_set.find(ireg) == inst_ireg_id_set.end()) {
                                    reg_interval->reg = ireg;
                                    istore_list.insert(ireg);
                                    inst_ireg_id_set.insert(ireg);
                                    break;
                                }
                            }
                        }
                        to_ld_ival_set.insert(opr);
                        use_tmp_regs_interval.insert(reg_interval);
                    } 
                    if(reg_interval->reg < 0) {}
                      //  LOG(ERROR) << "在为指令生成代码时分配临时寄存器出现异常";
                }
            }

            //& store the origin values in tmp used regs
            for(auto reg_id: istore_list) {
                if(free_locs_for_tmp_regs_saved.empty()) {
                    cur_tmp_reg_saved_stack_offset -= 8;
                    IRIA* loc = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset);
                    tmp_iregs_loc[reg_id] = loc;
                    to_store_iregs.push_back(std::make_pair(new IRA(reg_id), loc));
                } else {
                    IRIA* loc = *free_locs_for_tmp_regs_saved.begin();
                    free_locs_for_tmp_regs_saved.erase(free_locs_for_tmp_regs_saved.begin());
                    tmp_iregs_loc[reg_id] = loc;
                    to_store_iregs.push_back(std::make_pair(new IRA(reg_id), loc));
                }
                cur_tmp_iregs.insert(reg_id);
            }

            for(auto reg_id: fstore_list) {
                if(free_locs_for_tmp_regs_saved.empty()) {
                    cur_tmp_reg_saved_stack_offset -= 8;
                    IRIA* loc = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset);
                    tmp_fregs_loc[reg_id] = loc;
                    to_store_fregs.push_back(std::make_pair(new FRA(reg_id), loc));
                } else {
                    IRIA* loc = *free_locs_for_tmp_regs_saved.begin();
                    free_locs_for_tmp_regs_saved.erase(free_locs_for_tmp_regs_saved.begin());
                    tmp_fregs_loc[reg_id] = loc;
                    to_store_fregs.push_back(std::make_pair(new FRA(reg_id), loc));
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
                IRIA *reg_base = val2stack[fval];
                to_ld_fregs.push_back(std::make_pair(new FRA(fval2interval[fval]->reg), reg_base));
            }

            for(auto ival: to_ld_ival_set) {
                IRIA *reg_base = val2stack[ival];
                to_ld_iregs.push_back(std::make_pair(new IRA(ival2interval[ival]->reg), reg_base));
            }

            if(! to_ld_iregs.empty() || !to_store_iregs.empty())
                sequence->createAllocaTmpRegs(to_ld_iregs, to_store_iregs );

            if(! to_ld_fregs.empty() || !to_store_fregs.empty())
                sequence->createAllocaTmpRegs(to_ld_fregs, to_store_fregs );
            //*************************************为指令分配临时寄存器************************

            inst->accept(*this);
            
            //**********************************存储临时寄存器的值到栈中***********************
   
             std::vector<std::pair<IRA*, IRIA*>> to_store_iregs__;
             std::vector<std::pair<FRA*, IRIA*>> to_store_fregs__;
             
             if(!to_store_ivals.empty()) {
                 for(auto ival: to_store_ivals) {
                     IRIA *regbase = val2stack[ival];
                     to_store_iregs__.push_back(std::make_pair(new IRA(ival2interval[ival]->reg), regbase));
                 }   
                 to_store_ivals.clear();
             } 
             if(!to_store_fvals.empty()) {
                 for(auto fval: to_store_fvals) {
                     IRIA *regbase = val2stack[fval];
                     to_store_fregs__.push_back(std::make_pair(new FRA(fval2interval[fval]->reg), regbase));
                 }
                 to_store_fvals.clear();
             }
             
             for(auto inter: use_tmp_regs_interval) {
                 inter->reg = -1;
             }
             
             to_store_ivals.clear();
             to_store_fvals.clear();
             use_tmp_regs_interval.clear();
             
             if(! to_store_iregs__.empty())
                 sequence->createStoreTmpRegs(to_store_iregs__);
             
             if(! to_store_fregs__.empty())
             sequence->createStoreTmpRegs(to_store_fregs__);

            //**********************************存储临时寄存器的值到栈中***********************
        } 
}
   
    //********************************装载临时寄存器到指令********************
 ld_tmp_regs_for_inst(br_inst);
    //********************************装载临时寄存器到指令********************


    if(br_inst->isRet()) {
        br_inst->accept(*this);
    } else {
       
        //*****************************处理phi**********************
        phi_union(br_inst);

        //*****************************处理phi**********************
    }
}


void AsmGen::visit(BinaryInst &node){
    auto inst = &node;
    auto pair = b_inst_gen_map.find(node.getInstrType());
    if(pair==b_inst_gen_map.end()) return;
    pair->second(inst);
    
}

void AsmGen::visit(CmpInst &node){
    auto inst = &node;
    auto pair = cmp_inst_gen_map.find(node.getCmpOp());
    if(pair==cmp_inst_gen_map.end()) return;
    pair->second(inst);
}

void AsmGen::visit(FCmpInst &node){
    auto inst = &node;
    auto pair = fcmp_inst_gen_map.find(node.getCmpOp());
    if(pair==fcmp_inst_gen_map.end()) return;
    pair->second(inst);
}

void AsmGen::visit(CallInst &node){
    auto inst = &node;
    auto target_func = dynamic_cast<Function*>(inst->getOperand(0));
    ::std::string target_func_name = target_func->getName();
    sequence->createCall(new Label(target_func_name));
    if(target_func->getReturnType()->isVoidType()) return;
    else{
        if(target_func->getReturnType()->isFloatType()) {
            if(fval2interval.find(inst) != fval2interval.end()) {
                auto frs = new FReg(static_cast<int>(RISCV::FPR::fa0));
                if(fval2interval[inst]->reg >= 0) {
                    auto rd = new FRA(static_cast<int>(dynamic_cast<FReg*>( getAllocaReg(inst))->getID()) );
                    sequence->createCallerSaveResult(frs, rd);
                } else {
                    sequence->createCallerSaveResult(frs, val2stack[inst]);
             }
            } 
        } else {
            if(ival2interval.find(inst) != ival2interval.end()) {
                auto irs = new GReg(static_cast<int>(RISCV::GPR::a0));
                if(ival2interval[inst]->reg >= 0) {
                    auto rd = new IRA(static_cast<int>(dynamic_cast<GReg*>( getAllocaReg(inst))->getID()) );
                    sequence->createCallerSaveResult(irs, rd);
                } else {
                    sequence->createCallerSaveResult(irs, val2stack[inst]);
                }
            }
        }
    } 
}

void AsmGen::visit(BranchInst &node){
    //结束
}

void AsmGen::visit(ReturnInst &node){
    auto inst = &node;
    if(!inst->getOperands().empty()) {
        auto ret_val = inst->getOperand(0);
        if(ret_val->getType()->isFloatType()) {
            auto fconst_ret_val = dynamic_cast<ConstantFP*>(ret_val);
            auto frs = new FRA(static_cast<int>(RISCV::FPR::fa0));
            if(fconst_ret_val) {
                auto dst = new FConst(fconst_ret_val->getValue());
                sequence->createCalleeSaveResult(frs, dst);
            }   
            else {
                if(fval2interval[ret_val]->reg >= 0) {
                    auto dst_reg = getAllocaReg(ret_val);
                    sequence->createCalleeSaveResult(frs, dst_reg);
                } else {
                    auto dst_mem = new Mem(val2stack[ret_val]->getOffset(), static_cast<int>(val2stack[ret_val]->getReg()));
                    sequence->createCalleeSaveResult(frs, dst_mem);
                }     
            }
        } 
        else {
            auto iconst_ret_val = dynamic_cast<ConstantInt*>(ret_val);
            auto irs = new IRA(static_cast<int>(RISCV::GPR::a0));
            if(iconst_ret_val) {
                auto dst = new IConst(iconst_ret_val->getValue());
                sequence->createCalleeSaveResult(irs, dst);
            } else {
                if(ival2interval[ret_val]->reg >= 0) {
                    auto dst_reg = dynamic_cast<GReg*>(getAllocaReg(ret_val)); 
                    sequence->createCalleeSaveResult(irs, dst_reg);
                    
                } else {
                    auto dst_mem = new Mem( val2stack[ret_val]->getOffset(), static_cast<int>(val2stack[ret_val]->getReg()));
                    sequence->createCalleeSaveResult(irs, dst_mem);
                }      
            }
        }
    }
}   

//gep 首地址 0 0
//void AsmGen::visit(GetElementPtrInst &node){
//    auto inst = &node;
//    
//    auto rd = dynamic_cast<GReg*>( getAllocaReg(inst));
//    auto base = inst->getOperand(0);
//
//    auto offset = inst->getOperand(2);
//
//
//
//    auto global_base = dynamic_cast<GlobalVariable*>(base);
//    auto alloca_base = dynamic_cast<AllocaInst*>(base);
//    auto arg_base = dynamic_cast<Argument*>(base);
//    if(global_base) {
//        auto addr = global_variable_labels_table[global_base];
//       
//        sequence->createLa(rd, addr);
//        auto rs1 = dynamic_cast<ConstantInt*>(inst->getOperand(2))?new IConst(dynamic_cast<ConstantInt*>(inst->getOperand(2))->getValue()): getAllocaReg(inst->getOperand(2));
//        sequence->createSh2Add(rd, dynamic_cast<GReg*>(rs1), rd);
//    } 
//    else if(alloca_base) {
//        auto addr = val2stack[base];
//        auto rs2 = new GReg(static_cast<int>(addr->getReg()));
//        auto rs1 = new IConst(addr->getOffset());
//        auto r0 = new GReg(static_cast<int>(RISCV::GPR::zero));
//        sequence->createAdd(rd, r0, rs1);
//        sequence->createSh2Add(rd, rd, rs2);
//    } 
//    else{
//        auto rs1 = dynamic_cast<ConstantInt*>(inst->getOperand(2))?new IConst(dynamic_cast<ConstantInt*>(inst->getOperand(2))->getValue()): getAllocaReg(inst->getOperand(2));
//        auto rs2 = dynamic_cast<GReg*>( getAllocaReg(base));
// 
//        sequence->createSh2Add(rd, dynamic_cast<GReg*>(rs1), rs2);
//    }
//}

//void AsmGen::visit(GetElementPtrInst &node){
//    auto inst = &node;
//    auto base = inst->getOperand(0);
//    auto global_base = dynamic_cast<GlobalVariable*>(base);
//    auto alloca_base = dynamic_cast<AllocaInst*>(base);
//    auto arg_base = dynamic_cast<Argument*>(base);
//    if(global_base) {
//        auto addr = global_variable_labels_table[global_base];
//        auto rd = dynamic_cast<GReg*>( getAllocaReg(inst));
//        sequence->createLa(rd, addr);
//        auto offset = inst->getOperand(2);
//        if(dynamic_cast<ConstantInt*>(offset)){
//            int rs_ = dynamic_cast<ConstantInt*>(offset)->getValue()*4;
//            auto rs = new IConst(rs_);
//            sequence->createAdd(rd,  rs, rd);
//        }
//        else{
//            sequence->createSh2Add(rd,  dynamic_cast<GReg*>(getAllocaReg(offset)), rd);
//        }
//      
//         
//    } 
//    else if(alloca_base) {
//        auto addr = val2stack[base];
//        auto rs1 = new GReg(static_cast<int>(addr->getReg()));
//        auto rs2 = new IConst(addr->getOffset());
//        auto rd = dynamic_cast<GReg*>( getAllocaReg(inst));
//        sequence->createAdd(rd, rs1, rs2);
//    } 
//    else if(arg_base){
//        auto rs = dynamic_cast<GReg*>( getAllocaReg(base));
//        auto rd = dynamic_cast<GReg*>( getAllocaReg(inst));
//        sequence->createMv(rd, rs);
//    }
//}
//
void AsmGen::visit(GetElementPtrInst &node){
    auto inst = &node;
    auto base = inst->getOperand(0);
    auto global_base = dynamic_cast<GlobalVariable*>(base);
    auto alloca_base = dynamic_cast<AllocaInst*>(base);
    auto arg_base = dynamic_cast<Argument*>(base);
    if(global_base) {
        auto addr = global_variable_labels_table[global_base];
        auto rd = dynamic_cast<GReg*>( getAllocaReg(inst));
        sequence->createLa(rd, addr);
    } 
    else if(alloca_base) {
        auto addr = val2stack[base];
        auto rs1 = new GReg(static_cast<int>(addr->getReg()));
        auto rs2 = new IConst(addr->getOffset());
        auto rd = dynamic_cast<GReg*>( getAllocaReg(inst));
        sequence->createAdd(rd, rs1, rs2);
    } 
    else {
        auto rs = dynamic_cast<GReg*>( getAllocaReg(base));
        auto rd = dynamic_cast<GReg*>( getAllocaReg(inst));
        sequence->createMv(rd, rs);
    }
}


void AsmGen::visit(StoreInst &node){
    auto inst = &node;
    auto dst = global_variable_labels_table[dynamic_cast<GlobalVariable*>(inst->getOperand(1))];
    if(dynamic_cast<GlobalVariable*>(inst->getOperand(1))) {
        if(dynamic_cast<GlobalVariable*>(inst->getOperand(1))->getType()->getPointerElementType()->isFloatType()) {
            auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
            sequence->createFsw_label(frs1, dst);
        } 
        else {
            auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
            sequence->createSw_label(irs1, dst);     
        }
    }
}

void AsmGen::visit(MemsetInst &node){
    //结束
}

void AsmGen::visit(LoadInst &node){
    auto inst = &node;
    auto dst = global_variable_labels_table[dynamic_cast<GlobalVariable*>(inst->getOperand(0))];
    if(dynamic_cast<GlobalVariable*>(inst->getOperand(0))) {
        if(dynamic_cast<GlobalVariable*>(inst->getOperand(0))->getType()->getPointerElementType()->isFloatType()) {
            auto frs = dynamic_cast<FReg*>(getAllocaReg(inst));
            sequence->createFlw_label(frs , dst);
        } else {
            auto irs = dynamic_cast<GReg*>(getAllocaReg(inst));
            sequence->createLw_label(irs , dst);
        }
    }
}

void AsmGen::visit(AllocaInst &node){
    //结束
}

void AsmGen::visit(ZextInst &node){
    auto inst = &node;
    auto rd = dynamic_cast<GReg*>(getAllocaReg(inst));
    auto rs = dynamic_cast<GReg*>(getAllocaReg(inst->getOperand(0)));
    auto iconst_rs = dynamic_cast<ConstantInt*>(inst->getOperand(0));
    if(rs)
        sequence->createZext(rd, rs);
    else if(iconst_rs){
        int iflag = iconst_rs->getValue();

       sequence->createZextIConst(rd, iflag);
        
    }

}

void AsmGen::visit(SiToFpInst &node){
    auto inst = &node;
    auto frd = getFRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    sequence->createFcvt_s_w(frd , irs1);

}

void AsmGen::visit(FpToSiInst &node){
    auto inst = &node;
    auto grd = getGRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    sequence->createFcvt_w_s(grd, frs1);
}

void AsmGen::visit(PhiInst &node){
    //结束
}

void AsmGen::visit(CmpBrInst &node){
    //结束
}

void AsmGen::visit(FCmpBrInst &node){
    //结束
}

void AsmGen::visit(LoadOffsetInst &node){
    auto inst = &node;
    auto rs1 = dynamic_cast<GReg*>(getAllocaReg(inst->getOperand(0)));
    auto rs2 = dynamic_cast<ConstantInt*>(inst->getOffset())?new IConst(dynamic_cast<ConstantInt*>(inst->getOffset())->getValue()):getAllocaReg(inst->getOffset());

    if(inst->getLoadType()->isFloatType()) {
        auto frd = getFRD(inst);
        sequence->createFlw(frd, rs1, rs2);
    } else {
        auto grd = getGRD(inst);
        sequence->createLw(grd, rs1, rs2);
    }
}

void AsmGen::visit(StoreOffsetInst &node){
    auto inst = &node;
    auto base = dynamic_cast<GReg*>(getAllocaReg(inst->getOperand(1)));
   auto offset = dynamic_cast<ConstantInt*>(inst->getOffset())?new IConst(dynamic_cast<ConstantInt*>(inst->getOffset())->getValue()):getAllocaReg(inst->getOffset());

    if(inst->getStoreType()->isFloatType()) {
        auto src = dynamic_cast<ConstantFP*>(inst->getOperand(0))?new FConst(dynamic_cast<ConstantFP*>(inst->getOperand(0))->getValue()):getAllocaReg(inst->getOperand(0));
        sequence->createFsw(src, base, offset);
    } else {
        auto src = dynamic_cast<ConstantInt*>(inst->getOperand(0))?new IConst(dynamic_cast<ConstantInt*>(inst->getOperand(0))->getValue()):getAllocaReg(inst->getOperand(0));
        sequence->createSw(src, base, offset);
    }
}

void AsmGen::visitAdd(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst))?dynamic_cast<IConst*>(getIRS2(inst)):getIRS2(inst);
    sequence->createAdd(ird, irs1, irs2);
}

void AsmGen::visitSub(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst))?dynamic_cast<IConst*>(getIRS2(inst)):getIRS2(inst);
    sequence->createSubw(ird, irs1, irs2);
}

void AsmGen::visitMul(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst))?dynamic_cast<IConst*>(getIRS2(inst)):getIRS2(inst);
    sequence->createMulw(ird, irs1, irs2);
}

void AsmGen::visitMul64(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst))?dynamic_cast<IConst*>(getIRS2(inst)):getIRS2(inst);
    if(dynamic_cast<IConst*>(irs1) && dynamic_cast<IConst*>(irs2))
        return ;
    sequence->createMuld(ird, irs1, irs2);
}

void AsmGen::visitSDiv(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst))?dynamic_cast<IConst*>(getIRS2(inst)):getIRS2(inst);
    sequence->createDivw(ird, irs1, irs2);
}

void AsmGen::visitSRem(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst))?dynamic_cast<IConst*>(getIRS2(inst)):getIRS2(inst);
    sequence->createRemw(ird, irs1, irs2);
}

void AsmGen::visitAsr(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst));
    sequence->createSraw(ird, irs1, irs2);
}

void AsmGen::visitShl(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst));
    sequence->createSllw(ird, irs1, irs2);
}

void AsmGen::visitLsr(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst));
    sequence->createSrlw(ird, irs1, irs2);
}

void AsmGen::visitAsr64(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst));
    sequence->createSra(ird, irs1, irs2);
}

void AsmGen::visitShl64(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst));
    sequence->createSll(ird, irs1, irs2);
}

void AsmGen::visitLsr64(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst));
    sequence->createSrl(ird, irs1, irs2);
}

void AsmGen::visitLAnd(BinaryInst* inst){
    auto ird = getGRD(inst);
    auto irs1 = dynamic_cast<GReg*>(getIRS1(inst));
    auto irs2 = dynamic_cast<IConst*>(getIRS2(inst));
    sequence->createLand(ird, irs1, irs2);
}

void AsmGen::visitFAdd(BinaryInst* inst){
    auto frd = getFRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFadd_s(frd, frs1, frs2);
}

void AsmGen::visitFSub(BinaryInst* inst){
    auto frd = getFRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFsub_s(frd, frs1, frs2);
}

void AsmGen::visitFMul(BinaryInst* inst){
    auto frd = getFRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFmul_s(frd, frs1, frs2);
}

void AsmGen::visitFDiv(BinaryInst* inst){
    auto frd = getFRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFdiv_s(frd, frs1, frs2);
}

//！！！！！！！！！！！！！不考虑第二个操作数rs2！！！！！！！！！！！！
void AsmGen::visitEQ(CmpInst* inst){
    auto grd = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    sequence->createSeqz(grd,irs1);
}

void AsmGen::visitNE(CmpInst* inst){
    auto grd = getGRD(inst);
    auto irs1 = dynamic_cast<IConst*>(getIRS1(inst))?dynamic_cast<IConst*>(getIRS1(inst)):getIRS1(inst);
    sequence->createSnez(grd,irs1);
}

void AsmGen::visitFEQ(FCmpInst* inst){
    auto grd = getGRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFeq_s(grd, frs1, frs2);
}

void AsmGen::visitFGE(FCmpInst* inst){
    auto grd = getGRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFge_s(grd, frs1, frs2);
}

void AsmGen::visitFGT(FCmpInst* inst){
    auto grd = getGRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFgt_s(grd, frs1, frs2);
}

void AsmGen::visitFLE(FCmpInst* inst){
    auto grd = getGRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFle_s(grd, frs1, frs2);
}

void AsmGen::visitFLT(FCmpInst* inst){
    auto grd = getGRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFlt_s(grd, frs1, frs2);
}

void AsmGen::visitFNE(FCmpInst* inst){
    auto grd = getGRD(inst);
    auto frs1 = dynamic_cast<FConst*>(getFRS1(inst))?dynamic_cast<FConst*>(getFRS1(inst)):getFRS1(inst);
    auto frs2 = dynamic_cast<FConst*>(getFRS2(inst))?dynamic_cast<FConst*>(getFRS2(inst)):getFRS2(inst);
    sequence->createFne_s(grd, frs1, frs2);
}







//! 初赛测试中未出现寄存器移动loop的情况
std::vector<std::pair<AddressMode*, AddressMode*>> AsmGen::callee_iargs_move(Function *func) {
    std::vector<std::pair<AddressMode*, AddressMode*>> to_move_locs;
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
                int target_reg_id = ival2interval[iargs_vector[i]]->reg;
                if(is_args_moved[i]) {
                    for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                        auto iarg = riter->first;
                        to_move_locs.push_back(std::make_pair(new IRA(ival2interval[iarg]->reg), new IRA(arg_reg_base + riter->second)));
                        is_args_moved[riter->second] = true;
                    }
                    iargs_dependency_chain.clear();
                    break;
                } else if(target_reg_id < 0) {
                    int base_reg_id = static_cast<int>(val2stack[iargs_vector[i]]->getReg());
                    int offset = val2stack[iargs_vector[i]]->getOffset();
                    to_move_locs.push_back(std::make_pair(new IRIA(base_reg_id, offset), new IRA(arg_reg_base + i)));
                    is_args_moved[i] = true;
                    if(!iargs_dependency_chain.empty()) {
                        for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                            auto iarg = riter->first;
                            to_move_locs.push_back(std::make_pair(new IRA(ival2interval[iarg]->reg), new IRA(arg_reg_base + riter->second)));
                            is_args_moved[riter->second] = true;
                        }
                        iargs_dependency_chain.clear();
                    } 
                    break;
                } else if(target_reg_id - arg_reg_base == i) {
                    is_args_moved[i] = true;
                    break;
                } else if(target_reg_id - arg_reg_base >= first_iargs_num || target_reg_id - arg_reg_base < 0) {
                    to_move_locs.push_back(std::make_pair(new IRA(target_reg_id), new IRA(arg_reg_base + i)));
                    is_args_moved[i] = true;
                    if(!iargs_dependency_chain.empty()) {
                        for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                            auto iarg = riter->first;
                            to_move_locs.push_back(std::make_pair(new IRA(ival2interval[iarg]->reg), new IRA(arg_reg_base + riter->second)));
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
                        to_move_locs.push_back(std::make_pair(new IRA(static_cast<int>(RISCV::GPR::s1)), new IRA(arg_reg_base + iargs_dependency_chain.rbegin()->second)));
                        for(auto riter = iargs_dependency_chain.rbegin(); riter != iargs_dependency_chain.rend(); riter++) {
                            if(riter->first == iargs_dependency_chain.rbegin()->first)
                                continue;
                            auto iarg = riter->first;
                            to_move_locs.push_back(std::make_pair(new IRA(ival2interval[iarg]->reg), new IRA(arg_reg_base + riter->second)));
                            is_args_moved[riter->second] = true;
                        }
                        to_move_locs.push_back(std::make_pair(new IRA(ival2interval[iargs_dependency_chain.rbegin()->first]->reg), new IRA(static_cast<int>(RISCV::GPR::s1))));
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
                        to_move_locs.push_back(std::make_pair(new IRA(ival2interval[iarg]->reg), new IRA(arg_reg_base + riter->second)));
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
            int target_reg_id = ival2interval[iargs_vector[i]]->reg;
            if(target_reg_id < 0) {
                continue;
            } else {
                to_move_locs.push_back(std::make_pair(new IRA(target_reg_id), new IRIA(static_cast<int>(RISCV::GPR::s0), reg_size * (i - first_iargs_num))));
            }
        } 
    }

    return to_move_locs;
}

//! 初赛测试中未出现寄存器移动loop的情况
std::vector<std::pair<AddressMode*, AddressMode*>> AsmGen::callee_fargs_move(Function *func) {
    std::vector<std::pair<AddressMode*, AddressMode*>> to_move_locs;

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
                int target_reg_id = fval2interval[fargs_vector[i]]->reg;
                if(is_args_moved[i]) {
                    for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                        auto farg = riter->first;
                        to_move_locs.push_back(std::make_pair(new FRA(fval2interval[farg]->reg), new FRA(arg_reg_base + riter->second)));
                        is_args_moved[riter->second] = true;
                    }
                    fargs_dependency_chain.clear();
                    break;
                } else if(target_reg_id < 0) {
                    int base_reg_id = static_cast<int>( val2stack[fargs_vector[i]]->getReg());
                    int offset = val2stack[fargs_vector[i]]->getOffset();
                    to_move_locs.push_back(std::make_pair(new IRIA(base_reg_id, offset), new FRA(arg_reg_base + i)));
                    is_args_moved[i] = true;
                    if(!fargs_dependency_chain.empty()) {
                        for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                            auto farg = riter->first;
                            to_move_locs.push_back(std::make_pair(new FRA(fval2interval[farg]->reg), new FRA(arg_reg_base + riter->second)));
                            is_args_moved[riter->second] = true;
                        }
                        fargs_dependency_chain.clear();
                    } 
                    break;
                } else if(target_reg_id - arg_reg_base == i) {
                    is_args_moved[i] = true;
                    break;
                } else if(target_reg_id - arg_reg_base >= first_fargs_num || target_reg_id - arg_reg_base < 0) {
                    to_move_locs.push_back(std::make_pair(new FRA(target_reg_id), new FRA(arg_reg_base + i)));
                    is_args_moved[i] = true;
                    int target = i;
                    if(!fargs_dependency_chain.empty()) {
                        for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                            auto farg = riter->first;
                            to_move_locs.push_back(std::make_pair(new FRA(fval2interval[farg]->reg), new FRA(arg_reg_base + riter->second)));
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
                        to_move_locs.push_back(std::make_pair(new FRA(static_cast<int>(RISCV::FPR::fs1)), new FRA(arg_reg_base + fargs_dependency_chain.rbegin()->second)));
                        for(auto riter = fargs_dependency_chain.rbegin(); riter != fargs_dependency_chain.rend(); riter++) {
                            if(riter->first == fargs_dependency_chain.rbegin()->first)
                                continue;
                            auto farg = riter->first;
                            to_move_locs.push_back(std::make_pair(new FRA(fval2interval[farg]->reg), new FRA(arg_reg_base + riter->second)));
                            is_args_moved[riter->second] = true;
                        }
                        to_move_locs.push_back(std::make_pair(new FRA(fval2interval[fargs_dependency_chain.rbegin()->first]->reg), new FRA(static_cast<int>(RISCV::FPR::fs1))));
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
                        to_move_locs.push_back(std::make_pair(new FRA(fval2interval[farg]->reg), new FRA(arg_reg_base + riter->second)));
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
            int target_reg_id = fval2interval[fargs_vector[i]]->reg;
            if(target_reg_id < 0) {
                continue;
            } else {
                to_move_locs.push_back(std::make_pair(new FRA(target_reg_id), new IRIA(static_cast<int>(RISCV::GPR::s0), reg_size * (iargs_num + i - first_fargs_num))));
            }
        } 
    }
    return to_move_locs;
}




void AsmGen::ld_tmp_regs_for_inst(Instruction *inst) {
    if(inst->isAlloca() || inst->isPhi())
        return ;

    std::set<int> to_del_tmp_iregs_set;
    std::set<int> to_ld_tmp_iregs_set;
    std::set<int> to_del_tmp_fregs_set;
    std::set<int> to_ld_tmp_fregs_set;

    std::vector<std::pair<IRA*, IRIA*>> to_ld_tmp_iregs;
    std::vector<std::pair<FRA*, IRIA*>> to_ld_tmp_fregs;
    
    for(auto opr: inst->getOperands()) {
        if(dynamic_cast<Constant*>(opr) ||
        dynamic_cast<BasicBlock*>(opr) ||
        dynamic_cast<GlobalVariable*>(opr) ||
        dynamic_cast<AllocaInst*>(opr) ||
        dynamic_cast<Function*>(opr)) {
            continue;
        }
        if(opr->getType()->isFloatType()) {
            int opr_reg = fval2interval[opr]->reg;
            if(opr_reg >= 0) {
                if(cur_tmp_fregs.find(opr_reg) != cur_tmp_fregs.end()) {
                    to_ld_tmp_fregs_set.insert(opr_reg);
                    to_del_tmp_fregs_set.insert(opr_reg);
                }
            } 
        } else {
            int opr_reg = ival2interval[opr]->reg;
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
            int inst_reg_id = fval2interval[inst]->reg;
            if(inst_reg_id >= 0) {
                if(cur_tmp_fregs.find(inst_reg_id) != cur_tmp_fregs.end()) {
                    to_del_tmp_fregs_set.insert(inst_reg_id);
                }
            }
        } else {
            int inst_reg_id = ival2interval[inst]->reg;
            if(inst_reg_id >= 0) {
                if(cur_tmp_iregs.find(inst_reg_id) != cur_tmp_iregs.end()) {
                    to_del_tmp_iregs_set.insert(inst_reg_id);
                }
            }
        }
    }

    for(auto ld_reg: to_ld_tmp_iregs_set) {        
        IRIA* regbase = tmp_iregs_loc[ld_reg];
        to_ld_tmp_iregs.push_back(std::make_pair(new IRA(ld_reg), regbase));
    }

    for(auto ld_reg: to_ld_tmp_fregs_set) {
        IRIA* regbase = tmp_fregs_loc[ld_reg];
        to_ld_tmp_fregs.push_back(std::make_pair(new FRA(ld_reg), regbase));
    }

    if(! to_ld_tmp_iregs.empty())
        sequence->createLoadTmpRegs(to_ld_tmp_iregs);
    
    if(! to_ld_tmp_fregs.empty())
        sequence->createLoadTmpRegs(to_ld_tmp_fregs);

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




//! 初赛测试中未出现一个寄存器向多个寄存器中移动的情况
std::vector<std::pair<AddressMode*, AddressMode*>> AsmGen::caller_fargs_move(CallInst *call) {
    std::vector<std::pair<AddressMode*, AddressMode*>> to_move_locs;

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
            int reg_id = fval2interval[fargs[i]]->reg;
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
            auto reg_id = fval2interval[fargs[i]]->reg;
            if(reg_id >= 0) {
                to_move_locs.push_back(std::make_pair(new IRIA(static_cast<int>(RISCV::GPR::sp), extra_stack_offset + caller_trans_args_stack_offset), new FRA(reg_id)));
            } else {
                to_move_locs.push_back(std::make_pair(new IRIA(static_cast<int>(RISCV::GPR::sp), extra_stack_offset + caller_trans_args_stack_offset), val2stack[fargs[i]]));
            }
        } else {
            auto const_fp = dynamic_cast<ConstantFP*>(fargs[i]);
            if(const_fp) {
                to_move_locs.push_back(std::make_pair(new IRIA(static_cast<int>(RISCV::GPR::sp), extra_stack_offset + caller_trans_args_stack_offset), new FConstPool(const_fp->getValue())));
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
                            to_move_locs.push_back(std::make_pair(new FRA(static_cast<int>(RISCV::FPR::fs1)), new FRA(fval2interval[reg_dependency_chain.rbegin()->first]->reg)));
                            for(auto riter= reg_dependency_chain.rbegin(); riter != reg_dependency_chain.rend(); riter++) {
                                if(riter->first == reg_dependency_chain.rbegin()->first)
                                    continue;
                                int arg_no = riter->second;
                                int src_reg_id = fval2interval[riter->first]->reg;
                                to_move_locs.push_back(std::make_pair(new FRA(arg_no+arg_reg_base), new FRA(src_reg_id)));
                                is_args_moved[arg_no] = true;        
                                reg2fargnos[src_reg_id].erase(arg_no);
                                if(reg2fargnos[src_reg_id].empty())
                                    reg2fargnos.erase(src_reg_id);
                            }
                            to_move_locs.push_back(std::make_pair(new FRA(reg_dependency_chain.rbegin()->second + arg_reg_base), new FRA(static_cast<int>(RISCV::FPR::fs1))));
                            int arg_no = reg_dependency_chain.rbegin()->second;
                            int src_reg_id = fval2interval[reg_dependency_chain.rbegin()->first]->reg;
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
                    to_move_locs.push_back(std::make_pair(new FRA(i + arg_reg_base), new FConstPool(const_fp->getValue())));
                    is_args_moved[i] = true;
                    break;
                } else {
                    int src_reg_id = fval2interval[fargs[i]]->reg;
                    if(src_reg_id < 0) {
                        IRIA *regbase = val2stack[fargs[i]];
                        to_move_locs.push_back(std::make_pair(new FRA(i + arg_reg_base), regbase));
                        is_args_moved[i] = true;
                        break;
                    } else {
                        to_move_locs.push_back(std::make_pair(new FRA(i + arg_reg_base), new FRA(src_reg_id)));
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
                                    to_move_locs.push_back(std::make_pair(new FRA(fargno + arg_reg_base), new FConstPool(const_fp->getValue())));
                                    is_args_moved[fargno] = true;
                                    break;
                                } else {
                                    int src_reg_id = fval2interval[farg]->reg;
                                    if(src_reg_id < 0) {
                                        IRIA *regbase = val2stack[farg];
                                        to_move_locs.push_back(std::make_pair(new FRA(fargno + arg_reg_base), regbase));
                                        is_args_moved[fargno] = true;
                                        break;
                                    } else {
                                        if(reg2fargnos.find(fargno + arg_reg_base) != reg2fargnos.end()) 
                                            break;
                                        to_move_locs.push_back(std::make_pair(new FRA(fargno + arg_reg_base), new FRA(src_reg_id)));
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



//! 初赛测试中未出现一个寄存器向多个寄存器中移动的情况
std::vector<std::pair<AddressMode*, AddressMode*>> AsmGen::caller_iargs_move(CallInst *call) {
    std::vector<std::pair<AddressMode*, AddressMode*>> to_move_locs;

    std::vector<Value*> iargs;
    for(auto arg: call->getOperands()) {
        if(dynamic_cast<Function*>(arg))
            continue;
        if(!arg->getType()->isFloatType()) 
            iargs.push_back(arg);
    }

    int num_of_iargs = iargs.size() > 8 ? 8 : iargs.size(); //参数个数

    std::map<int, bool> is_args_moved;

    std::map<int, std::set<int>> reg2iargnos;

    std::list<std::pair<Value*, int>> reg_dependency_chain;


    for(int i = 0; i < num_of_iargs; i++) {
        is_args_moved[i] = false;
        if(ival2interval.find(iargs[i]) != ival2interval.end()) {
            int reg_id = ival2interval[iargs[i]]->reg;
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
            auto reg_id = ival2interval[iargs[i]]->reg;
            if(reg_id >= 0) {
                to_move_locs.push_back(std::make_pair(new IRIA(static_cast<int>(RISCV::GPR::sp), extra_stack_offset + caller_trans_args_stack_offset), new IRA(reg_id)));
            } else {
                to_move_locs.push_back(std::make_pair(new IRIA(static_cast<int>(RISCV::GPR::sp), extra_stack_offset + caller_trans_args_stack_offset), val2stack[iargs[i]]));
            }
        } else {
            auto const_int = dynamic_cast<ConstantInt*>(iargs[i]);
            if(const_int) {
                to_move_locs.push_back(std::make_pair(new IRIA(static_cast<int>(RISCV::GPR::sp), extra_stack_offset + caller_trans_args_stack_offset), new IConstPool(const_int->getValue())));
            } else {
              //  LOG(ERROR) << "汇编代码生成发生错误";
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
                            to_move_locs.push_back(std::make_pair(new IRA(static_cast<int>(RISCV::GPR::s1)), new IRA(ival2interval[reg_dependency_chain.rbegin()->first]->reg)));
                            for(auto riter= reg_dependency_chain.rbegin(); riter != reg_dependency_chain.rend(); riter++) {
                                if(riter->first == reg_dependency_chain.rbegin()->first)
                                    continue;
                                int arg_no = riter->second;
                                int src_reg_id = ival2interval[riter->first]->reg;
                                to_move_locs.push_back(std::make_pair(new IRA(arg_no+arg_reg_base), new IRA(src_reg_id)));
                                is_args_moved[arg_no] = true;        
                                reg2iargnos[src_reg_id].erase(arg_no);
                                if(reg2iargnos[src_reg_id].empty())
                                    reg2iargnos.erase(src_reg_id);
                            }
                            to_move_locs.push_back(std::make_pair(new IRA(reg_dependency_chain.rbegin()->second + arg_reg_base), new IRA(static_cast<int>(RISCV::GPR::s1))));
                            int arg_no = reg_dependency_chain.rbegin()->second;
                            int src_reg_id = ival2interval[reg_dependency_chain.rbegin()->first]->reg;
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
                    to_move_locs.push_back(std::make_pair(new IRA(i+arg_reg_base), new IConstPool(dynamic_cast<ConstantInt*>(iargs[i])->getValue())));
                    is_args_moved[i] = true;
                    break;
                } else {
                    int src_reg_id = ival2interval[iargs[i]]->reg;
                    if(src_reg_id < 0) {
                        IRIA *regbase = val2stack[iargs[i]];
                        to_move_locs.push_back(std::make_pair(new IRA(i+arg_reg_base), regbase));
                        is_args_moved[i] = true;
                        break;
                    } else {
                        to_move_locs.push_back(std::make_pair(new IRA(i+arg_reg_base), new IRA(src_reg_id)));
                        is_args_moved[i] = true;
                        reg2iargnos[src_reg_id].erase(i);
                        if(reg2iargnos[src_reg_id].empty())
                            reg2iargnos.erase(src_reg_id);
                        if(!reg_dependency_chain.empty()) {
                            for(auto riter = reg_dependency_chain.rbegin(); riter != reg_dependency_chain.rend(); riter++) {
                                auto iarg = riter->first;
                                auto iargno = riter->second;
                                if(ival2interval.find(iargs[iargno]) == ival2interval.end()) {
                                    to_move_locs.push_back(std::make_pair(new IRA(iargno+arg_reg_base), new IConstPool(dynamic_cast<ConstantInt*>(iarg)->getValue())));
                                    is_args_moved[iargno] = true;
                                    break;
                                } else {
                                    int src_reg_id = ival2interval[iarg]->reg;
                                    if(src_reg_id < 0) {
                                        IRIA *regbase = val2stack[iarg];
                                        to_move_locs.push_back(std::make_pair(new IRA(iargno+arg_reg_base), regbase));
                                        is_args_moved[iargno] = true;
                                        break;
                                    } else {
                                        if(reg2iargnos.find(iargno + arg_reg_base) != reg2iargnos.end()) 
                                            break;
                                        to_move_locs.push_back(std::make_pair(new IRA(iargno+arg_reg_base), new IRA(src_reg_id)));
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




std::vector<std::pair<AddressMode*, AddressMode*>> AsmGen::idata_move(std::vector<AddressMode*>&srcs, std::vector<AddressMode*>&dsts) {
 
    std::map<int, bool> is_data_moved;
    std::list<int> loc_dependency_chain;
    std::map<AddressMode*, std::set<int>> src2dstnos;

    std::vector<std::pair<AddressMode*, AddressMode*>> to_move_locs;

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
                            to_move_locs.push_back(std::make_pair(new IRA(static_cast<int>(RISCV::GPR::s1)) , srcs[*loc_dependency_chain.rbegin()]));
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
                            to_move_locs.push_back(std::make_pair(dsts[*loc_dependency_chain.rbegin()] , new IRA(static_cast<int>(RISCV::GPR::s1))));
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

std::vector<std::pair<AddressMode*, AddressMode*>> AsmGen::fdata_move(std::vector<AddressMode*>&srcs, std::vector<AddressMode*>&dsts) {
 
    std::map<int, bool> is_data_moved;
    std::list<int> loc_dependency_chain;
    std::map<AddressMode*, std::set<int>> src2dstnos;

    std::vector<std::pair<AddressMode*, AddressMode*>> to_move_locs;

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
                            to_move_locs.push_back(std::make_pair(new FRA(static_cast<int>(RISCV::FPR::fs1)), srcs[*loc_dependency_chain.rbegin()]));
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
                            to_move_locs.push_back(std::make_pair(dsts[*loc_dependency_chain.rbegin()], new FRA(static_cast<int>(RISCV::FPR::fs1))));
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

Val* AsmGen::getAllocaReg(Value *value) {
    if(value->getType()->isFloatType()) {
        auto iter = fval2interval.find(value);
        if(iter != fval2interval.end()) {
            int reg_id = static_cast<int>( iter->second->reg);
            return new FReg(reg_id);
        } 
    } else {
        auto iter = ival2interval.find(value);
        if(iter != ival2interval.end()) {
            int reg_id = static_cast<int>( iter->second->reg);
            return new GReg(reg_id);
        }
    }
    return nullptr;
}

GReg* AsmGen::getGRD(Instruction* inst){
    return dynamic_cast<GReg*>(getAllocaReg(inst));
}

FReg* AsmGen::getFRD(Instruction* inst){
    return dynamic_cast<FReg*>(getAllocaReg(inst));
} 

Val* AsmGen::getIRS1(Instruction* inst){
    auto iconst_rs1 = dynamic_cast<ConstantInt*>(inst->getOperand(0));
    if(iconst_rs1)
        return new IConst(iconst_rs1->getValue());
    return getAllocaReg(inst->getOperand(0));
}

Val* AsmGen::getIRS2(Instruction* inst){
    auto iconst_rs2 = dynamic_cast<ConstantInt*>(inst->getOperand(1));
    if(iconst_rs2)
        return new IConst(iconst_rs2->getValue());
    return getAllocaReg(inst->getOperand(1));
}

Val* AsmGen::getFRS1(Instruction* inst){
    auto fconst_rs1 = dynamic_cast<ConstantFP*>(inst->getOperand(0));
    if(fconst_rs1)
        return new FConst(fconst_rs1->getValue());
    return getAllocaReg(inst->getOperand(0));
}

Val* AsmGen::getFRS2(Instruction* inst){
    auto fconst_rs2 = dynamic_cast<ConstantFP*>(inst->getOperand(1));
    if(fconst_rs2)
        return new FConst(fconst_rs2->getValue());
    return getAllocaReg(inst->getOperand(1));
}



void AsmGen::tmp_regs_restore() {
    
    std::vector<std::pair<IRA*, IRIA*>> to_restore_iregs;
    std::vector<std::pair<FRA*, IRIA*>> to_restore_fregs;

    for(auto &[ireg_id, loc]: tmp_iregs_loc) {
        to_restore_iregs.push_back(std::make_pair(new IRA(ireg_id), loc));
    }

    for(auto &[freg_id, loc]: tmp_fregs_loc) {
        to_restore_fregs.push_back(std::make_pair(new FRA(freg_id), loc));
    }
    tmp_iregs_loc.clear();
    tmp_fregs_loc.clear();
    cur_tmp_iregs.clear();
    cur_tmp_fregs.clear();
    free_locs_for_tmp_regs_saved.clear();
    cur_tmp_reg_saved_stack_offset = 0;

    if(! to_restore_iregs.empty())
        sequence->createStoreTmpRegs(to_restore_iregs);

    if(! to_restore_fregs.empty())
        sequence->createStoreTmpRegs(to_restore_fregs);
    return ;
}

void AsmGen::phi_union(Instruction *br_inst) {
    
    PhiPass *succ_move_inst = nullptr;
    PhiPass *fail_move_inst = nullptr;
    PhiPass **move_inst;

    AsmInst *succ_br_inst = nullptr;
    AsmInst *fail_br_inst = nullptr;
    

    std::vector<AddressMode*> phi_itargets;
    std::vector<AddressMode*> phi_isrcs;
    std::vector<AddressMode*> phi_ftargets;
    std::vector<AddressMode*> phi_fsrcs;

    //& 保证寄存器地址的唯一性
    std::map<int, AddressMode*> ireg2loc;
    std::map<int, AddressMode*> freg2loc;

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

    for(auto sux: sequence->getBBOfSeq()->getSuccBasicBlocks()) {
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
                int target_reg_id = fval2interval[inst]->reg;
                AddressMode *target_loc_ptr = nullptr;
                if(target_reg_id >= 0) {
                    if(freg2loc.find(target_reg_id) == freg2loc.end()) 
                        freg2loc.insert({target_reg_id, new FRA(target_reg_id)});
                    target_loc_ptr = freg2loc[target_reg_id];
                } else {
                    target_loc_ptr = val2stack[inst];
                }
                for(auto opr: inst->getOperands()) {
                    if(dynamic_cast<BasicBlock*>(opr)) {
                        auto this_bb = dynamic_cast<BasicBlock*>(opr);
                        if(this_bb != sequence->getBBOfSeq())
                            continue;
                        if(dynamic_cast<ConstantFP*>(lst_val)) {
                            auto const_val = dynamic_cast<ConstantFP*>(lst_val);
                            auto src = new FConstPool(const_val->getValue());
                            phi_fsrcs.push_back(src);
                            phi_ftargets.push_back(target_loc_ptr);
                        } else {
                            int src_reg_id = fval2interval[lst_val]->reg;
                            if(src_reg_id >= 0) {
                                if(freg2loc.find(src_reg_id) == freg2loc.end())
                                    freg2loc.insert({src_reg_id, new FRA(src_reg_id)});
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
                        if(opr == nullptr){}
                            //LOG(ERROR) << "err";
                        lst_val = opr;
                    }
                }
            } else {
                int target_reg_id = ival2interval[inst]->reg;
                AddressMode *target_loc_ptr = nullptr;
                if(target_reg_id >= 0) {
                    if(ireg2loc.find(target_reg_id) == ireg2loc.end()) 
                        ireg2loc.insert({target_reg_id, new IRA(target_reg_id)});
                    target_loc_ptr = ireg2loc[target_reg_id];
                } else {
                    target_loc_ptr = val2stack[inst];
                }
                for(auto opr: inst->getOperands()) {
                    if(dynamic_cast<BasicBlock*>(opr)) {
                        auto this_bb = dynamic_cast<BasicBlock*>(opr);
                        if(this_bb != sequence->getBBOfSeq()) 
                            continue;
                        if(dynamic_cast<ConstantInt*>(lst_val)) {
                            auto const_val = dynamic_cast<ConstantInt*>(lst_val);
                            auto src = new IConstPool(const_val->getValue());
                            phi_isrcs.push_back(src);
                            phi_itargets.push_back(target_loc_ptr);
                        } else if(ival2interval.find(lst_val)!=ival2interval.end()){
                            int src_reg_id = ival2interval[lst_val]->reg;
                            if(src_reg_id >= 0) {
                                if(ireg2loc.find(src_reg_id) == ireg2loc.end()) 
                                    ireg2loc.insert({src_reg_id, new IRA(src_reg_id)});
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

        std::vector<std::pair<AddressMode*, AddressMode*>> to_move_ilocs;
        std::vector<std::pair<AddressMode*, AddressMode*>> to_move_flocs;

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
            *move_inst = sequence->createPhiPass(to_move_ilocs, to_move_flocs);
            sequence->deleteInst();
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
                        succ_br_inst = sequence->createFBeq(new FConst(const_cond1->getValue()), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBeq(new FConst(const_cond1->getValue()), new Mem(regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBeq(new FConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBeq(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBeq(getAllocaReg(cond1), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createFBeq(new Mem(regbase1->getOffset(), static_cast<int>(regbase1->getReg()) ), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBeq(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBeq(getAllocaReg(cond1), new Mem(regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBeq(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();
                }
                break;

            case CmpOp::GE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = sequence->createFBge(new FConst(const_cond1->getValue()), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBge(new FConst(const_cond1->getValue()), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBge(new FConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBge(new Mem(regbase->getOffset(), static_cast<int>(regbase->getReg())), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBge(getAllocaReg(cond1), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createFBge(new Mem(regbase1->getOffset(), static_cast<int>( regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBge(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBge(getAllocaReg(cond1), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBge(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();
                }
                break;
            case CmpOp::GT: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = sequence->createFBgt(new FConst(const_cond1->getValue()), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBgt(new FConst(const_cond1->getValue()), new Mem(regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBgt(new FConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBgt(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBgt(getAllocaReg(cond1), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createFBgt(new Mem( regbase1->getOffset(), static_cast<int>(regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBgt(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBgt(getAllocaReg(cond1), new Mem(regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBgt(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();
                }
                break; 

            case CmpOp::LE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = sequence->createFBle(new FConst(const_cond1->getValue()), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBle(new FConst(const_cond1->getValue()), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBle(new FConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBle(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBle(getAllocaReg(cond1), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createFBle(new Mem( regbase1->getOffset(), static_cast<int>(regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBle(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBle(getAllocaReg(cond1), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBle(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();
                }
                break;
            
            case CmpOp::LT: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = sequence->createFBlt(new FConst(const_cond1->getValue()), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBlt(new FConst(const_cond1->getValue()), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBlt(new FConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBlt(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBlt(getAllocaReg(cond1), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createFBlt(new Mem( regbase1->getOffset(), static_cast<int>(regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBlt(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBlt(getAllocaReg(cond1), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBlt(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();
                }
                break;

            case CmpOp::NE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = sequence->createFBne(new FConst(const_cond1->getValue()), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                     
                    } else if(const_cond1) {
                        if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBne(new FConst(const_cond1->getValue()), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBne(new FConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBne(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBne(getAllocaReg(cond1), new FConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createFBne(new Mem( regbase1->getOffset(), static_cast<int>(regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createFBne(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(fval2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createFBne(getAllocaReg(cond1), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createFBne(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();
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
                        succ_br_inst = sequence->createBeq(new IConst(const_cond1->getValue()), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                  
                    } else if(const_cond1) {
                        if(ival2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createBeq(new IConst(const_cond1->getValue()), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBeq(new IConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(ival2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createBeq(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBeq(getAllocaReg(cond1), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(ival2interval[cond1]->reg < 0 && ival2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createBeq(new Mem( regbase1->getOffset(), static_cast<int>(regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(ival2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createBeq(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(ival2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createBeq(getAllocaReg(cond1), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBeq(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();  
                }
                break;

            case CmpOp::GE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = sequence->createBge(new IConst(const_cond1->getValue()), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                  
                    } else if(const_cond1) {
                        if(ival2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createBge(new IConst(const_cond1->getValue()), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBge(new IConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(ival2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createBge(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBge(getAllocaReg(cond1), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(ival2interval[cond1]->reg < 0 && ival2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createBge(new Mem( regbase1->getOffset(), static_cast<int>(regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(ival2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createBge(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(ival2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createBge(getAllocaReg(cond1), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBge(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();  
                }
                break;
            case CmpOp::GT: {}
                //LOG(ERROR) << "phi union出现异常";
                break; 

            case CmpOp::LE: {}
               // LOG(ERROR) << "phi union出现异常";
                break;
            
            case CmpOp::LT: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = sequence->createBlt(new IConst(const_cond1->getValue()), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                  
                    } else if(const_cond1) {
                        if(ival2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createBlt(new IConst(const_cond1->getValue()), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBlt(new IConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(ival2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createBlt(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBlt(getAllocaReg(cond1), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(ival2interval[cond1]->reg < 0 && ival2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createBlt(new Mem( regbase1->getOffset(), static_cast<int>(regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(ival2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createBlt(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(ival2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createBlt(getAllocaReg(cond1), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBlt(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();  
                }
                break;

            case CmpOp::NE: {
                    if(const_cond1 && const_cond2) {
                        succ_br_inst = sequence->createBne(new IConst(const_cond1->getValue()), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                        sequence->deleteInst();                  
                    } else if(const_cond1) {
                        if(ival2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createBne(new IConst(const_cond1->getValue()), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBne(new IConst(const_cond1->getValue()), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else if(const_cond2) {
                        if(ival2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createBne(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBne(getAllocaReg(cond1), new IConst(const_cond2->getValue()), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    } else {
                        if(ival2interval[cond1]->reg < 0 && ival2interval[cond2]->reg < 0) {
                            auto regbase1 = val2stack[cond1];
                            auto regbase2 = val2stack[cond2];
                            succ_br_inst = sequence->createBne(new Mem( regbase1->getOffset(), static_cast<int>(regbase1->getReg())), new Mem( regbase2->getOffset(), static_cast<int>(regbase2->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(ival2interval[cond1]->reg < 0) {
                            auto regbase = val2stack[cond1];
                            succ_br_inst = sequence->createBne(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else if(ival2interval[cond2]->reg < 0) {
                            auto regbase = val2stack[cond2];
                            succ_br_inst = sequence->createBne(getAllocaReg(cond1), new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), bb2label[succ_bb]);
                            sequence->deleteInst();
                        } else {
                            succ_br_inst = sequence->createBne(getAllocaReg(cond1), getAllocaReg(cond2), bb2label[succ_bb]);
                            sequence->deleteInst();
                        }
                    }
                    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
                    sequence->deleteInst();  
                }
                break;

            default:
                break;
        }
    } else {
        if(br_inst->getNumOperands() == 1) {
            succ_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(0));
            succ_br_inst = sequence->createJump(bb2label[succ_bb]);
            sequence->deleteInst();
        } else {
            succ_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(1));
            fail_bb = dynamic_cast<BasicBlock*>(br_inst->getOperand(2));
            auto cond = br_inst->getOperand(0);
            auto const_cond = dynamic_cast<ConstantInt*>(cond);
            if(const_cond) {
                succ_br_inst = sequence->createBne(new IConst(const_cond->getValue()), new GReg(static_cast<int>(RISCV::GPR::zero)), bb2label[succ_bb]);
                sequence->deleteInst();
            } else if(ival2interval[cond]->reg < 0) {
                auto regbase = val2stack[cond];
                succ_br_inst = sequence->createBne(new Mem( regbase->getOffset(), static_cast<int>(regbase->getReg())), new GReg(static_cast<int>(RISCV::GPR::zero)), bb2label[succ_bb]);
                sequence->deleteInst();
            } else {
                succ_br_inst = sequence->createBne(getAllocaReg(cond), new GReg(static_cast<int>(RISCV::GPR::zero)), bb2label[succ_bb]);
                sequence->deleteInst();
            }
            fail_br_inst = sequence->createJump(bb2label[fail_bb]);
            sequence->deleteInst();
        }
    }

    if(is_fcmpbr) {
        if(have_succ_move) {}
            //LOG(ERROR) << "出现未预期情况";
            if(succ_move_inst)
                sequence->appendInst(succ_move_inst);
        if(succ_br_inst)
            sequence->appendInst(succ_br_inst);
        if(fail_move_inst)
            sequence->appendInst(fail_move_inst);
        if(fail_br_inst)
            sequence->appendInst(fail_br_inst);
        
    } else if(is_cmpbr) {
        if(have_succ_move) {}
        //    LOG(ERROR) << "出现未预期情况";
        if(succ_move_inst)
                sequence->appendInst(succ_move_inst);

        if(succ_br_inst)
            sequence->appendInst(succ_br_inst);
        if(fail_move_inst)
            sequence->appendInst(fail_move_inst);
        if(fail_br_inst)
            sequence->appendInst(fail_br_inst);

    } else {
        if(br_inst->getNumOperands() == 1) {
            if(succ_move_inst)
                sequence->appendInst(succ_move_inst);
            if(succ_br_inst)
                sequence->appendInst(succ_br_inst);

        } else {
            if(have_succ_move) {}
               // LOG(ERROR) << "出现未预期情况";

            if(succ_br_inst)
                sequence->appendInst(succ_br_inst);
            if(fail_move_inst)
                sequence->appendInst(fail_move_inst);
            if(fail_br_inst)
                sequence->appendInst(fail_br_inst);
        }
    }
}

int AsmGen::setCallerAndCalleeRegs(){
    used_iregs_pair.first.clear();
    used_iregs_pair.second.clear();
    used_fregs_pair.first.clear();
    used_fregs_pair.second.clear();
    //& build reg -> values map or val -> stack map
 //  for(auto iter: ival2interval) {
 //      Interval *interval_iter = iter.second;
 //      if(interval_iter){
 //      if(interval_iter->reg >= 0) {
 //          auto iter = ::std::find(icallee.begin(), icallee.end(), interval_iter->reg);
 //          if(iter != icallee.end()) {
 //              used_iregs_pair.second.push_back(interval_iter->reg);
 //          } else {
 //              used_iregs_pair.first.push_back(interval_iter->reg);
 //          }
 //      }
 //      }
 //  }

 //  for(auto iter: fval2interval) {
 //      Value *val_iter = iter.first;
 //      Interval *interval_iter = iter.second;
 //      if(interval_iter){
 //      if(interval_iter->reg >= 0) {
 //          auto iter = ::std::find(fcallee.begin(), fcallee.end(), interval_iter->reg);
 //          if(iter != fcallee.end()) {
 //              used_fregs_pair.second.push_back(interval_iter->reg);
 //          } else {
 //              used_fregs_pair.first.push_back(interval_iter->reg);
 //          }
 //      }
 //      }
 //  }

   ::std::vector<int> all_iregs = {};
   ::std::vector<int> all_fregs = {};
   for(auto i: ival2interval)
       if(i.second && i.second->reg>-1)    
           all_iregs.push_back(i.second->reg);

   for(auto f: fval2interval)
       if(f.second && f.second->reg>-1)    
           all_fregs.push_back(f.second->reg);
   
   ::std::sort(all_iregs.begin(), all_iregs.end());
   ::std::sort(all_fregs.begin(), all_fregs.end());

   ::std::set_intersection(all_iregs.begin(), all_iregs.end(),
                           icallee.begin(), icallee.end(),
                           ::std::back_inserter(used_iregs_pair.second));

   ::std::set_intersection(all_fregs.begin(), all_fregs.end(),
                           fcallee.begin(), fcallee.end(),
                           ::std::back_inserter(used_fregs_pair.second));

   ::std::set_difference(all_iregs.begin(), all_iregs.end(),
                         used_iregs_pair.second.begin(), used_iregs_pair.second.end(),
                         ::std::back_inserter(used_iregs_pair.first));

   ::std::set_difference(all_fregs.begin(), all_fregs.end(),
                         used_fregs_pair.second.begin(), used_fregs_pair.second.end(),
                         ::std::back_inserter(used_fregs_pair.first));


    

    //& 总是保存fp,ra,s1寄存器
    used_iregs_pair.second.push_back(static_cast<int>(RISCV::GPR::ra));
    used_iregs_pair.second.push_back(static_cast<int>(RISCV::GPR::s0));

    ::std::sort(used_iregs_pair.first.begin(), used_iregs_pair.first.end());
    ::std::sort(used_iregs_pair.second.begin(), used_iregs_pair.second.end());
    ::std::sort(used_fregs_pair.first.begin(), used_fregs_pair.first.end());
    ::std::sort(used_fregs_pair.second.begin(), used_fregs_pair.second.end());

    auto last_i_f = ::std::unique(used_iregs_pair.first.begin(), used_iregs_pair.first.end());
    auto last_i_s = ::std::unique(used_iregs_pair.second.begin(), used_iregs_pair.second.end());
    auto last_f_f = ::std::unique(used_fregs_pair.first.begin(), used_fregs_pair.first.end());
    auto last_f_s = ::std::unique(used_fregs_pair.second.begin(), used_fregs_pair.second.end());

    used_iregs_pair.first.erase(last_i_f, used_iregs_pair.first.end());
    used_iregs_pair.second.erase(last_i_s, used_iregs_pair.second.end());
    used_fregs_pair.first.erase(last_f_f, used_fregs_pair.first.end());
    used_fregs_pair.second.erase(last_f_s, used_fregs_pair.second.end());
    
    return reg_size * (used_iregs_pair.second.size() + used_fregs_pair.second.size());

}


int AsmGen::allocateMemForIArgs(){
    //! val2stack记录了通过栈传递给callee的参数,在callee中可能被分配到栈上(此时记录无用但不影响正确性)，也可能被分配到寄存器中
    int iparas_size = 0;
    for(int i=8; i<subroutine->getFuncOfSubroutine()->getIArgs().size(); i++){
        auto ipara = subroutine->getFuncOfSubroutine()->getIArgs()[i];
        val2stack[static_cast<Value*>(ipara)] = new IRIA(static_cast<int>(RISCV::GPR::s0), iparas_size);
        iparas_size += align_8(ipara->getType()->getSize());
    }
    return iparas_size;
}

int AsmGen::allocateMemForFArgs(){
    int fparas_size = 0;
    for(int i=8; i<subroutine->getFuncOfSubroutine()->getFArgs().size(); i++){
        auto fpara = subroutine->getFuncOfSubroutine()->getFArgs()[i];
        val2stack[static_cast<Value*>(fpara)] = new IRIA(static_cast<int>(RISCV::GPR::s0), fparas_size+iargs_size);
        fparas_size += align_8(fpara->getType()->getSize());
    }
    return fparas_size;
}

int AsmGen::allocateMemForIPointer(){

    //& 为栈上的指针参数分配空间

 //   for(auto iter: ival2interval) {
 //       Value *val_iter = iter.first;
 //       Interval *interval_iter = iter.second;
 //        if(interval_iter){
 //       if(interval_iter->reg < 0) {
 //           auto arg = dynamic_cast<Argument*>(val_iter);
 //           if(arg && val2stack[static_cast<Value*>(arg)] != nullptr) 
 //               continue;
 //           int size_of_val_type = val_iter->getType()->getSize();
 //           total_size += ((size_of_val_type + 7) / 8) * 8;
 //           val2stack[val_iter] = new IRIA(static_cast<int>(RISCV::GPR::s0), -total_size);
 //       }
 //        }
 //   }

    int isize_record = 0;
    for(auto i:ival2interval){
        if(i.second && i.second->reg<=-1 && (!dynamic_cast<Argument*>(i.first) || val2stack[static_cast<Value*>(dynamic_cast<Argument*>(i.first))] == nullptr)){
            isize_record+=align_8(i.first->getType()->getSize());
            val2stack[i.first] = new IRIA(static_cast<int>(RISCV::GPR::s0), -(total_size+isize_record));
        }
    }
    return isize_record;

}

int AsmGen::allocateMemForFPointer(){

 //   for(auto iter: fval2interval) {
 //       Value *val_iter = iter.first;
 //       Interval *interval_iter = iter.second;
 //        if(interval_iter){
 //       if(interval_iter->reg < 0) {
 //           auto arg = dynamic_cast<Argument*>(val_iter);
 //           if(arg && val2stack[static_cast<Value*>(arg)] != nullptr) 
 //               continue;
 //           int size_of_val_type = val_iter->getType()->getSize();
 //           total_size += ((size_of_val_type + 7) / 8) * 8;
 //           val2stack[val_iter] = new IRIA(static_cast<int>(RISCV::GPR::s0), -total_size);
 //       }
 //        }
 //   }

    int fsize_record = 0;
    for(auto f:fval2interval){
        if(f.second && f.second->reg<=-1 && (!dynamic_cast<Argument*>(f.first) || val2stack[static_cast<Value*>(dynamic_cast<Argument*>(f.first))] == nullptr)){
            fsize_record+=align_8(f.first->getType()->getSize());
            val2stack[f.first] = new IRIA(static_cast<int>(RISCV::GPR::s0), -(total_size+fsize_record));
        }
    }
    return fsize_record;
}

int AsmGen::allocateMemForAlloca(){
    //& handle alloc inst
    int alloca_size = 0;
    for(auto &inst: subroutine->getFuncOfSubroutine()->getEntryBlock()->getInstructions()) {
        if(dynamic_cast<AllocaInst*>(inst)){
            alloca_size+=align_4(dynamic_cast<AllocaInst*>(inst)->getAllocaType()->getSize());
            val2stack[static_cast<Value*>(dynamic_cast<AllocaInst*>(inst))] = new IRIA(static_cast<int>(RISCV::GPR::s0), -(total_size+alloca_size));
        }
    }

    return alloca_size;
}

std::vector<std::pair<IRA*, IRIA*>> AsmGen::getCalleeSaveIRegs(){
   //if(!used_iregs_pair.second.empty()) {
   //    for(auto iter ) {
   //        cur_offset -= reg_size;
   //        to_save_iregs.push_back(std::make_pair(new IRA(*iter), new IRIA(static_cast<int>(RISCV::GPR::sp), cur_offset)));
   //    }
   //}
    std::vector<std::pair<IRA*, IRIA*>> iregs;
    int index = 0;
    for(auto ireg: used_iregs_pair.second){        
        iregs.push_back(std::make_pair(new IRA(ireg), new IRIA(static_cast<int>(RISCV::GPR::sp), save_offset+reg_size*(--index))));
    }
    save_offset+=reg_size*index;
    return iregs;
}

std::vector<std::pair<FRA*, IRIA*>> AsmGen::getCalleeSaveFRegs(){
   //if(!used_fregs_pair.second.empty()) {
   //    for(auto iter = used_fregs_pair.second.begin(); iter != used_fregs_pair.second.end(); iter++) {
   //        cur_offset -= reg_size;
   //        to_save_fregs.push_back(std::make_pair(new FRA(*iter), new IRIA(static_cast<int>(RISCV::GPR::sp), cur_offset)));
   //    }
   //}
    std::vector<std::pair<FRA*, IRIA*>> fregs;
    int index = 0;
    for(auto freg: used_fregs_pair.second){        
        fregs.push_back(std::make_pair(new FRA(freg), new IRIA(static_cast<int>(RISCV::GPR::sp), save_offset+reg_size*(--index))));
    }
    save_offset+=reg_size*index;
    return fregs;
}