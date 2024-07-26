#ifndef RISCVINST_HPP
#define RISCVINST_HPP

#include "AsmString.hpp"
#include <cstdint>
#include <string>
class GReg;
class FReg;
class Label;
class RISCVInst{
    public:
        //---------------------I_Begin------------------------
        //2 reg 
        static ::std::string seqz(GReg *rd, GReg *rs1);
        static ::std::string snez(GReg *rd, GReg *rs1);       
        static ::std::string mv(GReg *rd, GReg *rs1);  
        static ::std::string sext_w(GReg* rd, GReg *rs1);

        //3 reg alu
        static ::std::string add(GReg* rd, GReg* rs1, GReg* rs2);
        static ::std::string addw(GReg* rd, GReg* rs1, GReg* rs2);
        static ::std::string subw(GReg* rd, GReg* rs1, GReg* rs2);
        static ::std::string and_(GReg* rd, GReg* rs1, GReg* rs2);

        //imm
        static ::std::string addi(GReg* rd, GReg* rs1, int imm);
        static ::std::string addiw(GReg* rd, GReg* rs1, int imm);
        static ::std::string andi(GReg* rd, GReg* rs1, int imm);
        static ::std::string slli(GReg* rd, GReg* rs1, int imm);
        static ::std::string slliw(GReg* rd, GReg* rs1, int imm);
        static ::std::string srli(GReg* rd, GReg* rs1, int imm);
        static ::std::string srliw(GReg* rd, GReg* rs1, int imm);
        static ::std::string srai(GReg* rd, GReg* rs1, int imm);
        static ::std::string sraiw(GReg* rd, GReg* rs1, int imm);
        static ::std::string li(GReg* rd, int imm);
        static ::std::string li(GReg* rd, uint32_t imm);//fliteral
 
        //branch
        static ::std::string j(Label *label);
        static ::std::string la(GReg *rd, Label *label);
        static ::std::string beq(GReg *rs1, GReg *rs2, Label *label);         
        static ::std::string bne(GReg *rs1, GReg *rs2, Label *label);         
        static ::std::string bge(GReg *rs1, GReg *rs2, Label *label);         
        static ::std::string blt(GReg *rs1, GReg *rs2, Label *label);

        //load store
        static ::std::string lw(GReg *rd, GReg *base, int offset);
        static ::std::string sw(GReg *rs1, GReg *base, int offset);
        static ::std::string ld(GReg *rd, GReg *base, int offset);
        static ::std::string sd(GReg *rs1, GReg *base, int offset);
 
        //call ret
        static ::std::string call(Label *label);
        static ::std::string ret();
        //---------------------I_End------------------------

        //---------------------F_Begin------------------------
        //2 reg
        static ::std::string fmv_s(FReg *rd, FReg *rs1);
        static ::std::string fmv_s_x(FReg *rd, GReg *rs1);
        static ::std::string fcvt_s_w(FReg *rd, GReg *rs1);
        static ::std::string fcvt_w_s(GReg *rd, FReg *rs1);
 
        //3 reg alu
        static ::std::string fadd_s(FReg *rd, FReg *rs1, FReg *rs2);
        static ::std::string fsub_s(FReg *rd, FReg *rs1, FReg *rs2);
        static ::std::string fmul_s(FReg *rd, FReg *rs1, FReg *rs2);
        static ::std::string fdiv_s(FReg *rd, FReg *rs1, FReg *rs2);

        //branch
        static ::std::string feq_s(GReg *rd, FReg *rs1, FReg *rs2);
        static ::std::string fle_s(GReg *rd, FReg *rs1, FReg *rs2);
        static ::std::string flt_s(GReg *rd, FReg *rs1, FReg *rs2);
 
        //load store
        static ::std::string flw(FReg *rd, GReg *base, int offset);
        static ::std::string fsw(FReg *rs1, GReg *base, int offset);
        //---------------------F_End------------------------
 
        //---------------------M_Begin------------------------
        static ::std::string mul(GReg *rd, GReg *rs1, GReg *rs2);
        static ::std::string mulw(GReg *rd, GReg *rs1, GReg *rs2);
        static ::std::string divw(GReg *rd, GReg *rs1, GReg *rs2);
        static ::std::string remw(GReg *rd, GReg *rs1, GReg *rs2);
        //---------------------M_End------------------------
        
        static ::std::string sh2add(GReg *rd, GReg *rs1, GReg *rs2);

};

#endif