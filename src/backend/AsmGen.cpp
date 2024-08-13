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

    auto callee_save_iregs = getCalleeSaveIRegs();
    auto callee_save_fregs = getCalleeSaveFRegs();

    if(!callee_save_iregs.empty())
        sequence->createCalleeSaveRegs(callee_save_iregs);
    if(!callee_save_fregs.empty())
        sequence->createCalleeSaveRegs(callee_save_fregs);
    sequence->createCalleeStackFrameInitialize(stack_size);
    //*************************被调用函数的栈帧初始化***************************


    iparas_pass_callee(subroutine->getFuncOfSubroutine());
    fparas_pass_callee(subroutine->getFuncOfSubroutine());

  
    if(!iparas_pass.empty() || !fparas_pass.empty())
        sequence->createCalleeParaPass(iparas_pass, fparas_pass);

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
            //处理尾指令
            //********************************装载临时寄存器到指令********************
                if(!inst->isAlloca() && !inst->isPhi()){
        

    std::set<int> record_iregs;
    std::set<int> load_iregs;
    std::set<int> record_fregs;
    std::set<int> load_fregs;

    std::vector<std::pair<IRA*, IRIA*>> resault_iregs;
    std::vector<std::pair<FRA*, IRIA*>> resault_fregs;
    

    loadITmpReg(inst, &load_iregs, &record_iregs);
    loadFTmpReg(inst, &load_fregs, &record_fregs);

    if(!inst->isVoid()) {
        if(inst->getType()->isFloatType()) {

            recordFReg(inst, &record_fregs);
        } else {

            recordIReg(inst, &record_iregs);
        }
    }

    for(auto ld_reg: load_iregs) {        
        resault_iregs.push_back(std::make_pair(new IRA(ld_reg), tmp_iregs_loc[ld_reg]));
    }

    for(auto ld_reg: load_fregs) {
        resault_fregs.push_back(std::make_pair(new FRA(ld_reg), tmp_fregs_loc[ld_reg]));
    }

    if(! resault_iregs.empty())
        sequence->createLoadTmpRegs(resault_iregs);
    
    if(! resault_fregs.empty())
        sequence->createLoadTmpRegs(resault_fregs);

    for(auto del_reg: record_iregs) {
        auto del_loc = tmp_iregs_loc[del_reg];
        free_locs_for_tmp_regs_saved.insert(del_loc);
        cur_tmp_iregs.erase(del_reg);
        tmp_iregs_loc.erase(del_reg);
    }

    for(auto del_reg: record_fregs) {
        auto del_loc = tmp_fregs_loc[del_reg];
        free_locs_for_tmp_regs_saved.insert(del_loc);
        cur_tmp_fregs.erase(del_reg);
        tmp_fregs_loc.erase(del_reg);
    }
}
            //********************************装载临时寄存器到指令********************        
            if(inst->isRet()) {
                inst->accept(*this);
            } else {
                //*****************************处理phi**********************
            if(dynamic_cast<CmpBrInst*>(inst)){
                auto c_inst = dynamic_cast<CmpBrInst*>(inst);
                initPhi();
                succ_bb = dynamic_cast<BasicBlock*>(c_inst->getOperand(2));
                fail_bb = dynamic_cast<BasicBlock*>(c_inst->getOperand(3));
                process();
                handleCmpbr(c_inst);
                if(succ_move_inst)  sequence->appendInst(succ_move_inst);
                if(succ_br_inst)    sequence->appendInst(succ_br_inst);
                if(fail_move_inst)  sequence->appendInst(fail_move_inst);
                if(fail_br_inst)    sequence->appendInst(fail_br_inst);
            }
            else if(dynamic_cast<FCmpBrInst*>(inst)){
                auto fc_inst = dynamic_cast<FCmpBrInst*>(inst);
                initPhi();
                succ_bb = dynamic_cast<BasicBlock*>(fc_inst->getOperand(2));
                fail_bb = dynamic_cast<BasicBlock*>(fc_inst->getOperand(3));
                process();
                handleFCmpbr(fc_inst);
                if(succ_move_inst)  sequence->appendInst(succ_move_inst);
                if(succ_br_inst)    sequence->appendInst(succ_br_inst);
                if(fail_move_inst)  sequence->appendInst(fail_move_inst);
                if(fail_br_inst)    sequence->appendInst(fail_br_inst);
            }
            else{
                auto br = dynamic_cast<BranchInst*>(inst); 
                initPhi();
                if(br->getNumOperands() == 1) {
                    succ_bb = dynamic_cast<BasicBlock*>(br->getOperand(0));
                } else {
                    succ_bb = dynamic_cast<BasicBlock*>(br->getOperand(1));
                    fail_bb = dynamic_cast<BasicBlock*>(br->getOperand(2));
                }
                process();
                handleBr(br);
                if(br->getNumOperands() == 1) {
                    if(succ_move_inst)  sequence->appendInst(succ_move_inst);
                    if(succ_br_inst)    sequence->appendInst(succ_br_inst);
        
                } else {
                    if(succ_br_inst)    sequence->appendInst(succ_br_inst);
                    if(fail_move_inst)  sequence->appendInst(fail_move_inst);
                    if(fail_br_inst)    sequence->appendInst(fail_br_inst);
                }
        
            }
               // phi_union(br_inst);
        
                //*****************************处理phi**********************
            }
        }

 
        //********************************************加载指令需要的临时寄存器***********************
            if(!inst->isAlloca() && !inst->isPhi()){
        

    std::set<int> record_iregs;
    std::set<int> load_iregs;
    std::set<int> record_fregs;
    std::set<int> load_fregs;

    std::vector<std::pair<IRA*, IRIA*>> resault_iregs;
    std::vector<std::pair<FRA*, IRIA*>> resault_fregs;
    

    loadITmpReg(inst, &load_iregs, &record_iregs);
    loadFTmpReg(inst, &load_fregs, &record_fregs);

    if(!inst->isVoid()) {
        if(inst->getType()->isFloatType()) {

            recordFReg(inst, &record_fregs);
        } else {

            recordIReg(inst, &record_iregs);
        }
    }

    for(auto ld_reg: load_iregs) {        
        resault_iregs.push_back(std::make_pair(new IRA(ld_reg), tmp_iregs_loc[ld_reg]));
    }

    for(auto ld_reg: load_fregs) {
        resault_fregs.push_back(std::make_pair(new FRA(ld_reg), tmp_fregs_loc[ld_reg]));
    }

    if(! resault_iregs.empty())
        sequence->createLoadTmpRegs(resault_iregs);
    
    if(! resault_fregs.empty())
        sequence->createLoadTmpRegs(resault_fregs);

    for(auto del_reg: record_iregs) {
        auto del_loc = tmp_iregs_loc[del_reg];
        free_locs_for_tmp_regs_saved.insert(del_loc);
        cur_tmp_iregs.erase(del_reg);
        tmp_iregs_loc.erase(del_reg);
    }

    for(auto del_reg: record_fregs) {
        auto del_loc = tmp_fregs_loc[del_reg];
        free_locs_for_tmp_regs_saved.insert(del_loc);
        cur_tmp_fregs.erase(del_reg);
        tmp_fregs_loc.erase(del_reg);
    }
}
        //********************************************加载指令需要的临时寄存器***********************

       if(inst->isCall()) {
            auto call_inst = dynamic_cast<CallInst*>(inst);
            
            //************************************调用前，保存caller寄存器************************
            caller_saved_ireg_locs.clear();
            caller_saved_freg_locs.clear();
            caller_save_iregs.clear();
            caller_save_fregs.clear();

            std::vector<std::pair<IRA*, IRIA*>> save_iregs;
            std::vector<std::pair<FRA*, IRIA*>> save_fregs;

            int call_inst_reg = -1;
     

            if(!call_inst->getType()->isVoidType()) {
                if(call_inst->getType()->isFloatType() && fval2interval.find(call_inst) != fval2interval.end()) {
                    call_inst_reg = fval2interval[call_inst]->reg;
                } else if(call_inst->getType()->isIntegerType() && ival2interval.find(call_inst) != ival2interval.end()) {
                    call_inst_reg = ival2interval[call_inst]->reg;
                }
            }

            for(auto ireg: used_iregs_pair.first) {
                if(!call_inst->getType()->isIntegerType() || ireg != call_inst_reg)
        
               if(::std::find(call_define_ireg_map[inst].begin(), call_define_ireg_map[inst].end(), ireg) != call_define_ireg_map[inst].end()){
                 //   ::std::cout<<"ireg："<<ireg<<::std::endl;
                   caller_save_iregs.push_back(ireg);
                }
                
            }

            for(auto freg: used_fregs_pair.first) {
                if(!call_inst->getType()->isFloatType() || freg != call_inst_reg)
              
               if(::std::find(call_define_freg_map[inst].begin(), call_define_freg_map[inst].end(), freg) != call_define_freg_map[inst].end()){
                 //   ::std::cout<<"freg："<<freg<<::std::endl;
                    caller_save_fregs.push_back(freg);
                }

            }
            for(auto reg: caller_save_iregs) {
                caller_saved_regs_stack_offset -= 8;
                caller_saved_ireg_locs[reg] = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset);
                save_iregs.push_back(std::make_pair(new IRA(reg), caller_saved_ireg_locs[reg]));
            }
            for(auto reg: caller_save_fregs) {
                caller_saved_regs_stack_offset -= 8;
                caller_saved_freg_locs[reg] = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset);
                save_fregs.push_back(std::make_pair(new FRA(reg), caller_saved_freg_locs[reg]));
            }

            if(! save_iregs.empty())
                sequence->createCallerSaveRegs(save_iregs);

            if(! save_fregs.empty())
                sequence->createCallerSaveRegs(save_fregs);



            //************************************调用前，保存caller寄存器************************

            fparas_pass_caller(call_inst);
            iparas_pass_caller(call_inst);

            if(!caller_iparas_pass.empty() || !caller_fparas_pass.empty())
                sequence->createCallerParaPass( caller_iparas_pass, caller_fparas_pass);

            int give_stack_space = caller_trans_args_stack_offset + cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset; 
            
            if(give_stack_space != 0) 
                sequence->createCalleeStackFrameExpand(give_stack_space);
            
            call_inst->accept(*this);
            
            if(give_stack_space != 0) 
                sequence->createCalleeStackFrameShrink(-give_stack_space);
            
       
            //************************************调用后，恢复caller寄存器******************************
            std::vector<std::pair<IRA*, IRIA*>> back_iregs;
            std::vector<std::pair<FRA*, IRIA*>> back_fregs;

            for(auto &[freg, loc]: caller_saved_freg_locs) {
                back_fregs.push_back(std::make_pair(new FRA(freg), loc));    
            }
            for(auto &[ireg, loc]: caller_saved_ireg_locs) {
                back_iregs.push_back(std::make_pair(new IRA(ireg), loc)); 
            }
            caller_saved_regs_stack_offset = 0;

            if(! back_fregs.empty())
                sequence->createCallerRestoreRegs(back_fregs);


            if(! back_iregs.empty())
                sequence->createCallerRestoreRegs(back_iregs);
         

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
                if(!dynamic_cast<Constant*>(opr) &&
                   !dynamic_cast<BasicBlock*>(opr) &&
                   !dynamic_cast<GlobalVariable*>(opr) &&
                   !dynamic_cast<AllocaInst*>(opr)) {
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

            }

     
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
                    
                }
            }

            for(auto opr: inst->getOperands()) {
                if(!dynamic_cast<Constant*>(opr) &&
                   !dynamic_cast<BasicBlock*>(opr) &&
                   !dynamic_cast<GlobalVariable*>(opr) &&
                   !dynamic_cast<AllocaInst*>(opr)) {
                
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
                   
                }
                }


            }

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
    //三个操作数，只能是取首地址
    if(inst->getNumOperands()==3){
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
    //2个操作数，求完整地址。有2种情况，一种是偏移量是寄存器，另一种是偏移量是立即数。
    //是寄存器的情况，直接求出完整地址
    //是立即数的情况，与访存指令合成为偏移化访存指令
   else if(inst->getNumOperands()==2){
        auto rd = dynamic_cast<GReg*>( getAllocaReg(inst));
        auto base = dynamic_cast<GReg*>( getAllocaReg(inst->getOperand(0)));


        if(dynamic_cast<ConstantInt*>(inst->getOperand(1))){
            int offset_const = dynamic_cast<ConstantInt*>(inst->getOperand(1))->getValue();
            assert(offset_const>=-512 && offset_const<511);
            int final_offset_const = offset_const*4;
            auto offset = new IConst(final_offset_const);
            sequence->createAdd(rd, base, offset);
        }
        else if(dynamic_cast<GReg*>( getAllocaReg(inst->getOperand(1)))){
            auto offset = dynamic_cast<GReg*>( getAllocaReg(inst->getOperand(1)));
            sequence->createSh2Add(rd, offset, base);
        }
        
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
    //增加地址是寄存器存储的
    else if(dynamic_cast<GReg*>( getAllocaReg(inst->getOperand(1)))){
        //通过存放立即数的寄存器类型或立即数类型判断是否是浮点数
        auto base = dynamic_cast<GReg*>( getAllocaReg(inst->getOperand(1)));
        auto offset = new IConst(0);
        if(dynamic_cast<FConst*>(getFRS1(inst))){
            sequence->createFsw(dynamic_cast<FConst*>(getFRS1(inst)), base, offset);
        }
        else if(dynamic_cast<FReg*>(getFRS1(inst))){
            sequence->createFsw(dynamic_cast<FReg*>(getFRS1(inst)), base, offset);
        }   
        else if(dynamic_cast<IConst*>(getIRS1(inst))){
            sequence->createSw(dynamic_cast<IConst*>(getIRS1(inst)), base, offset);
        }
        else if(dynamic_cast<GReg*>(getIRS1(inst))){
            sequence->createSw(dynamic_cast<GReg*>(getIRS1(inst)), base, offset);
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
    //源操作数是寄存器类型
    else if(dynamic_cast<GReg*>( getAllocaReg(inst->getOperand(0)))){
        auto base = dynamic_cast<GReg*>( getAllocaReg(inst->getOperand(0)));
        auto offset = new IConst(0);
        auto ird = getGRD(inst);
        auto frd = getFRD(inst);
        if(ird){
            sequence->createLw(ird, base, offset);
        }
        else if(frd){
            sequence->createFlw(frd, base, offset);
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

void AsmGen::visit(LoadImmInst &node){
    auto inst = &node;
    auto i_src = dynamic_cast<ConstantInt*>(inst->getOperand(0));
    auto f_src = dynamic_cast<ConstantFP*>(inst->getOperand(0));
    if(i_src){
        auto grd = getGRD(inst);
        auto i_const = new IConst(i_src->getValue());
        sequence->createLoadIImm(grd, i_const);
    }
    else if(dynamic_cast<ConstantFP*>(inst->getOperand(0))){
        auto frd = getFRD(inst);
        auto f_const = new FConst(f_src->getValue());
        sequence->createLoadFImm(frd, f_const);
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








void AsmGen::iparas_pass_callee(Function *func) {
    iparas_pass.clear();
    ipara_sh.clear();
    auto iargs_vector = func->getIArgs();
    int first_iargs_num = iargs_vector.size() > 8 ?   8 : iargs_vector.size();
    std::map<int, bool> flags;
    int iter=0;
    while(iter<first_iargs_num){
        flags[iter] = false;
        iter++;
    }
    while(true) {
        int i = 0;
        for(auto flag: flags) {
            if(!flag.second){
                break;
            } 
            i++;
        }
        if(i==flags.size())
            break;
        while(true) {
            if(ival2interval.find(iargs_vector[i]) != ival2interval.end()) {
                int target_reg_id = ival2interval[iargs_vector[i]]->reg;
                if(flags[i]) {
                    addIPara(&iparas_pass, &flags);
                    ipara_sh.clear();
                    break;
                } else if(target_reg_id < 0) {
                    auto iria = new IRIA(static_cast<int>(val2stack[iargs_vector[i]]->getReg()), val2stack[iargs_vector[i]]->getOffset());
                    auto ira = new IRA(arg_reg_base + i);
                    iparas_pass.push_back(std::make_pair(iria, ira));
                    flags[i] = true;
                    if(!ipara_sh.empty()) {
                        addIPara(&iparas_pass, &flags);
                        ipara_sh.clear();
                    } 
                    break;
                } else if(target_reg_id - arg_reg_base == i) {
                    flags[i] = true;
                    break;
                } else if(target_reg_id - arg_reg_base >= first_iargs_num || target_reg_id - arg_reg_base < 0) {
                    auto ira = new IRA(target_reg_id);
                    auto ira_ = new IRA(arg_reg_base + i);
                    iparas_pass.push_back(std::make_pair(ira, ira_));
                    flags[i] = true;
                    if(!ipara_sh.empty()) {
                        addIPara(&iparas_pass, &flags);
                        ipara_sh.clear();
                    }
                    break;
                } else {
                    ipara_sh.push_back({iargs_vector[i], i});
                    i = target_reg_id - arg_reg_base;
                    if(iargs_vector[i] == ipara_sh.begin()->first) {
                      
                        auto ira1 = new IRA(static_cast<int>(RISCV::GPR::s1));
                        auto ira1_ = new IRA(arg_reg_base + ipara_sh.rbegin()->second);
                        iparas_pass.push_back(std::make_pair(ira1, ira1_));
                        addILoopPara(&iparas_pass, &flags);
                        auto ira2 = new IRA(ival2interval[ipara_sh.rbegin()->first]->reg);
                        auto ira2_ = new IRA(static_cast<int>(RISCV::GPR::s1));
                        iparas_pass.push_back(std::make_pair(ira2, ira2_));
                        flags[ipara_sh.rbegin()->second] = true;
                        ipara_sh.clear();
                        break;
                    }
                }
            } else {
                flags[i] = true;
                if(!ipara_sh.empty()) {
                    addIPara(&iparas_pass, &flags);
                    ipara_sh.clear();
                }
                break;
            }
        }
    }


    for(int i = first_iargs_num; i < iargs_vector.size(); i++) {
        auto ipara_iter = ival2interval.find(iargs_vector[i]);
        if(ipara_iter != ival2interval.end() && ipara_iter->second->reg>-1) {
            auto ira = new IRA(ipara_iter->second->reg);
            auto iria = new IRIA(static_cast<int>(RISCV::GPR::s0), reg_size * (i - first_iargs_num));
            iparas_pass.push_back(std::make_pair(ira, iria));
        } 
    }

    return ;
}


void AsmGen::fparas_pass_callee(Function *func) {
    fparas_pass.clear();
    fpara_sh.clear();
    int iargs_num = func->getIArgs().size();
    iargs_num = iargs_num > 8 ? (iargs_num-8) : 0;

    auto fargs_vector = func->getFArgs();
    int first_fargs_num = fargs_vector.size() > 8 ?   8 : fargs_vector.size();

    std::map<int, bool> flags;

    int iter=0;
    while(iter<first_fargs_num){
        flags[iter] = false;
        iter++;
    }



    while(true) {
        int i = 0;
        for(auto flag: flags) {
            if(!flag.second){
                break;
            } 
            i++;
        }
        if(i==flags.size())
            break;

        while(true) {
            if(fval2interval.find(fargs_vector[i]) != fval2interval.end()) {
                int target_reg_id = fval2interval[fargs_vector[i]]->reg;
                if(flags[i]) {
                    addFPara(&fparas_pass, &flags);

                    fpara_sh.clear();
                    break;
                } else if(target_reg_id < 0) {
                    auto iria = new IRIA(static_cast<int>( val2stack[fargs_vector[i]]->getReg()), val2stack[fargs_vector[i]]->getOffset());\
                    auto fra = new FRA(arg_reg_base + i);
                    fparas_pass.push_back(std::make_pair(iria, fra));
                    flags[i] = true;
                    if(!fpara_sh.empty()) {
                        addFPara(&fparas_pass, &flags);
                        fpara_sh.clear();
                    } 
                    break;
                } else if(target_reg_id - arg_reg_base == i) {
                    flags[i] = true;
                    break;
                } else if(target_reg_id - arg_reg_base >= first_fargs_num || target_reg_id - arg_reg_base < 0) {
                    auto fra1 = new FRA(target_reg_id);
                    auto fra1_ = new FRA(arg_reg_base + i);
                    fparas_pass.push_back(std::make_pair(fra1, fra1_));
                    flags[i] = true;
                    int target = i;
                    if(!fpara_sh.empty()) {
                        addFPara(&fparas_pass, &flags);
                        fpara_sh.clear();
                    }
                    break;
                } else {
                    fpara_sh.push_back({fargs_vector[i], i});
                    i = target_reg_id - arg_reg_base;
                    if(fargs_vector[i] == fpara_sh.begin()->first) {
                        //& found loop    
                        auto fra1 = new FRA(static_cast<int>(RISCV::FPR::fs1));
                        auto fra1_ = new FRA(arg_reg_base + fpara_sh.rbegin()->second);
                        fparas_pass.push_back(std::make_pair(fra1, fra1_));
                        addFLoopPara(&fparas_pass, &flags);
                        auto fra2 = new FRA(fval2interval[fpara_sh.rbegin()->first]->reg);
                        auto fra2_ = new FRA(static_cast<int>(RISCV::FPR::fs1));
                        fparas_pass.push_back(std::make_pair(fra2, fra2_));
                        flags[fpara_sh.rbegin()->second] = true;
                        fpara_sh.clear();
                        break;
                    }
                }
            } else {
                flags[i] = true;
                if(!fpara_sh.empty()) {
                    addFPara(&fparas_pass, &flags);
                    fpara_sh.clear();
                }
                break;
            }
        }
    }

 
    for(int i = first_fargs_num; i < fargs_vector.size(); i++) {
        auto fpara_iter = fval2interval.find(fargs_vector[i]);
        if( fpara_iter!= fval2interval.end() && fpara_iter->second->reg>-1) {

            auto fra = new FRA(fpara_iter->second->reg);
            auto iria = new IRIA(static_cast<int>(RISCV::GPR::s0), reg_size * (iargs_num + i - first_fargs_num));
            fparas_pass.push_back(std::make_pair(fra, iria));

        } 
    }
    return ;
}






void AsmGen::loadITmpReg(Instruction* inst, std::set<int>* load_iregs, std::set<int>* record_iregs){
    for(auto opr: inst->getOperands()) {
        if(!dynamic_cast<Constant*>(opr) &&
            !dynamic_cast<BasicBlock*>(opr) &&
            !dynamic_cast<GlobalVariable*>(opr) &&
            !dynamic_cast<AllocaInst*>(opr) &&
            !dynamic_cast<Function*>(opr)) {
           if(!opr->getType()->isFloatType()) {
            int opr_reg = ival2interval[opr]->reg;
            if(opr_reg >= 0) {
                if(cur_tmp_iregs.find(opr_reg) != cur_tmp_iregs.end()) {
                    load_iregs->insert(opr_reg);
                    record_iregs->insert(opr_reg);
                }
            } 
        }
        }

    }
}

void AsmGen::loadFTmpReg(Instruction* inst, std::set<int>* load_fregs, std::set<int>* record_fregs){
    for(auto opr: inst->getOperands()) {
        if(!dynamic_cast<Constant*>(opr) &&
            !dynamic_cast<BasicBlock*>(opr) &&
            !dynamic_cast<GlobalVariable*>(opr) &&
            !dynamic_cast<AllocaInst*>(opr) &&
            !dynamic_cast<Function*>(opr)) {
           if(opr->getType()->isFloatType()) {
            int opr_reg = fval2interval[opr]->reg;
            if(opr_reg >= 0) {
                if(cur_tmp_fregs.find(opr_reg) != cur_tmp_fregs.end()) {
                    load_fregs->insert(opr_reg);
                    record_fregs->insert(opr_reg);
                }
            } 
        }
        }

    }
}



void AsmGen::fparas_pass_caller(CallInst *call) {
    caller_fparas_pass.clear();
    caller_fpara_sh.clear();

    std::vector<Value*> fargs;
    for(auto arg: call->getOperands()) {
        if(!dynamic_cast<Function*>(arg) && arg->getType()->isFloatType()){
            fargs.push_back(arg);
        }   
    }

    int num_of_fargs = fargs.size() > 8 ? 8 : fargs.size();

    std::map<int, bool> flags;

    std::map<int, std::set<int>> freg_fpara_map;

    std::list<std::pair<Value*, int>> caller_fpara_sh;



    initFPara(&fargs, &flags, num_of_fargs, &freg_fpara_map);




    if(fargs.size()>8)
        addFStackPara(fargs);

    processFPara(flags, freg_fpara_map, fargs);
    return ;
}




void AsmGen::iparas_pass_caller(CallInst *call) {
    caller_iparas_pass.clear();
    caller_ipara_sh.clear();

    std::vector<Value*> iargs;
    for(auto arg: call->getOperands()) {
        if(!dynamic_cast<Function*>(arg) && !arg->getType()->isFloatType()){
            iargs.push_back(arg);
        }
    }

    int num_of_iargs = iargs.size() > 8 ? 8 : iargs.size(); 

    std::map<int, bool> flags;

    std::map<int, std::set<int>> ireg_ipara_map;

    initIPara(&iargs, &flags, num_of_iargs, &ireg_ipara_map);


    if(iargs.size()>8)
        addIStackPara(iargs);

    processIPara(flags, ireg_ipara_map, iargs);
    return ;
}




void AsmGen::mov_value(std::vector<std::pair<AddressMode*, AddressMode*>>* address_mov, std::vector<AddressMode*>&srcs, std::vector<AddressMode*>&dsts, bool is_float) {
 
    std::map<int, bool> flags;
    std::list<int> address_mode;
    std::map<AddressMode*, std::set<int>> dst_src_map;


    for(int i = 0; i < srcs.size(); i++) {
        if(dst_src_map.find(srcs[i]) != dst_src_map.end())  dst_src_map[srcs[i]].insert(i);   
        else    dst_src_map.insert({srcs[i], {i}});  
        flags[i] = false;
    }

    while(true) {
        int i = 0;

        for(auto flag: flags) {
            if(flag.second) i++; 
            else break;
        }


        if(i == flags.size())
            break;

        while(true) {
            if(dst_src_map.find(dsts[i]) != dst_src_map.end()) {
                if(dst_src_map[dsts[i]].find(i) != dst_src_map[dsts[i]].end()) {
                    if(dst_src_map[dsts[i]].size() == 1) {
                        flags[i] = true;
                        dst_src_map[dsts[i]].erase(i);
                        dst_src_map.erase(dsts[i]);
                    } else {
                        for(auto tmp_dstno: dst_src_map[dsts[i]]) {
                            if(tmp_dstno != i) {
                                i = tmp_dstno;
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                } else {
                    address_mode.push_back(i);
                    i = *dst_src_map[dsts[i]].begin();
                    if(i == *address_mode.begin()) {
                        
                        bool is_single_circle = true;
                        for(auto iter = address_mode.begin(); iter != address_mode.end(); iter++) {
                            if(dst_src_map[dsts[*iter]].size() > 1) {
                                int next_dstno;
                                if(*iter == *address_mode.rbegin()) {
                                    next_dstno = *address_mode.begin();
                                } else {
                                    next_dstno = *(++iter);
                                    iter--;
                                }
                                for(auto tmp_dstno: dst_src_map[dsts[*iter]]) {
                                    if(tmp_dstno != next_dstno) {
                                        i = tmp_dstno;
                                        break;
                                    } else {
                                        continue;
                                    }
                                }
                                address_mode.clear();
                                is_single_circle = false;
                                break;
                            }
                        }
                        if(is_single_circle) {
                            if(!is_float)
                                address_mov->push_back(std::make_pair(new IRA(static_cast<int>(RISCV::GPR::s1)) , srcs[*address_mode.rbegin()]));
                            else    
                                address_mov->push_back(std::make_pair(new FRA(static_cast<int>(RISCV::FPR::fs1)) , srcs[*address_mode.rbegin()]));
                            for(auto riter = address_mode.rbegin(); riter != address_mode.rend(); riter++) {
                                if(*riter == *address_mode.rbegin())
                                    continue;
                                address_mov->push_back(std::make_pair(dsts[*riter] , srcs[*riter]));
                                flags[*riter] = true;
                                dst_src_map[srcs[*riter]].erase(*riter);
                                if(dst_src_map[srcs[*riter]].empty()) {
                                    dst_src_map.erase(srcs[*riter]);
                                }  
                            }
                            if(!is_float)
                                address_mov->push_back(std::make_pair(dsts[*address_mode.rbegin()] , new IRA(static_cast<int>(RISCV::GPR::s1))));  
                            else
                                address_mov->push_back(std::make_pair(dsts[*address_mode.rbegin()] , new FRA(static_cast<int>(RISCV::FPR::fs1))));
                            int tmp_no = *address_mode.rbegin();
                            flags[tmp_no] = true;
                            dst_src_map[srcs[tmp_no]].erase(tmp_no);
                            if(dst_src_map[srcs[tmp_no]].empty()) {
                                dst_src_map.erase(srcs[tmp_no]);
                            }
                            address_mode.clear();
                            break;
                        }
                    }
                }
            } else {
                address_mov->push_back(std::make_pair(dsts[i], srcs[i]));
                flags[i] = true;
                dst_src_map[srcs[i]].erase(i);
                if(dst_src_map[srcs[i]].empty()) {
                    dst_src_map.erase(srcs[i]);
                }
                if(!address_mode.empty()) {
                    for(auto riter = address_mode.rbegin(); riter != address_mode.rend(); riter++) {
                        if(dst_src_map.find(dsts[*riter]) != dst_src_map.end()) 
                            break;
                        address_mov->push_back(std::make_pair(dsts[*riter], srcs[*riter]));
                        flags[*riter] = true;
                        dst_src_map[srcs[*riter]].erase(*riter);
                        if(dst_src_map[srcs[*riter]].empty()) {
                            dst_src_map.erase(srcs[*riter]);
                        }   
                    }
                    address_mode.clear();
                }
                break;
            }
        }
    }

    return ;
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







int AsmGen::setCallerAndCalleeRegs(){
    used_iregs_pair.first.clear();
    used_iregs_pair.second.clear();
    used_fregs_pair.first.clear();
    used_fregs_pair.second.clear();
    

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

    std::vector<std::pair<IRA*, IRIA*>> iregs;
    int index = 0;
    for(auto ireg: used_iregs_pair.second){        
        iregs.push_back(std::make_pair(new IRA(ireg), new IRIA(static_cast<int>(RISCV::GPR::sp), save_offset+reg_size*(--index))));
    }
    save_offset+=reg_size*index;
    return iregs;
}

std::vector<std::pair<FRA*, IRIA*>> AsmGen::getCalleeSaveFRegs(){
 
    std::vector<std::pair<FRA*, IRIA*>> fregs;
    int index = 0;
    for(auto freg: used_fregs_pair.second){        
        fregs.push_back(std::make_pair(new FRA(freg), new IRIA(static_cast<int>(RISCV::GPR::sp), save_offset+reg_size*(--index))));
    }
    save_offset+=reg_size*index;
    return fregs;
}

void AsmGen::addIPara(::std::vector<std::pair<AddressMode*, AddressMode*>>* iparas_pass, ::std::map<int, bool>* flags){
    for(auto i = ipara_sh.rbegin(); i!=ipara_sh.rend(); i++){
        auto ira = new IRA(ival2interval[i->first]->reg);
        auto ira_ = new IRA(i->second+arg_reg_base);
        (*iparas_pass).push_back(::std::make_pair(ira, ira_));
        (*flags)[i->second] = true;
    }
    return;
}

void AsmGen::addFPara(::std::vector<std::pair<AddressMode*, AddressMode*>>* fparas_pass, ::std::map<int, bool>* flags){
    for(auto f = fpara_sh.rbegin(); f!=fpara_sh.rend(); f++){
        auto fra = new FRA(fval2interval[f->first]->reg);
        auto fra_ = new FRA(f->second+arg_reg_base);
        (*fparas_pass).push_back(::std::make_pair(fra, fra_));
        (*flags)[f->second] = true;
    }
    return;
}

void AsmGen::addILoopPara(::std::vector<std::pair<AddressMode*, AddressMode*>>* iparas_pass, ::std::map<int, bool>* flags){
    for(auto i = ipara_sh.rbegin(); i!=ipara_sh.rend(); i++){
        if(i->first != ipara_sh.rbegin()->first){
        auto ira = new IRA(ival2interval[i->first]->reg);
        auto ira_ = new IRA(i->second+arg_reg_base);
        (*iparas_pass).push_back(::std::make_pair(ira, ira_));
        (*flags)[i->second] = true;
        }

    }
    return;
}

void AsmGen::addFLoopPara(::std::vector<std::pair<AddressMode*, AddressMode*>>* fparas_pass, ::std::map<int, bool>* flags){
    for(auto f = fpara_sh.rbegin(); f!=fpara_sh.rend(); f++){
        if(f->first != fpara_sh.rbegin()->first){
        auto fra = new FRA(fval2interval[f->first]->reg);
        auto fra_ = new FRA(f->second+arg_reg_base);
        (*fparas_pass).push_back(::std::make_pair(fra, fra_));
        (*flags)[f->second] = true;
        }

    }
    return;
}

void AsmGen::initIPara(std::vector<Value*>* ip_s, std::map<int, bool>* flags, int num, std::map<int, std::set<int>>* ireg_ipara_map){
    for(int i=0; i<num; i++){
        if(ival2interval.find((*ip_s)[i])!=ival2interval.end() && ival2interval[(*ip_s)[i]]->reg>-1){
            if(ireg_ipara_map->find(ival2interval[(*ip_s)[i]]->reg) != ireg_ipara_map->end()){
                (*ireg_ipara_map)[ival2interval[(*ip_s)[i]]->reg].insert(i);
            }
            else{
                ireg_ipara_map->insert({ival2interval[(*ip_s)[i]]->reg, {i}});
            }
        }
        (*flags)[i] = false;
    }
    return;
}

void AsmGen::initFPara(std::vector<Value*>* fp_s, std::map<int, bool>* flags, int num,std::map<int, std::set<int>>* freg_fpara_map){
    for(int i=0; i<num; i++){
        if(fval2interval.find((*fp_s)[i])!=fval2interval.end() && fval2interval[(*fp_s)[i]]->reg>-1){
            if(freg_fpara_map->find(fval2interval[(*fp_s)[i]]->reg) != freg_fpara_map->end()){
                (*freg_fpara_map)[fval2interval[(*fp_s)[i]]->reg].insert(i);
            }
            else{
                freg_fpara_map->insert({fval2interval[(*fp_s)[i]]->reg, {i}});
            }
        }
        (*flags)[i] = false;
    }
    return;
}

void AsmGen::addIStackPara(std::vector<Value*> iargs){
    
    for(int i = iargs.size() - 1; i >=8; i--) {
        caller_trans_args_stack_offset -= reg_size;
        if(ival2interval.find(iargs[i]) != ival2interval.end()) {
            if(ival2interval[iargs[i]]->reg >= 0) {
                auto iria = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset + caller_trans_args_stack_offset);
                auto ira = new IRA(ival2interval[iargs[i]]->reg);
                caller_iparas_pass.push_back(std::make_pair(iria, ira));
            } else {
                auto iria = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset + caller_trans_args_stack_offset);
                auto ira = val2stack[iargs[i]];
                caller_iparas_pass.push_back(std::make_pair(iria, ira));
            }
        } else {
            if(dynamic_cast<ConstantInt*>(iargs[i])) {
                auto iria = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset + caller_trans_args_stack_offset);
                auto ira = new IConstPool(dynamic_cast<ConstantInt*>(iargs[i])->getValue());
                caller_iparas_pass.push_back(std::make_pair(iria, ira));
            } else {
             
                ::std::cerr<<"栈上参数传递错误"<<::std::endl;

            }
        }
    }
    return;
}

void AsmGen::addFStackPara(std::vector<Value*> fargs){


    
    for(int i = fargs.size() - 1; i >=8; i--) {
        caller_trans_args_stack_offset -= reg_size;
        if(fval2interval.find(fargs[i]) != fval2interval.end()) {
            if(fval2interval[fargs[i]]->reg >= 0) {
                auto iria = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset + caller_trans_args_stack_offset);
                auto fra = new FRA(fval2interval[fargs[i]]->reg);
                caller_fparas_pass.push_back(std::make_pair(iria, fra));
            } else {
                auto iria = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset + caller_trans_args_stack_offset);
                auto fra = val2stack[fargs[i]];
                caller_fparas_pass.push_back(std::make_pair(iria, fra));
            }
        } else {
            if(dynamic_cast<ConstantInt*>(fargs[i])) {
                auto iria = new IRIA(static_cast<int>(RISCV::GPR::sp), cur_tmp_reg_saved_stack_offset + caller_saved_regs_stack_offset + caller_trans_args_stack_offset);
                auto fra = new FConstPool(dynamic_cast<ConstantInt*>(fargs[i])->getValue());
                caller_fparas_pass.push_back(std::make_pair(iria, fra));
            } else {
              
                ::std::cerr<<"栈上参数传递错误"<<::std::endl;

            }
        }
    }
    return;


}

void AsmGen::processIPara(std::map<int, bool> flags, std::map<int, std::set<int>> ireg_ipara_map, std::vector<Value*> iargs){
    while(true) {
        int i = 0;
       
        for(auto flag: flags) {
            if(!flag.second) 
                break;
            i++;
        }
       
        if(i == flags.size()) 
            break;
        while(true) {
            if(ireg_ipara_map.find(i + arg_reg_base) != ireg_ipara_map.end()) {
                if(ireg_ipara_map[i+arg_reg_base].find(i) != ireg_ipara_map[i+arg_reg_base].end()) {
                    if(ireg_ipara_map[i+arg_reg_base].size() == 1) {
                        flags[i] = true;
                        ireg_ipara_map[i+arg_reg_base].erase(i);
                        ireg_ipara_map.erase(i+arg_reg_base);
                    } else {
                        for(auto tmp_argno: ireg_ipara_map[i+arg_reg_base]) {
                            if(tmp_argno != i) {
                                i = tmp_argno;
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                } else {
                    caller_ipara_sh.push_back({iargs[i], i});
                    i = *ireg_ipara_map[i+arg_reg_base].begin();
                    if(i == caller_ipara_sh.begin()->second) {
                     
                        bool is_single_circle = true; 
                        for(auto iter = caller_ipara_sh.begin(); iter != caller_ipara_sh.end(); iter++) {
                            if(ireg_ipara_map[iter->second + arg_reg_base].size() > 1) {
                                int next_argno;
                                if(iter->first == caller_ipara_sh.rbegin()->first) {
                                    next_argno = caller_ipara_sh.begin()->second;
                                } else {
                                    next_argno = (++iter)->second;
                                    iter--;
                                }
                                for(auto tmp_argno: ireg_ipara_map[iter->second + arg_reg_base]) {
                                    if(tmp_argno != next_argno) {
                                        i = tmp_argno;
                                        break;
                                    } else {
                                        continue;
                                    }
                                }
                                caller_ipara_sh.clear();
                                is_single_circle = false;
                                break;
                            }
                        }
                        if(is_single_circle) {
                            auto ira = new IRA(static_cast<int>(RISCV::GPR::s1));
                            auto ira_ = new IRA(ival2interval[caller_ipara_sh.rbegin()->first]->reg);
                            caller_iparas_pass.push_back(std::make_pair(ira, ira_));
                            for(auto riter= caller_ipara_sh.rbegin(); riter != caller_ipara_sh.rend(); riter++) {
                                if(riter->first == caller_ipara_sh.rbegin()->first)
                                    continue;
                                int arg_no = riter->second;
                                int src_reg_id = ival2interval[riter->first]->reg;
                                auto ira1 = new IRA(arg_no+arg_reg_base);
                                auto ira1_ = new IRA(src_reg_id);
                                caller_iparas_pass.push_back(std::make_pair(ira1, ira1_));
                                flags[arg_no] = true;        
                                ireg_ipara_map[src_reg_id].erase(arg_no);
                                if(ireg_ipara_map[src_reg_id].empty())
                                    ireg_ipara_map.erase(src_reg_id);
                            }
                            auto ira2 = new IRA(caller_ipara_sh.rbegin()->second + arg_reg_base);
                            auto ira2_ = new IRA(static_cast<int>(RISCV::GPR::s1));
                            caller_iparas_pass.push_back(std::make_pair(ira2, ira2_));
                            int arg_no = caller_ipara_sh.rbegin()->second;
                            int src_reg_id = ival2interval[caller_ipara_sh.rbegin()->first]->reg;
                            flags[arg_no] = true; 
                            ireg_ipara_map[src_reg_id].erase(arg_no);
                            if(ireg_ipara_map[src_reg_id].empty())
                                ireg_ipara_map.erase(src_reg_id);
                            caller_ipara_sh.clear();
                            break;
                        } 
                    }
                }
            } else {
                if(ival2interval.find(iargs[i]) == ival2interval.end()) {
                    auto ira = new IRA(i+arg_reg_base);
                    auto ira_ = new IConstPool(dynamic_cast<ConstantInt*>(iargs[i])->getValue());
                    caller_iparas_pass.push_back(std::make_pair(ira, ira_));
                    flags[i] = true;
                    break;
                } else {
                    int src_reg_id = ival2interval[iargs[i]]->reg;
                    if(src_reg_id < 0) {
                        auto ira = new IRA(i+arg_reg_base);
                        auto iria = val2stack[iargs[i]];
                        caller_iparas_pass.push_back(std::make_pair(ira, iria));
                        flags[i] = true;
                        break;
                    } else {
                        auto ira = new IRA(i+arg_reg_base);
                        auto ira_ = new IRA(src_reg_id);
                        caller_iparas_pass.push_back(std::make_pair(ira, ira_));
                        flags[i] = true;
                        ireg_ipara_map[src_reg_id].erase(i);
                        if(ireg_ipara_map[src_reg_id].empty())
                            ireg_ipara_map.erase(src_reg_id);
                        if(!caller_ipara_sh.empty()) {
                            for(auto riter = caller_ipara_sh.rbegin(); riter != caller_ipara_sh.rend(); riter++) {
                                auto iarg = riter->first;
                                auto iargno = riter->second;
                                if(ival2interval.find(iargs[iargno]) == ival2interval.end()) {
                                    auto ira1 = new IRA(iargno+arg_reg_base);
                                    auto iria1 = new IConstPool(dynamic_cast<ConstantInt*>(iarg)->getValue());
                                    caller_iparas_pass.push_back(std::make_pair(ira1, iria1));
                                    flags[iargno] = true;
                                    break;
                                } else {
                                    int src_reg_id = ival2interval[iarg]->reg;
                                    if(src_reg_id < 0) {
                                        auto ira2 = new IRA(iargno+arg_reg_base);
                                        auto iria2 = val2stack[iarg];
                                        caller_iparas_pass.push_back(std::make_pair(ira2, iria2));
                                        flags[iargno] = true;
                                        break;
                                    } else {
                                        if(ireg_ipara_map.find(iargno + arg_reg_base) != ireg_ipara_map.end()) 
                                            break;
                                        auto ira = new IRA(iargno+arg_reg_base);
                                        auto ira_ = new IRA(src_reg_id);
                                        caller_iparas_pass.push_back(std::make_pair(ira, ira_));
                                        flags[iargno] = true;
                                        ireg_ipara_map[src_reg_id].erase(iargno);
                                        if(ireg_ipara_map[src_reg_id].empty())
                                            ireg_ipara_map.erase(src_reg_id);
                                    }
                                }
                            }
                            caller_ipara_sh.clear();
                        }
                        break;
                    }
                }
            }
        }
    }
    return;
}

void AsmGen::processFPara(std::map<int, bool> flags, std::map<int, std::set<int>> freg_fpara_map, std::vector<Value*> fargs){
    while(true) {
        int i = 0;
        //& 寻找一个尚未移动的参数
        for(auto flag: flags) {
            if(!flag.second) 
                break;
            i++;
        }

        if(i == flags.size()) 
            break;
        while(true) {
            if(freg_fpara_map.find(i + arg_reg_base) != freg_fpara_map.end()) {
                if(freg_fpara_map[i+arg_reg_base].find(i) != freg_fpara_map[i+arg_reg_base].end()) {
                    if(freg_fpara_map[i+arg_reg_base].size() == 1) {
                        flags[i] = true;
                        freg_fpara_map[i+arg_reg_base].erase(i);
                        freg_fpara_map.erase(i+arg_reg_base);
                    } else {
                        for(auto tmp_argno: freg_fpara_map[i+arg_reg_base]) {
                            if(tmp_argno != i) {
                                i = tmp_argno;
                                break;
                            } else {
                                continue;
                            }
                        }
                    }
                } else {
                    caller_fpara_sh.push_back({fargs[i], i});
                    i = *freg_fpara_map[i+arg_reg_base].begin();
                    if(i == caller_fpara_sh.begin()->second) {
     
                        bool is_single_circle = true; 
                        for(auto iter = caller_fpara_sh.begin(); iter != caller_fpara_sh.end(); iter++) {
                            if(freg_fpara_map[iter->second + arg_reg_base].size() > 1) {
                                int next_argno;
                                if(iter->first == caller_fpara_sh.rbegin()->first) {
                                    next_argno = caller_fpara_sh.begin()->second;
                                } else {
                                    next_argno = (++iter)->second;
                                    iter--;
                                }
                                for(auto tmp_argno: freg_fpara_map[iter->second + arg_reg_base]) {
                                    if(tmp_argno != next_argno) {
                                        i = tmp_argno;
                                        break;
                                    } else {
                                        continue;
                                    }
                                }
                                caller_fpara_sh.clear();
                                is_single_circle = false;
                                break;
                            }
                        }
                        if(is_single_circle) {
                            auto fra = new FRA(static_cast<int>(RISCV::FPR::fs1));
                            auto fra_ = new FRA(fval2interval[caller_fpara_sh.rbegin()->first]->reg);
                            caller_fparas_pass.push_back(std::make_pair(fra, fra_));
                            for(auto riter= caller_fpara_sh.rbegin(); riter != caller_fpara_sh.rend(); riter++) {
                                if(riter->first == caller_fpara_sh.rbegin()->first)
                                    continue;
                                int arg_no = riter->second;
                                int src_reg_id = fval2interval[riter->first]->reg;
                                auto fra1 = new FRA(arg_no+arg_reg_base);
                                auto fra1_ = new FRA(src_reg_id);
                                caller_fparas_pass.push_back(std::make_pair(fra1, fra1_));
                                flags[arg_no] = true;        
                                freg_fpara_map[src_reg_id].erase(arg_no);
                                if(freg_fpara_map[src_reg_id].empty())
                                    freg_fpara_map.erase(src_reg_id);
                            }
                            auto fra2 = new FRA(caller_fpara_sh.rbegin()->second + arg_reg_base);
                            auto fra2_ = new FRA(static_cast<int>(RISCV::FPR::fs1));
                            caller_fparas_pass.push_back(std::make_pair(fra2, fra2_));
                            int arg_no = caller_fpara_sh.rbegin()->second;
                            int src_reg_id = fval2interval[caller_fpara_sh.rbegin()->first]->reg;
                            flags[arg_no] = true; 
                            freg_fpara_map[src_reg_id].erase(arg_no);
                            if(freg_fpara_map[src_reg_id].empty())
                                freg_fpara_map.erase(src_reg_id);
                            caller_fpara_sh.clear();
                            break;
                        } 
                    }
                }
            } else {
                if(fval2interval.find(fargs[i]) == fval2interval.end()) {
                    auto fra = new FRA(i + arg_reg_base);
                    auto iria = new FConstPool(dynamic_cast<ConstantFP*>(fargs[i])->getValue());
                    caller_fparas_pass.push_back(std::make_pair(fra, iria));
                    flags[i] = true;
                    break;
                } else {
                    int src_reg_id = fval2interval[fargs[i]]->reg;
                    if(src_reg_id < 0) {
                        auto fra1 = new FRA(i + arg_reg_base);
                        auto iria1 =  val2stack[fargs[i]];
                        caller_fparas_pass.push_back(std::make_pair(fra1, iria1));
                        flags[i] = true;
                        break;
                    } else {
                        auto fra1 = new FRA(i + arg_reg_base);
                        auto fra1_ = new FRA(src_reg_id);
                        caller_fparas_pass.push_back(std::make_pair(fra1, fra1_));
                        flags[i] = true;
                        freg_fpara_map[src_reg_id].erase(i);
                        if(freg_fpara_map[src_reg_id].empty())
                            freg_fpara_map.erase(src_reg_id);
                        if(!caller_fpara_sh.empty()) {
                            for(auto riter = caller_fpara_sh.rbegin(); riter != caller_fpara_sh.rend(); riter++) {
                                auto farg = riter->first;
                                auto fargno = riter->second;
                                if(fval2interval.find(fargs[fargno]) == fval2interval.end()) {
                                    auto const_fp =  dynamic_cast<ConstantFP*>(farg);
                                    auto fra2 = new FRA(fargno + arg_reg_base);
                                    auto iria2_ =  new FConstPool(const_fp->getValue());
                                    caller_fparas_pass.push_back(std::make_pair(fra2, iria2_));
                                    flags[fargno] = true;
                                    break;
                                } else {
                                    int src_reg_id = fval2interval[farg]->reg;
                                    if(src_reg_id < 0) {
                                        auto fra3 = new FRA(fargno + arg_reg_base);
                                        auto iria3 = val2stack[farg];
                                        caller_fparas_pass.push_back(std::make_pair(fra3, iria3));
                                        flags[fargno] = true;
                                        break;
                                    } else {
                                        if(freg_fpara_map.find(fargno + arg_reg_base) != freg_fpara_map.end()) 
                                            break;
                                        auto fra3 = new FRA(fargno + arg_reg_base);
                                        auto fra3_ = new FRA(src_reg_id);
                                        caller_fparas_pass.push_back(std::make_pair(fra3, fra3_));
                                        flags[fargno] = true;
                                        freg_fpara_map[src_reg_id].erase(fargno);
                                        if(freg_fpara_map[src_reg_id].empty())
                                            freg_fpara_map.erase(src_reg_id);
                                    }
                                }
                            }
                            caller_fpara_sh.clear();
                        }
                        break;
                    }
                }
            }
        }
    }
    return;
}

void AsmGen::getIPass(Instruction* inst, Value* lst_val){
          
               dst_ptr = nullptr;
               setDIPtr(inst);
                for(auto opr: inst->getOperands()) {
                    if(dynamic_cast<BasicBlock*>(opr)) {
                        if(dynamic_cast<BasicBlock*>(opr) == sequence->getBBOfSeq()) {
        
                        if(dynamic_cast<ConstantInt*>(lst_val)) {
                          
                            phi_isrcs.push_back(new IConstPool(dynamic_cast<ConstantInt*>(lst_val)->getValue()));
                           
                        } else if(ival2interval.find(lst_val)!=ival2interval.end()){
                            if(ival2interval[lst_val]->reg >= 0) {
                                if(ireg2loc.find(ival2interval[lst_val]->reg) == ireg2loc.end()) 
                                    ireg2loc.insert({ival2interval[lst_val]->reg, new IRA(ival2interval[lst_val]->reg)});
                             
                                phi_isrcs.push_back(ireg2loc[ival2interval[lst_val]->reg]);
                           
                            } else {
                               
                                phi_isrcs.push_back( val2stack[lst_val]);
                          
                            }
                        }
                              phi_itargets.push_back(dst_ptr);
                    }
                    } else {
                        lst_val = opr;
                    }
                }
                return;
}

void AsmGen::getFPass(Instruction* inst, Value* lst_val){
        
                dst_ptr = nullptr;
                setDFPtr(inst);
                for(auto opr: inst->getOperands()) {
                    if(dynamic_cast<BasicBlock*>(opr)) {
                        if(dynamic_cast<BasicBlock*>(opr) == sequence->getBBOfSeq()) {
        
                        if(dynamic_cast<ConstantFP*>(lst_val)) {
                          
                            phi_fsrcs.push_back(new FConstPool(dynamic_cast<ConstantFP*>(lst_val)->getValue()));
                           
                        } else if(fval2interval.find(lst_val)!=fval2interval.end()){
                            if(fval2interval[lst_val]->reg >= 0) {
                                if(freg2loc.find(fval2interval[lst_val]->reg) == freg2loc.end()) 
                                    freg2loc.insert({fval2interval[lst_val]->reg, new FRA(fval2interval[lst_val]->reg)});
                             
                                phi_fsrcs.push_back(freg2loc[fval2interval[lst_val]->reg]);
                           
                            } else {
                               
                                phi_fsrcs.push_back( val2stack[lst_val]);
                          
                            }
                        }
                              phi_ftargets.push_back(dst_ptr);
                    }
                    } else {
                        lst_val = opr;
                    }
                }
                return;
}

void AsmGen::handleBr(BranchInst* br){
        if(br->getNumOperands() == 1) {
            succ_bb = dynamic_cast<BasicBlock*>(br->getOperand(0));
            succ_br_inst = sequence->createJump(bb2label[succ_bb]);
            sequence->deleteInst();
        } else {
            succ_bb = dynamic_cast<BasicBlock*>(br->getOperand(1));
            fail_bb = dynamic_cast<BasicBlock*>(br->getOperand(2));
            auto cond = br->getOperand(0);
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
        return;
}

void AsmGen::handleCmpbr(CmpBrInst* cmpbr){
    succ_bb = dynamic_cast<BasicBlock*>(cmpbr->getOperand(2));
    fail_bb = dynamic_cast<BasicBlock*>(cmpbr->getOperand(3));
    auto pair = phi_handle_cmp_map.find(cmpbr->getCmpOp());
    if(pair==phi_handle_cmp_map.end())  return;
    pair->second(cmpbr->getOperand(0), cmpbr->getOperand(1));
    return;
}

void AsmGen::handleFCmpbr(FCmpBrInst* fcmpbr){
    succ_bb = dynamic_cast<BasicBlock*>(fcmpbr->getOperand(2));
    fail_bb = dynamic_cast<BasicBlock*>(fcmpbr->getOperand(3));
    auto pair = phi_handle_fcmp_map.find(fcmpbr->getCmpOp());
    if(pair==phi_handle_fcmp_map.end())  return;
    pair->second(fcmpbr->getOperand(0), fcmpbr->getOperand(1));
    return;
}

void AsmGen::initPhi(){    
    restore_ireg_s.clear();
    restore_freg_s.clear();
    succ_move_inst = nullptr;
    fail_move_inst = nullptr;
    move_inst = nullptr;

    succ_br_inst = nullptr;
    fail_br_inst = nullptr;
    

    succ_bb = nullptr;
    fail_bb = nullptr;

    ireg2loc.clear();
    freg2loc.clear();

    
    have_succ_move = false;
    have_fail_move = false;




    for(auto ireg: tmp_iregs_loc) {
        auto ira = new IRA(ireg.first);
        restore_ireg_s.push_back(std::make_pair(ira, ireg.second));
    }

    for(auto freg: tmp_fregs_loc) {
        auto fra = new FRA(freg.first);
        restore_freg_s.push_back(std::make_pair(fra, freg.second));
    }

    tmp_iregs_loc.clear();
    tmp_fregs_loc.clear();
    cur_tmp_iregs.clear();
    cur_tmp_fregs.clear();
    free_locs_for_tmp_regs_saved.clear();
    cur_tmp_reg_saved_stack_offset = 0;

    if(! restore_ireg_s.empty())
        sequence->createStoreTmpRegs(restore_ireg_s);

    if(! restore_freg_s.empty())
        sequence->createStoreTmpRegs(restore_freg_s);
    return;
}

void AsmGen::process(){
    std::vector<std::pair<AddressMode*, AddressMode*>> i_address;
    std::vector<std::pair<AddressMode*, AddressMode*>> f_address;

    //存储可能复用寄存器的编号
    ::std::vector<::std::pair<int, int>> multiplex_iregs;
    
    std::vector<std::pair<AddressMode*, AddressMode*>> delete_iregs;
    ::std::map<std::pair<int, int>, bool> is_rm;

    for(auto succ: sequence->getBBOfSeq()->getSuccBasicBlocks()) {
        i_address.clear();
        f_address.clear();
        phi_isrcs.clear();
        phi_fsrcs.clear();
        phi_itargets.clear();
        phi_ftargets.clear();
        bool is_succ_bb;
        Value *last_value;
        if(succ == succ_bb) {
            is_succ_bb = true;
            move_inst = &succ_move_inst;
        } else {
            is_succ_bb = false;
            move_inst = &fail_move_inst;
        }
        processSucc(succ, last_value);
        if(!phi_isrcs.empty()) {
            mov_value(&i_address, phi_isrcs, phi_itargets, false);
        }
        if(!phi_fsrcs.empty()) {
            mov_value(&f_address, phi_fsrcs, phi_ftargets, true);
        }

        //适用情况：
        //addi r2, r1, 1指令簇
        //mv r1,r2
        //其他情况应该在活跃变量分析和寄存器分配考虑，后端信息比中端少，不好做

        //找到会形成mv指令的目标——源操作数对
        for(auto i: i_address){
            //形成mv指令的充要条件——源操作数和目标操作数都是整型寄存器寻址类型
            if(dynamic_cast<IRA*>(i.first) && dynamic_cast<IRA*>(i.second)){
                //目标——源

                multiplex_iregs.push_back( ::std::pair<int, int>(static_cast<int>(dynamic_cast<IRA*>(i.first)->getReg()), static_cast<int>(dynamic_cast<IRA*>(i.second)->getReg())));
                is_rm[multiplex_iregs.back()] = false;
            }
        }


        //接下来查找add 源, 目标, 1 指令，之所以能复用，就是因为冗余，所以是和上面的可能形成的mv指令的寄存器使用情况相反
        //逆序遍历
        //搜集addi r2, r1, 1指令簇
        auto sequence_insts = sequence->getAsmInsts();
        ::std::vector<Add*> add_list;
        for(auto add_inst = sequence_insts->rbegin(); add_inst!=sequence_insts->rend(); ++add_inst){
            auto asm_inst = *add_inst;
            if(asm_inst->getID() == AsmInst::Op::add){
                auto asm_add = dynamic_cast<Add*>(asm_inst);
                add_list.push_back(asm_add);
            }
            else{
                break;
            }
        }

        bool not_first = false;
        if(!add_list.empty()){
            for(auto iter = sequence_insts->begin(); iter!=sequence_insts->end();){
                auto asm_inst = *iter;
                if(asm_inst->getID() == AsmInst::Op::add){
                    auto asm_add = dynamic_cast<Add*>(asm_inst);
                    if(asm_add == add_list.front() || not_first){
                        not_first = true;
                        GReg* rd = getAddRD(asm_add);
                        int rd_ireg = rd->getID();
                        auto rs1 = getAddRS1(asm_add);
                        auto rs2 = getAddRS2(asm_add);
                        if(dynamic_cast<IConst*>(rs1) && dynamic_cast<GReg*>(rs2) && dynamic_cast<IConst*>(rs1)->getIConst()==1){
                            int rs2_ireg = dynamic_cast<GReg*>(rs2)->getID();
                            auto target_src = ::std::pair<int, int>(rs2_ireg, rd_ireg);
                            auto it = ::std::find(multiplex_iregs.begin(), multiplex_iregs.end(), target_src);
                                if(it!=multiplex_iregs.end()){
                                    is_rm[*it] = true;
                                    iter = sequence_insts->erase(iter);
                                    iter = iter==sequence_insts->end()?sequence_insts->end():iter;
                                    auto add_multiplex_ireg = sequence->createAdd(dynamic_cast<GReg*>(rs2), rs2, rs1);
                                    sequence->deleteInst();
                                    iter = sequence_insts->insert(iter, add_multiplex_ireg);
                                    if(iter!=sequence_insts->end())
                                        iter++;
                                }
                                else{
                                    iter++;
                                }
                        }
                        else if(dynamic_cast<IConst*>(rs2) && dynamic_cast<GReg*>(rs1) && dynamic_cast<IConst*>(rs2)->getIConst()==1){
                            int rs1_ireg = dynamic_cast<GReg*>(rs1)->getID();
                            auto target_src = ::std::pair<int, int>(rs1_ireg, rd_ireg);
                            auto it = ::std::find(multiplex_iregs.begin(), multiplex_iregs.end(), target_src);
                                if(it!=multiplex_iregs.end()){
          
                                    is_rm[*it] = true;
                                    iter = sequence_insts->erase(iter);
                                    iter = iter==sequence_insts->end()?sequence_insts->end():iter;
                                    auto add_multiplex_ireg = sequence->createAdd(dynamic_cast<GReg*>(rs1), rs1, rs2);
                                    sequence->deleteInst();
                                    iter = sequence_insts->insert(iter, add_multiplex_ireg);
                                    if(iter!=sequence_insts->end())
                                        iter++;
                                }
                                else{
                                    iter++;
                                }
                        }
                        else{
                            iter++;
                        }
                    }
  //  iter++;//sss

                    else{
                        iter++;
                    }

                }   
                else{
                    iter++;
                //break;
                }
            }        
        }
    

        for(auto i=i_address.begin(); i!=i_address.end(); ){
            //形成mv指令的充要条件——源操作数和目标操作数都是整型寄存器寻址类型
        
            if(dynamic_cast<IRA*>(i->first) && dynamic_cast<IRA*>(i->second)){
                //目标——源
                if(!multiplex_iregs.empty()){
                auto is_true = ::std::find(multiplex_iregs.begin(), multiplex_iregs.end(), ::std::pair<int, int>(static_cast<int>(dynamic_cast<IRA*>(i->first)->getReg()), static_cast<int>(dynamic_cast<IRA*>(i->second)->getReg())));
                if(is_true!=multiplex_iregs.end() && is_rm[::std::pair<int, int>(static_cast<int>(dynamic_cast<IRA*>(i->first)->getReg()), static_cast<int>(dynamic_cast<IRA*>(i->second)->getReg()))]){
                    i = i_address.erase(i);
                }
                else{
                    i++;
                }
            }
                else{
                    i++;
                }
       //i++;//sss
            }
            else{
                i++;
            }
        
        }

        

        if(! i_address.empty() || ! f_address.empty()) {
            have_succ_move = is_succ_bb;
            have_fail_move = !is_succ_bb; 
            *move_inst = sequence->createPhiPass(i_address, f_address);
            sequence->deleteInst();
        }
    }
    return;
}

void AsmGen::handleEQ(Value* cond1, Value* cond2){
    auto iconst_cond1 = dynamic_cast<ConstantInt*>(cond1);
    auto iconst_cond2 = dynamic_cast<ConstantInt*>(cond2);
    if(iconst_cond1 && iconst_cond2) {
        auto iconst1 = new IConst(iconst_cond1->getValue());
        auto iconst2 = new IConst(iconst_cond2->getValue());
        succ_br_inst = sequence->createBeq(iconst1, iconst2, bb2label[succ_bb]);
                        
    } else if(iconst_cond1) {
                    auto iconst1 = new IConst(iconst_cond1->getValue());
        if(ival2interval[cond2]->reg < 0) {

            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBeq(iconst1, mem2, bb2label[succ_bb]);
            
        } else {
          
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBeq(iconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(iconst_cond2) {
               auto iconst2 = new IConst(iconst_cond2->getValue());
        if(ival2interval[cond1]->reg < 0) {

            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            succ_br_inst = sequence->createBeq(mem1, iconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
     
            succ_br_inst = sequence->createBeq(r1, iconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(ival2interval[cond1]->reg < 0 && ival2interval[cond2]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBeq(mem1, mem2, bb2label[succ_bb]);
           
        } else if(ival2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBeq(mem1, r2, bb2label[succ_bb]);
           
        } else if(ival2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBeq(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBeq(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();  
    return;
}

void AsmGen::handleLT(Value* cond1, Value* cond2){
    auto iconst_cond1 = dynamic_cast<ConstantInt*>(cond1);
    auto iconst_cond2 = dynamic_cast<ConstantInt*>(cond2);
    if(iconst_cond1 && iconst_cond2) {
        auto iconst1 = new IConst(iconst_cond1->getValue());
        auto iconst2 = new IConst(iconst_cond2->getValue());
        succ_br_inst = sequence->createBlt(iconst1, iconst2, bb2label[succ_bb]);
                        
    } else if(iconst_cond1) {
                auto iconst1 = new IConst(iconst_cond1->getValue());
        if(ival2interval[cond2]->reg < 0) {
       
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBlt(iconst1, mem2, bb2label[succ_bb]);
            
        } else {
    
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBlt(iconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(iconst_cond2) {
               auto iconst2 = new IConst(iconst_cond2->getValue());
        if(ival2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));

            succ_br_inst = sequence->createBlt(mem1, iconst2, bb2label[succ_bb]);
           
        } else {
            auto r1 = getAllocaReg(cond1);
     
            succ_br_inst = sequence->createBlt(r1, iconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(ival2interval[cond1]->reg < 0 && ival2interval[cond2]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBlt(mem1, mem2, bb2label[succ_bb]);
            
        } else if(ival2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBlt(mem1, r2, bb2label[succ_bb]);
            
        } else if(ival2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 = new Mem(  val2stack[cond2]->getOffset(), static_cast<int>( val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBlt(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBlt(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();  
    return;
}

void AsmGen::handleGE(Value* cond1, Value* cond2){
    auto iconst_cond1 = dynamic_cast<ConstantInt*>(cond1);
    auto iconst_cond2 = dynamic_cast<ConstantInt*>(cond2);
    if(iconst_cond1 && iconst_cond2) {
        auto iconst1 = new IConst(iconst_cond1->getValue());
        auto iconst2 = new IConst(iconst_cond2->getValue());
        succ_br_inst = sequence->createBge(iconst1, iconst2, bb2label[succ_bb]);
                          
    } else if(iconst_cond1) {
                   auto iconst1 = new IConst(iconst_cond1->getValue());
        if(ival2interval[cond2]->reg < 0) {
      
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBge(iconst1, mem2, bb2label[succ_bb]);
            
        } else {
 
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBge(iconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(iconst_cond2) {
        auto iconst2 = new IConst(iconst_cond2->getValue());
        if(ival2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
     
            succ_br_inst = sequence->createBge(mem1, iconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            
            succ_br_inst = sequence->createBge(r1, iconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(ival2interval[cond1]->reg < 0 && ival2interval[cond2]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBge(mem1, mem2, bb2label[succ_bb]);
            
        } else if(ival2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBge(mem1, r2, bb2label[succ_bb]);
            
        } else if(ival2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBge(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 =  getAllocaReg(cond2);
            succ_br_inst = sequence->createBge(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst(); 
    return; 
}

void AsmGen::handleNE(Value* cond1, Value* cond2){
    auto iconst_cond1 = dynamic_cast<ConstantInt*>(cond1);
    auto iconst_cond2 = dynamic_cast<ConstantInt*>(cond2);
    if(iconst_cond1 && iconst_cond2) {
        auto iconst1 = new IConst(iconst_cond1->getValue());
        auto iconst2 = new IConst(iconst_cond2->getValue());
        succ_br_inst = sequence->createBne(iconst1, iconst2, bb2label[succ_bb]);
                          
    } else if(iconst_cond1) {
         auto iconst1 = new IConst(iconst_cond1->getValue());
        if(ival2interval[cond2]->reg < 0) {
           
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBne(iconst1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBne(iconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(iconst_cond2) {
         auto iconst2 = new IConst(iconst_cond2->getValue());
        if(ival2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
        
            succ_br_inst = sequence->createBne(mem1, iconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
           
            succ_br_inst = sequence->createBne(r1, iconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(ival2interval[cond1]->reg < 0 && ival2interval[cond2]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBne(mem1, mem2, bb2label[succ_bb]);
            
        } else if(ival2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBne(mem1, r2, bb2label[succ_bb]);
            
        } else if(ival2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createBne(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createBne(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();  
    return;
}

void AsmGen::handleFEQ(Value* cond1, Value* cond2){
    auto fconst_cond1 = dynamic_cast<ConstantFP*>(cond1);
    auto fconst_cond2 = dynamic_cast<ConstantFP*>(cond2);
    if(fconst_cond1 && fconst_cond2) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        auto fconst2 = new FConst(fconst_cond2->getValue());
        succ_br_inst = sequence->createFBeq(fconst1, fconst2, bb2label[succ_bb]);
                      
    } else if(fconst_cond1) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        if(fval2interval[cond2]->reg < 0) {
            
            auto mem2 = new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBeq(fconst1, mem2, bb2label[succ_bb]);
            
        } else {
            
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBeq(fconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(fconst_cond2) {
        auto fconst2 = new FConst(fconst_cond2->getValue());
        if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            
            succ_br_inst = sequence->createFBeq(mem1, fconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            
            succ_br_inst = sequence->createFBeq(r1, fconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()) );
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBeq(mem1, mem2, bb2label[succ_bb]);
            
        } else if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBeq(mem1, r2, bb2label[succ_bb]);
           
        } else if(fval2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 =  new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBeq(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBeq(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();
    return;
}

void AsmGen::handleFLT(Value* cond1, Value* cond2){

    auto fconst_cond1 = dynamic_cast<ConstantFP*>(cond1);
    auto fconst_cond2 = dynamic_cast<ConstantFP*>(cond2);
    if(fconst_cond1 && fconst_cond2) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        auto fconst2 = new FConst(fconst_cond2->getValue());
        succ_br_inst = sequence->createFBlt(fconst1, fconst2, bb2label[succ_bb]);
                      
    } else if(fconst_cond1) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        if(fval2interval[cond2]->reg < 0) {
            
            auto mem2 = new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBlt(fconst1, mem2, bb2label[succ_bb]);
            
        } else {
            
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBlt(fconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(fconst_cond2) {
        auto fconst2 = new FConst(fconst_cond2->getValue());
        if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            
            succ_br_inst = sequence->createFBlt(mem1, fconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            
            succ_br_inst = sequence->createFBlt(r1, fconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()) );
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBlt(mem1, mem2, bb2label[succ_bb]);
            
        } else if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBlt(mem1, r2, bb2label[succ_bb]);
           
        } else if(fval2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 =  new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBlt(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBlt(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();
    return;
}

void AsmGen::handleFGE(Value* cond1, Value* cond2){
  
    auto fconst_cond1 = dynamic_cast<ConstantFP*>(cond1);
    auto fconst_cond2 = dynamic_cast<ConstantFP*>(cond2);
    if(fconst_cond1 && fconst_cond2) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        auto fconst2 = new FConst(fconst_cond2->getValue());
        succ_br_inst = sequence->createFBge(fconst1, fconst2, bb2label[succ_bb]);
                      
    } else if(fconst_cond1) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        if(fval2interval[cond2]->reg < 0) {
            
            auto mem2 = new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBge(fconst1, mem2, bb2label[succ_bb]);
            
        } else {
            
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBge(fconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(fconst_cond2) {
        auto fconst2 = new FConst(fconst_cond2->getValue());
        if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            
            succ_br_inst = sequence->createFBge(mem1, fconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            
            succ_br_inst = sequence->createFBge(r1, fconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()) );
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBge(mem1, mem2, bb2label[succ_bb]);
            
        } else if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBge(mem1, r2, bb2label[succ_bb]);
           
        } else if(fval2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 =  new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBge(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBge(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();
    return;
}

void AsmGen::handleFNE(Value* cond1, Value* cond2){

    auto fconst_cond1 = dynamic_cast<ConstantFP*>(cond1);
    auto fconst_cond2 = dynamic_cast<ConstantFP*>(cond2);
    if(fconst_cond1 && fconst_cond2) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        auto fconst2 = new FConst(fconst_cond2->getValue());
        succ_br_inst = sequence->createFBne(fconst1, fconst2, bb2label[succ_bb]);
                      
    } else if(fconst_cond1) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        if(fval2interval[cond2]->reg < 0) {
            
            auto mem2 = new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBne(fconst1, mem2, bb2label[succ_bb]);
            
        } else {
            
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBne(fconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(fconst_cond2) {
        auto fconst2 = new FConst(fconst_cond2->getValue());
        if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            
            succ_br_inst = sequence->createFBne(mem1, fconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            
            succ_br_inst = sequence->createFBne(r1, fconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()) );
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBne(mem1, mem2, bb2label[succ_bb]);
            
        } else if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBne(mem1, r2, bb2label[succ_bb]);
           
        } else if(fval2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 =  new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBne(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBne(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();
    return;
}

void AsmGen::handleFGT(Value* cond1, Value* cond2){

    auto fconst_cond1 = dynamic_cast<ConstantFP*>(cond1);
    auto fconst_cond2 = dynamic_cast<ConstantFP*>(cond2);
    if(fconst_cond1 && fconst_cond2) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        auto fconst2 = new FConst(fconst_cond2->getValue());
        succ_br_inst = sequence->createFBgt(fconst1, fconst2, bb2label[succ_bb]);
                      
    } else if(fconst_cond1) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        if(fval2interval[cond2]->reg < 0) {
            
            auto mem2 = new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBgt(fconst1, mem2, bb2label[succ_bb]);
            
        } else {
            
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBgt(fconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(fconst_cond2) {
        auto fconst2 = new FConst(fconst_cond2->getValue());
        if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            
            succ_br_inst = sequence->createFBgt(mem1, fconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            
            succ_br_inst = sequence->createFBgt(r1, fconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()) );
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBgt(mem1, mem2, bb2label[succ_bb]);
            
        } else if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBgt(mem1, r2, bb2label[succ_bb]);
           
        } else if(fval2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 =  new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBgt(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBgt(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();
    return;
}

void AsmGen::handleFLE(Value* cond1, Value* cond2){

    auto fconst_cond1 = dynamic_cast<ConstantFP*>(cond1);
    auto fconst_cond2 = dynamic_cast<ConstantFP*>(cond2);
    if(fconst_cond1 && fconst_cond2) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        auto fconst2 = new FConst(fconst_cond2->getValue());
        succ_br_inst = sequence->createFBle(fconst1, fconst2, bb2label[succ_bb]);
                      
    } else if(fconst_cond1) {
        auto fconst1 = new FConst(fconst_cond1->getValue());
        if(fval2interval[cond2]->reg < 0) {
            
            auto mem2 = new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBle(fconst1, mem2, bb2label[succ_bb]);
            
        } else {
            
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBle(fconst1, r2, bb2label[succ_bb]);
            
        }
    } else if(fconst_cond2) {
        auto fconst2 = new FConst(fconst_cond2->getValue());
        if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            
            succ_br_inst = sequence->createFBle(mem1, fconst2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            
            succ_br_inst = sequence->createFBle(r1, fconst2, bb2label[succ_bb]);
            
        }
    } else {
        if(fval2interval[cond1]->reg < 0 && fval2interval[cond2]->reg < 0) {
            auto mem1 = new Mem(val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()) );
            auto mem2 = new Mem( val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBle(mem1, mem2, bb2label[succ_bb]);
            
        } else if(fval2interval[cond1]->reg < 0) {
            auto mem1 = new Mem( val2stack[cond1]->getOffset(), static_cast<int>(val2stack[cond1]->getReg()));
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBle(mem1, r2, bb2label[succ_bb]);
           
        } else if(fval2interval[cond2]->reg < 0) {
            auto r1 = getAllocaReg(cond1);
            auto mem2 =  new Mem(val2stack[cond2]->getOffset(), static_cast<int>(val2stack[cond2]->getReg()));
            succ_br_inst = sequence->createFBle(r1, mem2, bb2label[succ_bb]);
            
        } else {
            auto r1 = getAllocaReg(cond1);
            auto r2 = getAllocaReg(cond2);
            succ_br_inst = sequence->createFBle(r1, r2, bb2label[succ_bb]);
            
        }
    }
    sequence->deleteInst();
    fail_br_inst = sequence->createJump(bb2label[fail_bb]);
    sequence->deleteInst();
    return;
}

void AsmGen::setDIPtr(Instruction* inst){
    if(ival2interval[inst]->reg >-1) {
        if(ireg2loc.find(ival2interval[inst]->reg) == ireg2loc.end())  
            ireg2loc.insert({ival2interval[inst]->reg, new IRA(ival2interval[inst]->reg)});
        dst_ptr = ireg2loc[ival2interval[inst]->reg];
    } else {
        dst_ptr = val2stack[inst];
    }
    return;
}

void AsmGen::setDFPtr(Instruction* inst){
    if(fval2interval[inst]->reg >-1) {
        if(freg2loc.find(fval2interval[inst]->reg) == freg2loc.end())    
            freg2loc.insert({fval2interval[inst]->reg, new FRA(fval2interval[inst]->reg)});
        dst_ptr = freg2loc[fval2interval[inst]->reg];
    } else {
        dst_ptr = val2stack[inst];
    }
    return;
}

void AsmGen::processSucc(BasicBlock* succ, Value* last_value){
for(auto inst: succ->getInstructions()) {
            
            if(inst->isPhi()){
                last_value = nullptr;
            if(inst->getType()->isFloatType()) {
                getFPass(inst, last_value);
            } else {
                getIPass(inst, last_value);
            }
            }
            else{
                break;
            }
            


        }
        return;
}



void AsmGen::recordIReg(Instruction* inst, std::set<int>* record_iregs){
    if(ival2interval[inst]->reg >= 0) {
        if(cur_tmp_iregs.find(ival2interval[inst]->reg) != cur_tmp_iregs.end()) {
            record_iregs->insert(ival2interval[inst]->reg);
        }
    }
    return;
}

void AsmGen::recordFReg(Instruction* inst, std::set<int>* record_fregs){
    if(fval2interval[inst]->reg >= 0) {
        if(cur_tmp_fregs.find(fval2interval[inst]->reg) != cur_tmp_fregs.end()) {
            record_fregs->insert(fval2interval[inst]->reg);
        }
    }
    return;
}

