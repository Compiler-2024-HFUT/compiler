//该文件里写ASM的相关数据结构的定义
#ifndef ASM_HPP
#define ASM_HPP

#include "RISCVInst.hpp"
#include "RISCVReg.hpp"
#include "AsmString.hpp"
#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"

#include <vector>
#include <utility>

class Val{
    public:
        virtual bool isReg() = 0;
        virtual bool isConst() = 0;
        virtual ::std::string print() = 0;
};

class IConst: public Val{
    public:
        IConst(int val): val(val) {}
        bool isConst() {return true;}
        bool isIConst() {return true;}
        bool isReg() {return false;}
        int &getIConst() {return val;}
        ::std::string print() final;

    private:
        int val;
};

class FConst: public Val{
    public:
        FConst(float val): val(val) {}
        bool isConst() {return true;}
        bool isIConst() {return false;}
        bool isReg() {return false;}
        float &getFConst() {return val;}
        ::std::string print() final;

    private:
        float val;
};



class GReg: public Val{
    public:
        GReg(int id): reg(RISCV::id2GReg(id)){}
        bool isReg() {return true;}
        bool isGPR() {return true;}
        bool isConst() {return false;}
        int getID() {return static_cast<int>(reg);}
        ::std::string print() final;

    private:
        RISCV::GPR reg;
};

class FReg: public Val{
    public:
        FReg(int id): reg(RISCV::id2FReg(id)){}
        bool isReg() {return true;}
        bool isGPR() {return false;}
        bool isConst() {return false;}
        int getID() {return static_cast<int>(reg);}
        ::std::string print() final;

    private:
        RISCV::FPR reg;
};

class Mem: public Val{
    public:
        Mem(int offset, int reg_id): offset(offset), reg(RISCV::id2GReg(reg_id)) {}
        int getRegId() {return static_cast<int>(reg);}
        int getOffset() {return offset;}
        bool isReg() {return false;}
        bool isConse() {return false;}
        ::std::string print() final;

    private:
        int offset;
        RISCV::GPR reg; //地址相关的一定是整型，不可能是浮点数，所以直接就是GPR
};


//寻址方式
class AddressMode{
  public:
    virtual ::std::string print() = 0;  
};


//标号
class Label: public AddressMode{
    public:
        Label(::std::string label): label(label){}
        ::std::string getLabel(){return label;}
        ::std::string print() final;
    private:
        ::std::string label;
};


class IConstPool: AddressMode{
    public:
        IConstPool(int i_const_pool): i_const_pool(i_const_pool){}
        bool isI(){return true;}
        int &getIConstPool(){return i_const_pool;}
        ::std::string print() final;

    private:
        int i_const_pool;
};

class FConstPool: AddressMode{
    public:
        FConstPool(float f_const_pool): f_const_pool(f_const_pool){}
        bool isI(){return false;}
        float &getFConstPool(){return f_const_pool;}
        ::std::string print() final;

    private:
        float f_const_pool;
};

//寄存器寻址
class IRA: public AddressMode{
    public:
        IRA(int id):reg(RISCV::id2GReg(id)){}
        bool isI(){return true;}
        RISCV::GPR getReg(){return reg;}

        ::std::string print() final;

    private:
        RISCV::GPR reg; 

};


class FRA: public AddressMode{
    public:
        FRA(int id):reg(RISCV::id2FReg(id)){}
        bool isI(){return false;}
        RISCV::FPR getReg(){return reg;}

        ::std::string print() final;

    private:
        RISCV::FPR reg; 

};



//寄存器间接寻址
class IRIA: public AddressMode{
    public:
        IRIA(int id, int offset):reg(RISCV::id2GReg(id)), offset(offset){}
        bool isI(){return true;}
        RISCV::GPR getReg(){return reg;}
        int getOffset(){return offset;}
        ::std::string print() final;

    private:
        RISCV::GPR reg; 
        int offset;

};



class AsmUnit;
class Subroutine;
class Sequence;
class Label;
class AsmInst;

class Add;
class Subw;
class Mulw;
class Muld;
class Divw;
class Remw;
class Sraw;
class Sllw;
class Srlw;
class Sra;
class Sll;
class Srl;
class Land; 
class Fadd_s;
class Fsub_s; 
class Fmul_s; 
class Fdiv_s; 
class Fcvt_w_s;
class Fcvt_s_w;
class Zext;
class Snez;
class Seqz;
class Feq_s;
class Fle_s;
class Flt_s;
class Fgt_s;
class Fge_s;
class Fne_s;
class Lw;
class Lw_label;
class Flw;
class Flw_label;
class Sw;
class Sw_label;
class Fsw;
class Fsw_label;
class Call;
class La;
class Mv;
class Beq;
class Bne;
class Bge;
class Blt;
class FBeq;
class FBge;
class FBgt;
class FBle;
class FBlt;
class FBne;
class Jump;
class Ret;

class CallerSaveRegs;
class CallerSaveRegs;
class CalleeSaveRegs;
class CalleeSaveRegs;
class CallerRestoreRegs;
class CallerRestoreRegs;
class CalleeRestoreRegs;
class CalleeRestoreRegs;
class CallerParaPass;
class CalleeParaPass;
class CallerSaveResult;
class CallerSaveResult;
class CalleeSaveResult;
class CalleeSaveResult;
class CalleeStackFrameInitialize;
class CalleeStackFrameClear;
class CalleeStackFrameExpand;
class CalleeStackFrameShrink;
class LoadTmpRegs;
class LoadTmpRegs;
class StoreTmpRegs;
class StoreTmpRegs;
class AllocaTmpRegs;
class AllocaTmpRegs;
class InitializeAllTempRegs;
class InitializeAllTempRegs;
class PhiPass;

//序列（指令序列，对若干指令的抽象，抽象层次介于子程序与指令之间）
class Sequence{
    public:
        Sequence(Subroutine* subroutine, BasicBlock* bb, Label* label): parent(subroutine), bb(bb), label(label){}
        void appendInst(AsmInst* inst){
            insts.push_back(inst);
        }
        void deleteInst(){
            insts.pop_back();
        }
        BasicBlock* getBBOfSeq(){return bb;}
        Label* getLabelOfSeq(){return label;}
        Subroutine* getSubroutineOfSeq(){return parent;}
        ::std::string print();


    public:
        Add* createAdd(GReg* rd,Val* rs1,Val* rs2);
        Subw* createSubw(GReg* rd,Val* rs1,Val* rs2);
        Mulw* createMulw(GReg* rd,Val* rs1,Val* rs2);
        Muld* createMuld(GReg* rd,Val* rs1,Val* rs2);
        Divw* createDivw(GReg* rd,Val* rs1,Val* rs2);
        Remw* createRemw(GReg* rd,Val* rs1,Val* rs2);
        Sraw* createSraw(GReg* rd,Val* rs1,IConst* iconst_rs2);
        Sllw* createSllw(GReg* rd,Val* rs1,IConst* iconst_rs2);
        Srlw* createSrlw(GReg* rd,Val* rs1,IConst* iconst_rs2);
        Sra* createSra(GReg* rd,Val* rs1,IConst* iconst_rs2);
        Sll* createSll(GReg* rd,Val* rs1,IConst* iconst_rs2);
        Srl* createSrl(GReg* rd,Val* rs1,IConst* iconst_rs2);
        Land* createLand(GReg* rd,GReg* rs1,IConst* iconst_rs2);
        Fadd_s* createFadd_s(FReg* rd,Val* rs1,Val* rs2);
        Fsub_s* createFsub_s(FReg* rd,Val* rs1,Val* rs2);
        Fmul_s* createFmul_s(FReg* rd,Val* rs1,Val* rs2);
        Fdiv_s* createFdiv_s(FReg* rd,Val* rs1,Val* rs2);
        Fcvt_w_s* createFcvt_w_s(GReg* rd,Val* rs1);
        Fcvt_s_w* createFcvt_s_w(FReg* rd,Val* rs1);
        Zext* createZext(GReg* rd,GReg* rs1);
        Snez* createSnez(GReg* rd,Val* cond);
        Seqz* createSeqz(GReg* rd,Val* cond);
        Feq_s* createFeq_s(GReg* rd,Val* cond1,Val* cond2);
        Fle_s* createFle_s(GReg* rd,Val* cond1,Val* cond2);
        Flt_s* createFlt_s(GReg* rd,Val* cond1,Val* cond2);
        Fgt_s* createFgt_s(GReg* rd,Val* cond1,Val* cond2);
        Fge_s* createFge_s(GReg* rd,Val* cond1,Val* cond2);
        Fne_s* createFne_s(GReg* rd,Val* cond1,Val* cond2);
        Lw* createLw(GReg* rd, GReg* base, Val*offset);
        Lw_label* createLw_label(GReg* rd, Label* label);
        Flw* createFlw(FReg* rd, GReg* base, Val*offset);
        Flw_label* createFlw_label(FReg* rd, Label* label);
        Sw* createSw(Val* src, GReg* base, Val*offset);
        Sw_label* createSw_label(Val* src, Label* label);
        Fsw* createFsw(Val* src, GReg* base, Val*offset);
        Fsw_label* createFsw_label(Val* src, Label* label);
        Call* createCall(Label* label);
        La* createLa(GReg* rd, Label* label);
        Mv* createMv(GReg* rd, GReg* rs1);
        Beq* createBeq(Val* cond1, Val* cond2, Label* label);
        Bne* createBne(Val* cond1, Val* cond2, Label* label);
        Bge* createBge(Val* cond1, Val* cond2, Label* label);
        Blt* createBlt(Val* cond1, Val* cond2, Label* label);
        FBeq* createFBeq(Val* cond1, Val* cond2, Label* label);
        FBge* createFBge(Val* cond1, Val* cond2, Label* label);
        FBgt* createFBgt(Val* cond1, Val* cond2, Label* label);
        FBle* createFBle(Val* cond1, Val* cond2, Label* label);
        FBlt* createFBlt(Val* cond1, Val* cond2, Label* label);
        FBne* createFBne(Val* cond1, Val* cond2, Label* label);
        Jump* createJump(Label* label);
        Ret* createRet();

        CallerSaveRegs* createCallerSaveRegs(::std::vector<::std::pair<IRA*, IRIA*>> caller_iregs_save);
        CallerSaveRegs* createCallerSaveRegs(::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_save);
        CalleeSaveRegs* createCalleeSaveRegs(::std::vector<::std::pair<IRA*, IRIA*>> callee_iregs_save);
        CalleeSaveRegs* createCalleeSaveRegs(::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_save);
        CallerRestoreRegs* createCallerRestoreRegs(::std::vector<::std::pair<IRA*, IRIA*>> caller_iregs_restore);
        CallerRestoreRegs* createCallerRestoreRegs(::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_restore);
        CalleeRestoreRegs* createCalleeRestoreRegs(::std::vector<::std::pair<IRA*, IRIA*>> callee_iregs_restore);
        CalleeRestoreRegs* createCalleeRestoreRegs(::std::vector<::std::pair<FRA*, IRIA*>> callee_fregs_restore);
        CallerParaPass* createCallerParaPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> caller_iparas_pass, ::std::vector<::std::pair<AddressMode*, AddressMode*>> caller_fparas_pass);
        CalleeParaPass* createCalleeParaPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> callee_iparas_pass, ::std::vector<::std::pair<AddressMode*, AddressMode*>> callee_fparas_pass);
        CallerSaveResult* createCallerSaveResult(GReg* grs, AddressMode* dst);
        CallerSaveResult* createCallerSaveResult(FReg* frs, AddressMode* dst);
        CalleeSaveResult* createCalleeSaveResult(IRA* idst, Val* src);
        CalleeSaveResult* createCalleeSaveResult(FRA* fdst, Val* src);
        CalleeStackFrameInitialize* createCalleeStackFrameInitialize(int stack_initial_size);
        CalleeStackFrameClear* createCalleeStackFrameClear(int stack_size_now);
        CalleeStackFrameExpand* createCalleeStackFrameExpand(int stack_size_expand);
        CalleeStackFrameShrink* create(int stack_size_shrink);
        LoadTmpRegs* createLoadTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_load);
        LoadTmpRegs* createLoadTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_load);
        StoreTmpRegs* createStoreTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_store);
        StoreTmpRegs* createStoreTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_store);
        AllocaTmpRegs* createAllocaTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_load, ::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_store);
        AllocaTmpRegs* createAllocaTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_load, ::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_store);
        InitializeAllTempRegs* createInitializeAllTempRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_restore);
        InitializeAllTempRegs* createInitializeAllTempRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_restore);
        PhiPass* createPhiPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> i_phi, ::std::vector<::std::pair<AddressMode*, AddressMode*>> f_phi);
    private:
        BasicBlock* bb;
        Subroutine* parent;
        Label* label;
        ::std::vector<AsmInst*> insts;
};


//子程序
class Subroutine{
    public:
        Subroutine(AsmUnit* parent, Function* func): parent(parent), func(func) {}
        Function* getFuncOfSubroutine() {return func;}
        void addSequence(BasicBlock* bb, Label* label){
            Sequences.push_back(new Sequence(this, bb, label));
        }
        ::std::string print();

    private:
        Function* func;
        AsmUnit *parent;
        ::std::vector<Sequence*> Sequences;


};

//汇编单元（一个源程序对应的一个汇编源程序文件的内容就是一个汇编单元）
//设计思想：总之就是得到汇编源程序字面量（string）,.data、.bss可以直接得到，只是.text需要自己构造
class AsmUnit{
    public:
        AsmUnit(Module* module): module(module){}
        Module* getModuleOfAsmUnit() {return module;}
        void addSubroutine( Function* func){
            Subroutines.push_back(new Subroutine(this, func));
        }
        ::std::string print();

    private:
        ::std::string asm_context;                  //汇编源程序字面量
        Module *module;                             //基于IR的Module
        ::std::vector<Subroutine*> Subroutines;     //一个AsmUnit包括多个子程序Subroutine
};













class AsmInst{
    public:
        enum class Op{
            //I
            add = 0,
            subw,
            sraw,
            sllw,
            srlw,
            sra,
            sll,
            srl,
            land,
            zext,
            snez,
            seqz,
            lw,
            lw_label,
            sw,
            sw_label,
            call,
            la,
            mv,
            beq,
            bne,
            bge,
            blt,
            j,
            ret,
            memset,

            //F
            fadd_s,
            fsub_s,
            fmul_s,
            fdiv_s,
            fcvt_w_s,
            fcvt_s_w,
            feq_s,
            fle_s,
            flt_s,
            fge_s,//编译伪指令
            fgt_s,//编译伪指令
            fne_s,//编译伪指令
            flw,
            flw_label,
            fsw,
            fsw_label,
            fbeq,
            fbge,
            fbgt,
            fble,
            fblt,
            fbne,

            //M
            mulw,
            divw,
            remw,
            mul64,//编译伪指令

            //编译伪指令
            //现场保护
            caller_save_regs,//调用函数负责保存的寄存器
            callee_save_regs,//被调用函数负责保存的寄存器
            
            //现场恢复
            caller_restore_regs,//恢复调用者保存的寄存器
            callee_restore_regs,//恢复被调用者保存的寄存器

            //被调者的堆栈控制
            callee_stack_frame_initialize,//被调用者的堆栈帧的初始化
            callee_stack_frame_clear,//清理被调用者的堆栈帧
            callee_stack_frame_expand,//扩展被调者的堆栈空间
            callee_stack_frame_shrink,//缩小被调者的堆栈空间

            //参数传递
            caller_parameters_passing,//将参数移动到调用者的适当位置
            callee_parameters_passing,//将参数移动到被调用者的适当位置
            caller_save_result,//保存调用结果
            callee_save_result,//被调用者保存调用结果

            //临时寄存器控制
            load_tmp_regs,//加载值到临时寄存器
            store_tmp_regs,//存储临时寄存器的值
            alloca_tmp_regs,//分配临时寄存器并保存各寄存器对应的初始值
            initialize_all_temp_regs,//恢复所有临时寄存器

            //phi节点处理
            phi_passing //移动PHI节点的数据

        };
    public:
        AsmInst(Op id, Sequence* seq):id(id), seq(seq){}
        virtual ~AsmInst() = default;
        Sequence* getSeq(){return seq;}
        Subroutine* getSub(){return seq->getSubroutineOfSeq();}
        virtual ::std::string print() = 0;

    private:
        Op id;
        Sequence* seq; 
};

//I

//
class Add: public AsmInst{
    public:
        Add(GReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::add, seq){} 
        ::std::string print() final;

    private:
        GReg* rd;
        Val* rs1;
        Val* rs2;
};

class Subw: public AsmInst{
    public:
        Subw(GReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::subw, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        Val* rs2;
};

class Mulw: public AsmInst{
    public:
        Mulw(GReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::mulw, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        Val* rs2;
};

class Muld: public AsmInst{
    public:
        Muld(GReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::mul64, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        Val* rs2;
};

class Divw: public AsmInst{
    public:
        Divw(GReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::divw, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        Val* rs2;
};


class Remw: public AsmInst{
    public:
        Remw(GReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::remw, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        Val* rs2;
};

class Sraw: public AsmInst{
    public:
        Sraw(GReg* rd,Val* rs1,IConst* iconst_rs2, Sequence* seq)
        :rd(rd), rs1(rs1), iconst_rs2(iconst_rs2), AsmInst(Op::sraw, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        IConst* iconst_rs2;
};

class Sllw: public AsmInst{
    public:
        Sllw(GReg* rd,Val* rs1,IConst* iconst_rs2, Sequence* seq)
        :rd(rd), rs1(rs1), iconst_rs2(iconst_rs2), AsmInst(Op::sllw, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        IConst* iconst_rs2;
};

class Srlw: public AsmInst{
    public:
        Srlw(GReg* rd,Val* rs1,IConst* iconst_rs2, Sequence* seq)
        :rd(rd), rs1(rs1), iconst_rs2(iconst_rs2), AsmInst(Op::srlw, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        IConst* iconst_rs2;
};

class Sra: public AsmInst{
    public:
        Sra(GReg* rd,Val* rs1,IConst* iconst_rs2, Sequence* seq)
        :rd(rd), rs1(rs1), iconst_rs2(iconst_rs2), AsmInst(Op::sra, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        IConst* iconst_rs2;
};


class Sll: public AsmInst{
    public:
        Sll(GReg* rd,Val* rs1,IConst* iconst_rs2, Sequence* seq)
        :rd(rd), rs1(rs1), iconst_rs2(iconst_rs2), AsmInst(Op::sll, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        IConst* iconst_rs2;
};

class Srl: public AsmInst{
    public:
        Srl(GReg* rd,Val* rs1,IConst* iconst_rs2, Sequence* seq)
        :rd(rd), rs1(rs1), iconst_rs2(iconst_rs2), AsmInst(Op::srl, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
        IConst* iconst_rs2;
};

class Land: public AsmInst{
    public:
        Land(GReg* rd,GReg* rs1,IConst* iconst_rs2, Sequence* seq)
        :rd(rd), rs1(rs1), iconst_rs2(iconst_rs2), AsmInst(Op::land, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        GReg* rs1;
        IConst* iconst_rs2;
};

class Fadd_s: public AsmInst{
    public:
        Fadd_s(FReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::fadd_s, seq){} 
        ::std::string print() final;
        
    private:
        FReg* rd;
        Val* rs1;
        Val* rs2;
};

class Fsub_s: public AsmInst{
    public:
        Fsub_s(FReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::fsub_s, seq){} 
        ::std::string print() final;
        
    private:
        FReg* rd;
        Val* rs1;
        Val* rs2;
};

class Fmul_s: public AsmInst{
    public:
        Fmul_s(FReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::fmul_s, seq){} 
        ::std::string print() final;
        
    private:
        FReg* rd;
        Val* rs1;
        Val* rs2;
};

class Fdiv_s: public AsmInst{
    public:
        Fdiv_s(FReg* rd,Val* rs1,Val* rs2, Sequence* seq)
        :rd(rd), rs1(rs1), rs2(rs2), AsmInst(Op::fdiv_s, seq){} 
        ::std::string print() final;
        
    private:
        FReg* rd;
        Val* rs1;
        Val* rs2;
};


class Fcvt_w_s: public AsmInst{
    public:
        Fcvt_w_s(GReg* rd,Val* rs1, Sequence* seq)
        :rd(rd), rs1(rs1), AsmInst(Op::fcvt_w_s, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* rs1;
       
};

class Fcvt_s_w: public AsmInst{
    public:
        Fcvt_s_w(FReg* rd,Val* rs1, Sequence* seq)
        :rd(rd), rs1(rs1), AsmInst(Op::fcvt_s_w, seq){} 
        ::std::string print() final;
        
    private:
        FReg* rd;
        Val* rs1;
       
};

class Zext: public AsmInst{
    public:
        Zext(GReg* rd,GReg* rs1, Sequence* seq)
        :rd(rd), rs1(rs1), AsmInst(Op::zext, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        GReg* rs1;
       
};

class Snez: public AsmInst{
    public:
        Snez(GReg* rd,Val* cond, Sequence* seq)
        :rd(rd), cond(cond), AsmInst(Op::snez, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* cond;
       
};

class Seqz: public AsmInst{
    public:
        Seqz(GReg* rd,Val* cond, Sequence* seq)
        :rd(rd), cond(cond), AsmInst(Op::seqz, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* cond;
       
};


class Feq_s: public AsmInst{
    public:
        Feq_s(GReg* rd,Val* cond1,Val* cond2, Sequence* seq)
        :rd(rd), cond1(cond1), cond2(cond2), AsmInst(Op::feq_s, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* cond1;
        Val* cond2;
};

class Fle_s: public AsmInst{
    public:
        Fle_s(GReg* rd,Val* cond1,Val* cond2, Sequence* seq)
        :rd(rd), cond1(cond1), cond2(cond2), AsmInst(Op::fle_s, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* cond1;
        Val* cond2;
};

class Flt_s: public AsmInst{
    public:
        Flt_s(GReg* rd,Val* cond1,Val* cond2, Sequence* seq)
        :rd(rd), cond1(cond1), cond2(cond2), AsmInst(Op::flt_s, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* cond1;
        Val* cond2;
};

class Fgt_s: public AsmInst{
    public:
        Fgt_s(GReg* rd,Val* cond1,Val* cond2, Sequence* seq)
        :rd(rd), cond1(cond1), cond2(cond2), AsmInst(Op::fgt_s, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* cond1;
        Val* cond2;
};

class Fge_s: public AsmInst{
    public:
        Fge_s(GReg* rd,Val* cond1,Val* cond2, Sequence* seq)
        :rd(rd), cond1(cond1), cond2(cond2), AsmInst(Op::fge_s, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* cond1;
        Val* cond2;
};


class Fne_s: public AsmInst{
    public:
        Fne_s(GReg* rd,Val* cond1,Val* cond2, Sequence* seq)
        :rd(rd), cond1(cond1), cond2(cond2), AsmInst(Op::fne_s, seq){} 
        ::std::string print() final;
        
    private:
        GReg* rd;
        Val* cond1;
        Val* cond2;
};

class Lw: public AsmInst{
    public:
        Lw(GReg* rd, GReg* base, Val*offset, Sequence* seq)
        :rd(rd), base(base), offset(offset), AsmInst(Op::lw, seq){}
        ::std::string print() final;
    private:
        GReg* rd;
        GReg* base;
        Val* offset;
};


class Lw_label: public AsmInst{
    public:
        Lw_label(GReg* rd, Label* label, Sequence* seq)
        :rd(rd), label(label), AsmInst(Op::lw_label, seq){}
        ::std::string print() final;
    private:
        GReg* rd;
        Label* label;

};

class Flw: public AsmInst{
    public:
        Flw(FReg* rd, GReg* base, Val*offset, Sequence* seq)
        :rd(rd), base(base), offset(offset), AsmInst(Op::flw, seq){}
        ::std::string print() final;
    private:
        FReg* rd;
        GReg* base;
        Val* offset;
};


class Flw_label: public AsmInst{
    public:
        Flw_label(FReg* rd, Label* label, Sequence* seq)
        :rd(rd), label(label), AsmInst(Op::flw_label, seq){}
        ::std::string print() final;
    private:
        FReg* rd;
        Label* label;

};

class Sw: public AsmInst{
    public:
        Sw(Val* src, GReg* base, Val*offset, Sequence* seq)
        :src(src), base(base), offset(offset), AsmInst(Op::sw, seq){}
        ::std::string print() final;
    private:
        Val* src;
        GReg* base;
        Val* offset;
};


class Sw_label: public AsmInst{
    public:
        Sw_label(Val* src, Label* label, Sequence* seq)
        :src(src), label(label), AsmInst(Op::sw_label, seq){}
        ::std::string print() final;
    private:
        Val* src;
        Label* label;

};

class Fsw: public AsmInst{
    public:
        Fsw(Val* src, GReg* base, Val*offset, Sequence* seq)
        :src(src), base(base), offset(offset), AsmInst(Op::fsw, seq){}
        ::std::string print() final;
    private:
        Val* src;
        GReg* base;
        Val* offset;
};


class Fsw_label: public AsmInst{
    public:
        Fsw_label(Val* src, Label* label, Sequence* seq)
        :src(src), label(label), AsmInst(Op::fsw_label, seq){}
        ::std::string print() final;
    private:
        Val* src;
        Label* label;

};


/*中端已有，后端暂时不需要
class Imemset: public AsmInst{
    public: 
        Imemset(IRIA* base, int size, int block_size, Sequence* seq)
        :base(base), size(size), block_size(block_size), AsmInst(Op::memset, seq){}
        ::std::string print() final;
    private:
        IRIA* base;
        int size;
        int block_size;
};


class Fmemset: public AsmInst{
    public: 
        Fmemset(FRIA* base, int size, int block_size, Sequence* seq)
        :base(base), size(size), block_size(block_size), AsmInst(Op::memset, seq){}
        ::std::string print() final;
    private:
        FRIA* base;
        int size;
        int block_size;
};
*/

class Call: public AsmInst{
    public:
        Call(Label* label, Sequence* seq)
        :label(label), AsmInst(Op::call, seq){}
        ::std::string print() final;

    private:
        Label* label;
};

class La: public AsmInst{
    public:
        La(GReg* rd, Label* label, Sequence* seq)
        :rd(rd), label(label), AsmInst(Op::la, seq){}
        ::std::string print() final;
    private:
        GReg* rd;
        Label* label;
};

class Mv: public AsmInst{
    public:
        Mv(GReg* rd, GReg* rs1, Sequence* seq)
        :rd(rd), rs1(rs1), AsmInst(Op::mv, seq){}
        ::std::string print() final;
    private:
        GReg* rd;
        GReg* rs1;

};

class Beq: public AsmInst{
    public:
        Beq(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::beq, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};

class Bne: public AsmInst{
    public:
        Bne(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::bne, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};

class Bge: public AsmInst{
    public:
        Bge(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::bge, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};
class Blt: public AsmInst{
    public:
        Blt(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::blt, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};

class FBeq: public AsmInst{
    public:
        FBeq(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::fbeq, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};

class FBge: public AsmInst{
    public:
        FBge(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::fbge, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};
class FBgt: public AsmInst{
    public:
        FBgt(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::fbgt, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};

class FBle: public AsmInst{
    public:
        FBle(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::fble, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};

class FBlt: public AsmInst{
    public:
        FBlt(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::fblt, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};

class FBne: public AsmInst{
    public:
        FBne(Val* cond1, Val* cond2, Label* label, Sequence* seq)
        :cond1(cond1), cond2(cond2), label(label), AsmInst(Op::fbne, seq){}
        ::std::string print() final;
    private:
        Val* cond1;
        Val* cond2;
        Label* label;
};

class Jump: public AsmInst{
    public:
        Jump(Label* label, Sequence* seq)
        :label(label), AsmInst(Op::j, seq){}

        ::std::string print() final;
    private:
        Label* label;

};

class Ret: public AsmInst{
    public:
        Ret(Sequence* seq)
        :AsmInst(Op::ret, seq){}
        ::std::string print() final;


};

class CallerSaveRegs: public AsmInst{
    public:
        CallerSaveRegs(::std::vector<::std::pair<IRA*, IRIA*>> caller_iregs_save, Sequence* seq)
        :caller_iregs_save(caller_iregs_save), AsmInst(Op::caller_save_regs, seq){}
        CallerSaveRegs(::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_save, Sequence* seq)
        :caller_fregs_save(caller_fregs_save), AsmInst(Op::caller_save_regs, seq){}
        ::std::string print() final;
    private:
        ::std::vector<::std::pair<IRA*, IRIA*>> caller_iregs_save;
        ::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_save;
};

class CalleeSaveRegs: public AsmInst{
    public:
        CalleeSaveRegs(::std::vector<::std::pair<IRA*, IRIA*>> callee_iregs_save, Sequence* seq)
        :callee_iregs_save(callee_iregs_save), AsmInst(Op::callee_save_regs, seq){}
        CalleeSaveRegs(::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_save, Sequence* seq)
        :callee_fregs_save(callee_fregs_save), AsmInst(Op::callee_save_regs, seq){}
        ::std::string print() final;
    private:
        ::std::vector<::std::pair<IRA*, IRIA*>> callee_iregs_save;
        ::std::vector<::std::pair<FRA*, IRIA*>> callee_fregs_save;
};

class CallerRestoreRegs: public AsmInst{
    public:
        CallerRestoreRegs(::std::vector<::std::pair<IRA*, IRIA*>> caller_iregs_restore, Sequence* seq)
        :caller_iregs_restore(caller_iregs_restore), AsmInst(Op::caller_restore_regs, seq){}
        CallerRestoreRegs(::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_restore, Sequence* seq)
        :caller_fregs_restore(caller_fregs_restore), AsmInst(Op::caller_restore_regs, seq){}
        ::std::string print() final;
    private:
        ::std::vector<::std::pair<IRA*, IRIA*>> caller_iregs_restore;
        ::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_restore; 
};

class CalleeRestoreRegs: public AsmInst{
    public:
        CalleeRestoreRegs(::std::vector<::std::pair<IRA*, IRIA*>> callee_iregs_restore, Sequence* seq)
        :callee_iregs_restore(callee_iregs_restore), AsmInst(Op::callee_restore_regs, seq){}
        CalleeRestoreRegs(::std::vector<::std::pair<FRA*, IRIA*>> callee_fregs_restore, Sequence* seq)
        :callee_fregs_restore(callee_fregs_restore), AsmInst(Op::callee_restore_regs, seq){}
        ::std::string print() final;
    private:
        ::std::vector<::std::pair<IRA*, IRIA*>> callee_iregs_restore;
        ::std::vector<::std::pair<FRA*, IRIA*>> callee_fregs_restore; 
};

class CallerParaPass: public AsmInst{
    public:
        CallerParaPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> caller_iparas_pass, ::std::vector<::std::pair<AddressMode*, AddressMode*>> caller_fparas_pass, Sequence* seq)
        :caller_iparas_pass(caller_iparas_pass), caller_fparas_pass(caller_fparas_pass), AsmInst(Op::caller_parameters_passing, seq){}
        ::std::string print() final;

    private:
        ::std::vector<::std::pair<AddressMode*, AddressMode*>> caller_iparas_pass;
        ::std::vector<::std::pair<AddressMode*, AddressMode*>> caller_fparas_pass;         
};

class CalleeParaPass: public AsmInst{
    public:
        CalleeParaPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> callee_iparas_pass, ::std::vector<::std::pair<AddressMode*, AddressMode*>> callee_fparas_pass, Sequence* seq)
        :callee_iparas_pass(callee_iparas_pass), callee_fparas_pass(callee_fparas_pass), AsmInst(Op::callee_parameters_passing, seq){}
        ::std::string print() final;

    private:
        ::std::vector<::std::pair<AddressMode*, AddressMode*>> callee_iparas_pass;
        ::std::vector<::std::pair<AddressMode*, AddressMode*>> callee_fparas_pass;         
};

class CallerSaveResult: public AsmInst{
    public:
        CallerSaveResult(GReg* grs, AddressMode* dst, Sequence* seq)
        :grs(grs), dst(dst), AsmInst(Op::caller_save_result, seq){}
        CallerSaveResult(FReg* frs, AddressMode* dst, Sequence* seq)
        :frs(frs), dst(dst), AsmInst(Op::caller_save_result, seq){}
        ::std::string print() final;

    private:
        GReg* grs=nullptr;
        FReg* frs=nullptr;
        AddressMode* dst;
};

class CalleeSaveResult: public AsmInst{
    public:
        CalleeSaveResult(IRA* idst, Val* src, Sequence* seq)
        :idst(idst), src(src), AsmInst(Op::callee_save_result, seq){}
        CalleeSaveResult(FRA* fdst, Val* src, Sequence* seq)
        :fdst(fdst), src(src), AsmInst(Op::callee_save_result, seq){}
        ::std::string print() final;

    private:
        IRA* idst=nullptr;
        FRA* fdst=nullptr;
        Val* src;
};

class CalleeStackFrameInitialize: public AsmInst{
    public:
        CalleeStackFrameInitialize(int stack_initial_size, Sequence* seq)
        :stack_initial_size(stack_initial_size), AsmInst(Op::callee_stack_frame_initialize, seq){}
        ::std::string print() final;
    private:
        int stack_initial_size;
};

class CalleeStackFrameClear: public AsmInst{
    public:
        CalleeStackFrameClear(int stack_size_now, Sequence* seq)
        :stack_size_now(stack_size_now), AsmInst(Op::callee_stack_frame_clear, seq){}
        ::std::string print() final;
    private:
        int stack_size_now;
};

class CalleeStackFrameExpand: public AsmInst{
    public:
        CalleeStackFrameExpand(int stack_size_expand, Sequence* seq)
        :stack_size_expand(stack_size_expand), AsmInst(Op::callee_stack_frame_expand, seq){}
        ::std::string print() final;       
    private:
        int stack_size_expand;

};

class CalleeStackFrameShrink: public AsmInst{
    public:
        CalleeStackFrameShrink(int stack_size_shrink, Sequence* seq)
        :stack_size_shrink(stack_size_shrink), AsmInst(Op::callee_stack_frame_shrink, seq){}
        ::std::string print() final;       
    private:
        int stack_size_shrink;

};

class LoadTmpRegs: public AsmInst{
    public:
        LoadTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_load, Sequence* seq)
        :iregs_tmp_load(iregs_tmp_load), AsmInst(Op::load_tmp_regs, seq){}
        LoadTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_load, Sequence* seq)
        :fregs_tmp_load(fregs_tmp_load), AsmInst(Op::load_tmp_regs, seq){}
        ::std::string print() final;  
    private:
        ::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_load;
        ::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_load;  
};

class StoreTmpRegs: public AsmInst{
    public:
        StoreTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_store, Sequence* seq)
        :iregs_tmp_store(iregs_tmp_store), AsmInst(Op::store_tmp_regs, seq){}
        StoreTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_store, Sequence* seq)
        :fregs_tmp_store(fregs_tmp_store), AsmInst(Op::store_tmp_regs, seq){}
        ::std::string print() final;  

    private:
        ::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_store;
        ::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_store;  
};

class AllocaTmpRegs: public AsmInst{
    public:
        AllocaTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_load, ::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_store, Sequence* seq)
        :iregs_tmp_load(iregs_tmp_load), iregs_tmp_store(iregs_tmp_store), AsmInst(Op::alloca_tmp_regs, seq){}
        AllocaTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_load, ::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_store, Sequence* seq)
        :fregs_tmp_load(fregs_tmp_load), fregs_tmp_store(fregs_tmp_store), AsmInst(Op::alloca_tmp_regs, seq){}

        ::std::string print() final;
    private:
        ::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_load;
        ::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_load;  

        ::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_store;
        ::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_store; 


};

class InitializeAllTempRegs: public AsmInst{
    public:
        InitializeAllTempRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_restore, Sequence* seq)
        :iregs_tmp_restore(iregs_tmp_restore), AsmInst(Op::initialize_all_temp_regs, seq){}
        InitializeAllTempRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_restore, Sequence* seq)
        :fregs_tmp_restore(fregs_tmp_restore), AsmInst(Op::initialize_all_temp_regs, seq){}
        ::std::string print() final;
    private:
        ::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_restore;
        ::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_restore;

};

class PhiPass: public AsmInst{
    public:
        PhiPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> i_phi, ::std::vector<::std::pair<AddressMode*, AddressMode*>> f_phi, Sequence* seq)
        :i_phi(i_phi), f_phi(f_phi), AsmInst(Op::phi_passing, seq){}
        ::std::string print() final;
    private:
        ::std::vector<::std::pair<AddressMode*, AddressMode*>> i_phi;
        ::std::vector<::std::pair<AddressMode*, AddressMode*>> f_phi;
};

#endif