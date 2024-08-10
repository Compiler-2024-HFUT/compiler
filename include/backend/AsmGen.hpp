//该文件里写ASM的内存构建
#ifndef ASMGEN_HPP
#define ASMGEN_HPP

#include "midend/IRVisitor.hpp"
#include "Asm.hpp"
#include "LSRA.hpp"

#include <vector>
#include <map>

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

//以8字节为对齐
#define align_8(address) ((address+7)/8)*8

//以4字节为对齐
#define align_4(address) ((address+3)/4)*4

extern::std::vector<int> all_alloca_gprs;

extern::std::vector<int> all_alloca_fprs;
class AsmGen : public IRVisitor{
    public:
        AsmGen(Module* module):b_inst_gen_map{
            {Instruction::OpID::add, [this](BinaryInst* inst)->void{visitAdd(inst);}},
            {Instruction::OpID::sub, [this](BinaryInst* inst)->void{visitSub(inst);}},
            {Instruction::OpID::mul, [this](BinaryInst* inst)->void{visitMul(inst);}},
            {Instruction::OpID::mul64, [this](BinaryInst* inst)->void{visitMul64(inst);}},
            {Instruction::OpID::sdiv, [this](BinaryInst* inst)->void{visitSDiv(inst);}},
            {Instruction::OpID::srem, [this](BinaryInst* inst)->void{visitSRem(inst);}},
            {Instruction::OpID::asr, [this](BinaryInst* inst)->void{visitAsr(inst);}},
            {Instruction::OpID::shl, [this](BinaryInst* inst)->void{visitShl(inst);}},
            {Instruction::OpID::lsr, [this](BinaryInst* inst)->void{visitLsr(inst);}},
            {Instruction::OpID::asr64, [this](BinaryInst* inst)->void{visitAsr64(inst);}},
            {Instruction::OpID::shl64, [this](BinaryInst* inst)->void{visitShl64(inst);}},
            {Instruction::OpID::lsr64, [this](BinaryInst* inst)->void{visitLsr64(inst);}},
            {Instruction::OpID::land, [this](BinaryInst* inst)->void{visitLAnd(inst);}},
            {Instruction::OpID::fadd, [this](BinaryInst* inst)->void{visitFAdd(inst);}},
            {Instruction::OpID::fsub, [this](BinaryInst* inst)->void{visitFSub(inst);}},
            {Instruction::OpID::fmul, [this](BinaryInst* inst)->void{visitFMul(inst);}},
            {Instruction::OpID::fdiv, [this](BinaryInst* inst)->void{visitFDiv(inst);}}
        }, cmp_inst_gen_map{
            {CmpOp::EQ, [this](CmpInst* inst)->void{visitEQ(inst);}}, 
            {CmpOp::NE, [this](CmpInst* inst)->void{visitNE(inst);}}
        }, fcmp_inst_gen_map{
            {CmpOp::EQ, [this](FCmpInst* inst)->void{visitFEQ(inst);}},
            {CmpOp::GE, [this](FCmpInst* inst)->void{visitFGE(inst);}},
            {CmpOp::GT, [this](FCmpInst* inst)->void{visitFGT(inst);}},
            {CmpOp::LE, [this](FCmpInst* inst)->void{visitFLE(inst);}},
            {CmpOp::LT, [this](FCmpInst* inst)->void{visitFLT(inst);}},
            {CmpOp::NE, [this](FCmpInst* inst)->void{visitFNE(inst);}}
        }, phi_handle_cmp_map{
            {CmpOp::EQ, [this](Value* cond1, Value* cond2)->void{handleEQ(cond1, cond2);}},
            {CmpOp::LT, [this](Value* cond1, Value* cond2)->void{handleLT(cond1, cond2);}},
            {CmpOp::GE, [this](Value* cond1, Value* cond2)->void{handleGE(cond1, cond2);}},
            {CmpOp::NE, [this](Value* cond1, Value* cond2)->void{handleNE(cond1, cond2);}}

        }, phi_handle_fcmp_map{
            {CmpOp::EQ, [this](Value* cond1, Value* cond2)->void{handleFEQ(cond1, cond2);}},
            {CmpOp::LT, [this](Value* cond1, Value* cond2)->void{handleFLT(cond1, cond2);}},
            {CmpOp::GE, [this](Value* cond1, Value* cond2)->void{handleFGE(cond1, cond2);}},
            {CmpOp::NE, [this](Value* cond1, Value* cond2)->void{handleFNE(cond1, cond2);}},
            {CmpOp::GT, [this](Value* cond1, Value* cond2)->void{handleFGT(cond1, cond2);}},
            {CmpOp::LE, [this](Value* cond1, Value* cond2)->void{handleFLE(cond1, cond2);}}
        }{
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
        virtual void visit(LoadImmInst &node) override;
    
    private:
        void visitAdd(BinaryInst* inst);
        void visitSub(BinaryInst* inst);
        void visitMul(BinaryInst* inst);
        void visitMul64(BinaryInst* inst);
        void visitSDiv(BinaryInst* inst);
        void visitSRem(BinaryInst* inst);
        void visitAsr(BinaryInst* inst);
        void visitShl(BinaryInst* inst);
        void visitLsr(BinaryInst* inst);
        void visitAsr64(BinaryInst* inst);
        void visitShl64(BinaryInst* inst);
        void visitLsr64(BinaryInst* inst);
        void visitLAnd(BinaryInst* inst);
        void visitFAdd(BinaryInst* inst);
        void visitFSub(BinaryInst* inst);
        void visitFMul(BinaryInst* inst);
        void visitFDiv(BinaryInst* inst);

        void visitEQ(CmpInst* inst);
        void visitNE(CmpInst* inst);

        void visitFEQ(FCmpInst* inst);
        void visitFGE(FCmpInst* inst);
        void visitFGT(FCmpInst* inst);
        void visitFLE(FCmpInst* inst);
        void visitFLT(FCmpInst* inst);
        void visitFNE(FCmpInst* inst);

    private:
        void handleEQ(Value* cond1, Value* cond2);
        void handleLT(Value* cond1, Value* cond2);
        void handleGE(Value* cond1, Value* cond2);
        void handleNE(Value* cond1, Value* cond2);

        void handleFEQ(Value* cond1, Value* cond2);
        void handleFLT(Value* cond1, Value* cond2);
        void handleFGE(Value* cond1, Value* cond2);
        void handleFNE(Value* cond1, Value* cond2);
        void handleFGT(Value* cond1, Value* cond2);
        void handleFLE(Value* cond1, Value* cond2);
    
    private:
        const ::std::map<Instruction::OpID, ::std::function<void(BinaryInst*)>> b_inst_gen_map;
        const ::std::map<CmpOp, ::std::function<void(CmpInst*)>> cmp_inst_gen_map;
        const ::std::map<CmpOp, ::std::function<void(FCmpInst*)>> fcmp_inst_gen_map;

        const ::std::map<CmpOp, ::std::function<void(Value* , Value* )>> phi_handle_cmp_map;
        const ::std::map<CmpOp, ::std::function<void(Value* , Value* )>> phi_handle_fcmp_map;

    private:
        GReg* getGRD(Instruction* inst);
        FReg* getFRD(Instruction* inst);        
        Val* getIRS1(Instruction* inst);
        Val* getIRS2(Instruction* inst);
        Val* getFRS1(Instruction* inst);
        Val* getFRS2(Instruction* inst);


    private:

    private:
        AsmUnit* asm_unit;
        Subroutine* subroutine;
        Sequence* sequence;

     
        std::map<Value*, Interval*> ival2interval;
        std::map<Value*, Interval*> fval2interval;

        std::map<Function*, map<Value*, Interval*>> ivalue_interval_map;
        std::map<Function*, map<Value*, Interval*>> fvalue_interval_map;

        int cur_tmp_reg_saved_stack_offset = 0;
        int caller_saved_regs_stack_offset = 0;
        int caller_trans_args_stack_offset = 0;

  
    std::set<Interval*> use_tmp_regs_interval;
        std::set<int> cur_tmp_iregs;                                //// 当前用来保存栈上值的临时寄存器
        std::set<int> cur_tmp_fregs;                                //// 当前用来保存栈上值的临时寄存器
        std::map<int, IRIA*> tmp_iregs_loc;                      //// 临时寄存器原值保存的地址(位于函数栈布局中临时寄存器保存区)
        std::map<int, IRIA*> tmp_fregs_loc;                      //// 临时寄存器原值保存的地址(位于函数栈布局中临时寄存器保存区)

        std::set<IRIA*> free_locs_for_tmp_regs_saved;

     
        std::vector<BasicBlock*> linear_bbs;
        std::map<BasicBlock*, Label *> bb2label; 

     
        std::pair<std::vector<int>, std::vector<int>> used_iregs_pair; 
        std::pair<std::vector<int>, std::vector<int>> used_fregs_pair; 
        
       
        ::std::map<Value*, IRIA*> val2stack;

        ::std::vector<int> icallee = {
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

        ::std::vector<int> fcallee = {
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


    
        const int var_align = 2;
        const int func_align = 1;
        const int reg_size = 8;

        const int arg_reg_base = 10;  //a0 or fa0

        int total_size;
        int iargs_size;
        int fargs_size;

        int save_offset;


        void iparas_pass_callee(Function *func);
        void fparas_pass_callee(Function *func);
        void iparas_pass_caller(CallInst *call);
        void fparas_pass_caller(CallInst *call);

        int setCallerAndCalleeRegs();

        int allocateMemForIArgs();
        int allocateMemForFArgs();

        int allocateMemForIPointer();
        int allocateMemForFPointer();

        int allocateMemForAlloca();

        std::vector<std::pair<IRA*, IRIA*>> getCalleeSaveIRegs();
        std::vector<std::pair<FRA*, IRIA*>> getCalleeSaveFRegs();

        void addIPara(::std::vector<std::pair<AddressMode*, AddressMode*>>* iparas_pass, ::std::map<int, bool>* flags);
        void addFPara(::std::vector<std::pair<AddressMode*, AddressMode*>>* fparas_pass, ::std::map<int, bool>* flags);

        void addILoopPara(::std::vector<std::pair<AddressMode*, AddressMode*>>* iparas_pass, ::std::map<int, bool>* flags);
        void addFLoopPara(::std::vector<std::pair<AddressMode*, AddressMode*>>* fparas_pass, ::std::map<int, bool>* flags);

        void initIPara(std::vector<Value*>* ip_s, std::map<int, bool>* flags, int num, std::map<int, std::set<int>>* ireg_ipara_map);
        void initFPara(std::vector<Value*>* fp_s, std::map<int, bool>* flags, int num, std::map<int, std::set<int>>* freg_fpara_map);

        void processIPara(std::map<int, bool> flags, std::map<int, std::set<int>> ireg_ipara_map, std::vector<Value*> iargs);
        void processFPara(std::map<int, bool> flags, std::map<int, std::set<int>> freg_fpara_map, std::vector<Value*> fargs);


        void addIStackPara(std::vector<Value*> iargs);
        void addFStackPara(std::vector<Value*> fargs);

        void getIPass(Instruction* inst, Value* lst_val);
        void getFPass(Instruction* inst, Value* lst_val);

        void handleFCmpbr(FCmpBrInst* fcmpbr);
        void handleCmpbr(CmpBrInst* cmpbr);
        void handleBr(BranchInst* br);
        void initPhi();
        void process();

        void setDIPtr(Instruction* inst);
        void setDFPtr(Instruction* inst);

        void processSucc(BasicBlock* succ, Value* last_value);

        void loadITmpReg(Instruction* inst, std::set<int>* load_iregs, std::set<int>* record_iregs);
        void loadFTmpReg(Instruction* inst, std::set<int>* load_fregs, std::set<int>* record_fregs);

        void recordIReg(Instruction* inst, std::set<int>* record_iregs);
        void recordFReg(Instruction* inst, std::set<int>* record_fregs);
        

        std::map<int, IRIA*> caller_saved_ireg_locs;             //// caller在调用函数前保存寄存器的位置
        std::map<int, IRIA*> caller_saved_freg_locs;             //// caller在调用函数前保存寄存器的位置
        std::vector<int> caller_save_iregs;
        std::vector<int> caller_save_fregs;

        
        std::set<int> istore_list;                                   //// 待保存原值的临时使用的整数寄存器
        std::set<int> fstore_list;                                   //// 待保存原值的临时使用的浮点寄存器

        std::set<Value*> to_store_ivals; 
        std::set<Value*> to_store_fvals;



    void mov_value(std::vector<std::pair<AddressMode*, AddressMode*>>* to_move_locs, std::vector<AddressMode*>& src, std::vector<AddressMode*>&dst, bool is_float);
    


        
    Val *getAllocaReg(Value* value);

       
    std::map<GlobalVariable*, Label*> global_variable_labels_table;

    //收集call指令前定义寄存器的信息
    ::std::map<Instruction* , ::std::vector<int>> call_define_ireg_map;
    ::std::map<Instruction* , ::std::vector<int>> call_define_freg_map;

    ::std::vector<std::pair<AddressMode*, AddressMode*>> iparas_pass;
    ::std::vector<std::pair<AddressMode*, AddressMode*>> fparas_pass;

    std::list<std::pair<Argument*, int>> ipara_sh;
    std::list<std::pair<Argument*, int>> fpara_sh;

    ::std::vector<std::pair<AddressMode*, AddressMode*>> caller_iparas_pass;
    ::std::vector<std::pair<AddressMode*, AddressMode*>> caller_fparas_pass;

    ::std::vector<std::pair<Value*, int>> caller_ipara_sh;
    ::std::vector<std::pair<Value*, int>> caller_fpara_sh;

    std::vector<AddressMode*> phi_itargets;
    std::vector<AddressMode*> phi_isrcs;
    std::vector<AddressMode*> phi_ftargets;
    std::vector<AddressMode*> phi_fsrcs;

      
    std::map<int, AddressMode*> ireg2loc;
    std::map<int, AddressMode*> freg2loc;

    BasicBlock *succ_bb;
    BasicBlock *fail_bb;

    AsmInst *succ_br_inst;
    AsmInst *fail_br_inst;

    PhiPass *succ_move_inst;
    PhiPass *fail_move_inst;
    PhiPass **move_inst;

    bool have_succ_move;    
    bool have_fail_move;

    AddressMode *dst_ptr;

    std::vector<std::pair<IRA*, IRIA*>> restore_ireg_s;
    std::vector<std::pair<FRA*, IRIA*>> restore_freg_s;
    

    BasicBlock *ret_bb;

friend class LSRA;
    
};



#endif