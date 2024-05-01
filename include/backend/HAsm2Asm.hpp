#ifndef HASM2ASM_HPP
#define HASM2ASM_HPP

#include "HAsmVal.hpp"
#include "HAsmLoc.hpp"

#include <string>

namespace HAsm2Asm { 

    const std::string space = std::string(4, ' ');
    const std::string newline = "\n";

    //& immediate instructions
    std::string addi(Reg *rd, Reg *rs, int imm);                //& 12位立即数(-2^11, 2^11-1)

    std::string addiw(Reg *rd, Reg *rs, int imm);

    std::string li(Reg *rd, int imm);
    std::string li(Reg *rd, uint32_t fliteral);

    //& 3 regs alu instructions
    std::string add(Reg *rd, Reg *rs1, Reg *rs2);

    std::string addw(Reg *rd, Reg *rs1, Reg *rs2);
    std::string subw(Reg *rd, Reg *rs1, Reg *rs2);
    std::string mulw(Reg *rd, Reg *rs1, Reg *rs2);
    std::string mul(Reg *rd, Reg *rs1, Reg *rs2);
    std::string divw(Reg *rd, Reg *rs1, Reg *rs2);
    std::string remw(Reg *rd, Reg *rs1, Reg *rs2);

    std::string slliw(Reg *rd, Reg *rs, int imm);
    std::string sraiw(Reg *rd, Reg *rs, int imm);
    std::string srliw(Reg *rd, Reg *rs, int imm);
    std::string slli(Reg *rd, Reg *rs, int imm);
    std::string srai(Reg *rd, Reg *rs, int imm);
    std::string srli(Reg *rd, Reg *rs, int imm);

    std::string fadds(Reg *rd, Reg *rs1, Reg *rs2);
    std::string fsubs(Reg *rd, Reg *rs1, Reg *rs2);
    std::string fmuls(Reg *rd, Reg *rs1, Reg *rs2);
    std::string fdivs(Reg *rd, Reg *rs1, Reg *rs2);

    //& logical ops
    std::string andi(Reg *rd, Reg *rs, int imm);
    std::string land(Reg *rd, Reg *rs1, Reg *rs2);

    //& cmp ops
    std::string seqz(Reg *rd, Reg *rs);
    std::string snez(Reg *rd, Reg *rs);

    std::string feqs(Reg *rd1, Reg *rs1, Reg *rs2);
    std::string fles(Reg *rd1, Reg *rs1, Reg *rs2);
    std::string flts(Reg *rd1, Reg *rs1, Reg *rs2);


    //& cond branches

    std::string beq(Reg *rs1, Reg *rs2, Label *label);         //& 13位立即数(-2^12, 2^12-1)
    std::string bne(Reg *rs1, Reg *rs2, Label *label);         //& 13位立即数(-2^12, 2^12-1)
    std::string bge(Reg *rs1, Reg *rs2, Label *label);         //& 13位立即数(-2^12, 2^12-1)
    std::string blt(Reg *rs1, Reg *rs2, Label *label);         //& 13位立即数(-2^12, 2^12-1)

    //& mem ops
    std::string sd(Reg *src, Reg *base, int offset);
    std::string ld(Reg *dst, Reg *base, int offset);
    std::string sw(Reg *src, Reg *base, int offset);
    std::string lw(Reg *dst, Reg *base, int offset);

    std::string fsw(Reg *src, Reg *base, int offset);
    std::string flw(Reg *dst, Reg *base, int offset);

    //& mv ops
    std::string mv(Reg *dst, Reg *src);
    std::string fmvs(Reg *dst, Reg *src);

    //& cast move
    std::string fmvsx(Reg *dst, Reg *src);

    //& unconditional branch
    std::string j(Label *label);                                   //& 21位立即数(-2^20, 2^20-1)


    //& cast ops
    std::string fcvtsw(Reg *dst, Reg *src);
    std::string fcvtws(Reg *dst, Reg *src);

    

    //& load address
    std::string la(Reg *rd, Label *label);

    //& sext
    std::string sextw(Reg* rd, Reg *rs);

    //& ret
    std::string ret();

    //& call
    std::string call(Label *label);

}

#endif