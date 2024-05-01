#ifndef HASMVAL_HPP
#define HASMVAL_HPP

#include <string>

//& 定义RV寄存器的别名 
//重构时，此处常量定义换成枚举类

const int reg_zero = 0;      //硬编码0
const int reg_ra = 1;        //返回地址
const int reg_sp = 2;        //栈指针
const int reg_gp = 3;        //全局指针（通用指针）
const int reg_tp = 4;        //线程指针
const int reg_t0 = 5;        //临时寄存器
const int reg_t1 = 6;        //临时寄存器
const int reg_t2 = 7;        //临时寄存器
const int reg_s0 = 8;        //需要保存的寄存器
const int reg_fp = 8;        //帧指针
const int reg_s1 = 9;        //需要保存的寄存器
const int reg_a0 = 10;       //函数参数或返回值寄存器
const int reg_a1 = 11;       //函数参数或返回值寄存器   
const int reg_a2 = 12;       //函数传递参数寄存器
const int reg_a3 = 13;       //函数传递参数寄存器
const int reg_a4 = 14;       //函数传递参数寄存器
const int reg_a5 = 15;       //函数传递参数寄存器
const int reg_a6 = 16;       //函数传递参数寄存器
const int reg_a7 = 17;       //函数传递参数寄存器
const int reg_s2 = 18;       //需要保存的寄存器
const int reg_s3 = 19;       //需要保存的寄存器
const int reg_s4 = 20;       //需要保存的寄存器
const int reg_s5 = 21;       //需要保存的寄存器
const int reg_s6 = 22;       //需要保存的寄存器
const int reg_s7 = 23;       //需要保存的寄存器
const int reg_s8 = 24;       //需要保存的寄存器
const int reg_s9 = 25;       //需要保存的寄存器
const int reg_s10 = 26;       //需要保存的寄存器
const int reg_s11 = 27;       //需要保存的寄存器
const int reg_t3 = 28;      //临时寄存器
const int reg_t4 = 29;      //临时寄存器
const int reg_t5 = 30;      //临时寄存器
const int reg_t6 = 31;      //临时寄存器

const int reg_ft0 = 0;     //浮点临时寄存器
const int reg_ft1 = 1;     //浮点临时寄存器
const int reg_ft2 = 2;     //浮点临时寄存器
const int reg_ft3 = 3;     //浮点临时寄存器
const int reg_ft4 = 4;     //浮点临时寄存器
const int reg_ft5 = 5;     //浮点临时寄存器
const int reg_ft6 = 6;     //浮点临时寄存器
const int reg_ft7 = 7;     //浮点临时寄存器
const int reg_fs0 = 8;     //需要保存的浮点寄存器
const int reg_fs1 = 9;     //需要保存的浮点寄存器
const int reg_fa0 = 10;    //浮点函数参数或返回值寄存器
const int reg_fa1 = 11;    //浮点函数参数或返回值寄存器
const int reg_fa2 = 12;    //浮点函数参数
const int reg_fa3 = 13;    //浮点函数参数
const int reg_fa4 = 14;    //浮点函数参数
const int reg_fa5 = 15;    //浮点函数参数
const int reg_fa6 = 16;    //浮点函数参数
const int reg_fa7 = 17;    //浮点函数参数
const int reg_fs2 = 18;     //需要保存的浮点寄存器
const int reg_fs3 = 19;     //需要保存的浮点寄存器
const int reg_fs4 = 20;     //需要保存的浮点寄存器
const int reg_fs5 = 21;     //需要保存的浮点寄存器
const int reg_fs6 = 22;     //需要保存的浮点寄存器
const int reg_fs7 = 23;     //需要保存的浮点寄存器
const int reg_fs8 = 24;     //需要保存的浮点寄存器
const int reg_fs9 = 25;     //需要保存的浮点寄存器
const int reg_fs10 = 26;     //需要保存的浮点寄存器
const int reg_fs11 = 27;     //需要保存的浮点寄存器
const int reg_ft8 = 28;     //浮点临时寄存器
const int reg_ft9 = 29;     //浮点临时寄存器
const int reg_ft10 = 30;     //浮点临时寄存器
const int reg_ft11 = 31;     //浮点临时寄存器


//重构时，查找表换成map
const std::string Ireg2name[32] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "fp", "s1", 
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3",
    "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", 
    "t5", "t6" 
};

const std::string Freg2name[32] = {
    "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "fs0", "fs1", 
    "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", "fs2", "fs3",
    "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", 
    "ft10", "ft11" 
};

class HAsmVal {
public:
    virtual bool is_reg() = 0;
    virtual bool is_const() = 0;
    virtual std::string print() = 0;
    virtual std::string get_asm_code() = 0;
};

class Reg: public HAsmVal {
public:
    Reg(int id, bool is_fp): id_(id), is_fp_(is_fp) {}
public:
    int get_id() { return id_; }
    bool is_reg() final { return true; }
    bool is_const() final { return false; }
    bool is_float() { return is_fp_; }
    std::string print();
    std::string get_asm_code();

private:
    int id_;
    bool is_fp_;
};

class Mem: public HAsmVal {
public:
    Mem(int id, int offset): id_(id), offset_(offset) {}

public:
    int get_offset() { return offset_; }
    int get_reg_id() { return id_; }
    bool is_reg() final { return false; }
    bool is_const() final { return false; }
    std::string print();
    std::string get_asm_code();
private:
    int id_;
    int offset_;
};

class Const: public HAsmVal {
public:
    Const(int ival): ival_(ival), is_fp_(false) {}
    Const(float fval): fval_(fval), is_fp_(true) {}
    bool is_const() final { return true; }
    bool is_reg() final { return false; }
    int &get_ival() { return ival_; }
    float &get_fval() { return fval_; }
    std::string print();
    std::string get_asm_code();
private:
    int ival_;
    float fval_;
    bool is_fp_;
};

#endif