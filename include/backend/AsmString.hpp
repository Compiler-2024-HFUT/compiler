//该文件里写ASM的打印
#ifndef ASMPRINT_HPP
#define ASMPRINT_HPP

#include <string>

class AsmString{
    public:
        //单个双引号
        static const ::std::string dq;
        //空格
        static const ::std::string space;
        //换行
        static const ::std::string newline;
        //逗号
        static const ::std::string comma;
        //冒号
        static const ::std::string colon;

};


//指令字符串生成模板
//op rd, rs1
#define DR(op, rd, rs1) (AsmString::space + (op) + AsmString::space + (rd)->print() + AsmString::comma +  (rs1)->print() + AsmString::newline)
//op rd, imm
#define DRI(op, rd, imm) (AsmString::space + (op) + AsmString::space + (rd)->print() + AsmString::comma +  ::std::to_string(imm) + AsmString::newline)
//op, rd, rs1, rs2
#define TR(op, rd, rs1, rs2) (AsmString::space + (op) + AsmString::space + (rd)->print() + AsmString::comma + (rs1)->print() + AsmString::comma+ (rs2)->print() + AsmString::newline) 
//op, rd, rs1, imm
#define TRI(op, rd, rs1, imm) (AsmString::space + (op) + AsmString::space + (rd)->print() + AsmString::comma + (rs1)->print() + AsmString::comma+ ::std::to_string(imm) + AsmString::newline)
//op, rs1, rs2, label
#define B(op, rs1, rs2, label) (AsmString::space + (op) + AsmString::space + (rs1)->print() + AsmString::comma + (rs2)->print() + AsmString::comma + (label)->print() + AsmString::newline)
//op, reg, offset(base)
#define LS(op, reg, base, offset) (AsmString::space + (op) + AsmString::space + (reg)->print() + AsmString::comma + ::std::to_string(offset) +"(" + (base)->print() + ")" + AsmString::newline)
//j label
#define J(op, label) (AsmString::space+(op)+AsmString::space+(label)->print()+AsmString::newline)
//la rd, label
#define LA(op, rd, Label) (AsmString::space+(op)+AsmString::space+(rd)->print()+AsmString::comma+(Label)->print()+AsmString::newline)
//ret
#define RET() (AsmString::space+"ret"+AsmString::newline)
//call label
#define CALL(label) (AsmString::space+"call"+AsmString::space+(label)->print()+AsmString::newline)

#endif