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
        int getIConst() {return val;}
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
        float getIConst() {return val;}
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
        int getIConstPool(){return i_const_pool;}
        ::std::string print() final;

    private:
        int i_const_pool;
};

class FConstPool: AddressMode{
    public:
        FConstPool(float f_const_pool): f_const_pool(f_const_pool){}
        bool isI(){return false;}
        int getFConstPool(){return f_const_pool;}
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

class FRIA: public AddressMode{
    public:
        FRIA(int id, int offset):reg(RISCV::id2GReg(id)), offset(offset){}
        bool isI(){return false;}
        RISCV::GPR getReg(){return reg;}
        int getOffset(){return offset;}
        ::std::string print() final;

    private:
        RISCV::GPR reg; 
        int offset;
};



class Subroutine;
class Sequence;
class Label;
class AsmInst;


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


    private:
        BasicBlock* bb;
        Subroutine* parent;
        Label* label;
        ::std::vector<AsmInst*> insts;
};

#endif