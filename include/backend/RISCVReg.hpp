//该文件里写RISCV架构的定义
#ifndef RISCV_HPP
#define RISCV_HPP

#include <map>
#include <string>
#include <iostream>

using ::std::map;

class RISCV{
    public:
        //定义通用整型寄存器(General-Purpose Registers)的索引，共32个物理寄存器。
        //存储数据或地址。
        enum class GPR{
            zero = 0,   //永远是0
            ra,         //函数返回地址寄存器
            sp,         //栈指针寄存器，存储当前栈顶的地址
            gp,         //全局指针寄存器，存储全局数据区（Global Data Segment）的起始地址，用于访问全局变量和静态数据
            tp,         //线程指针寄存器，存储当前线程的指针或者线程局部存储（Thread-Local Storage, TLS）的基址
            t0,         //************
            t1,         //t0-t2，临时寄存器，存储临时变量和函数调用过程中的临时计算结果
            t2,         //************
            s0,         //帧指针fp，存储当前函数调用的栈帧的基址
            s1,         //s1-s11，保存寄存器，在函数调用中用于保存调用前的寄存器状态
            a0,         //************
            a1,
            a2,
            a3,         //a0-a7，参数寄存器，用于存储函数调用的参数，参数按照从左到右顺序依次通过a0-a7传
            a4,
            a5,
            a6,
            a7,         //************
            s2,
            s3,
            s4,
            s5,
            s6,
            s7,
            s8,
            s9,
            s10,
            s11,
            t3,         //************   
            t4,         //临时寄存器
            t5,
            t6          //************
        };

        //(Floating-Point Registers)
        enum class FPR{
            ft0 = 0,
            ft1,
            ft2,
            ft3,
            ft4,
            ft5,
            ft6,
            ft7,
            fs0,
            fs1,
            fa0,
            fa1,
            fa2,
            fa3,
            fa4,
            fa5,
            fa6,
            fa7,
            fs2,
            fs3,
            fs4,
            fs5,
            fs6,
            fs7,
            fs8,
            fs9,
            fs10,
            fs11,
            ft8,
            ft9,
            ft10,
            ft11
        };


    static inline GPR id2GReg(int reg_id){
        if(reg_id>=static_cast<int>(GPR::zero) && reg_id<=static_cast<int>(GPR::t6))
            return static_cast<GPR>(reg_id);
        else ::std::cout<<"寄存器ID非法"<<::std::endl;
    }

    static inline FPR id2FReg(int reg_id){
        if(reg_id>=static_cast<int>(FPR::ft0) && reg_id<=static_cast<int>(FPR::ft11))
            return static_cast<FPR>(reg_id);
        else ::std::cout<<"寄存器ID非法"<<::std::endl;
    }


    static inline ::std::string reg2String(GPR reg){
        int index = static_cast<int>(reg);
        switch(index){
            case 0: return "zero";
            case 1: return "ra";
            case 2: return "sp";
            case 3: return "gp";
            case 4: return "tp";
            case 5: return "t0";
            case 6: return "t1";
            case 7: return "t2";
            case 8: return "fp";
            case 9: return "s1";
            case 10: return "a0";
            case 11: return "a1";
            case 12: return "a2";
            case 13: return "a3";
            case 14: return "a4";
            case 15: return "a5";
            case 16: return "a6";
            case 17: return "a7";
            case 18: return "s2";
            case 19: return "s3";
            case 20: return "s4";
            case 21: return "s5";
            case 22: return "s6";
            case 23: return "s7";
            case 24: return "s8";
            case 25: return "s9";
            case 26: return "s10";
            case 27: return "s11";
            case 28: return "t3";
            case 29: return "t4";
            case 30: return "t5";
            case 31: return "t6";
        }
    }

    static inline ::std::string freg2String(FPR reg){
        int index = static_cast<int>(reg);
        switch(index){
            case 0: return "ft0";
            case 1: return "ft1";
            case 2: return "ft2";
            case 3: return "ft3";
            case 4: return "ft4";
            case 5: return "ft5";
            case 6: return "ft6";
            case 7: return "ft7";
            case 8: return "fs0";
            case 9: return "fs1";
            case 10: return "fa0";
            case 11: return "fa1";
            case 12: return "fa2";
            case 13: return "fa3";
            case 14: return "fa4";
            case 15: return "fa5";
            case 16: return "fa6";
            case 17: return "fa7";
            case 18: return "fs2";
            case 19: return "fs3";
            case 20: return "fs4";
            case 21: return "fs5";
            case 22: return "fs6";
            case 23: return "fs7";
            case 24: return "fs8";
            case 25: return "fs9";
            case 26: return "fs10";
            case 27: return "fs11";
            case 28: return "ft8";
            case 29: return "ft9";
            case 30: return "ft10";
            case 31: return "ft11";
        }
    }

};



#endif