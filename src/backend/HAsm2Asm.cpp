#include "backend/HAsm2Asm.hpp"
#include "backend/HAsmLoc.hpp"
#include "backend/HAsmVal.hpp"


#define __ASM_TRIPLE_REG_OP__(op, rd, rs1, rs2)         \
        (HAsm2Asm::space + (op) + HAsm2Asm::space + (rd)->get_asm_code() + ", " + (rs1)->get_asm_code() + ", "+ (rs2)->get_asm_code() + HAsm2Asm::newline) 

namespace HAsm2Asm {

//& immediate instructions

//& 外部调用保证rs不可能为s1号寄存器
std::string addi(Reg *rd, Reg *rs, int imm) {
    std::string asm_code;
    Reg* tmp_s1 = new Reg(reg_s1, false);
    if(imm <= 2047 && imm >= -2048) {
        asm_code += HAsm2Asm::space + "addi" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    } else {
        asm_code += HAsm2Asm::space + "li" + HAsm2Asm::space + tmp_s1->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
        asm_code += __ASM_TRIPLE_REG_OP__("add", rd, rs, tmp_s1);
    }
    return asm_code;
}

std::string addiw(Reg *rd, Reg *rs, int imm) {
    std::string asm_code;
    Reg* tmp_s1 = new Reg(reg_s1, false);
    if(imm <= 2047 && imm >= -2048) {
        asm_code += HAsm2Asm::space + "addiw" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    } else {
        asm_code += HAsm2Asm::li(tmp_s1, imm);
        asm_code += __ASM_TRIPLE_REG_OP__("addw", rd, rs, tmp_s1);
    }
    return asm_code;
}

std::string li(Reg *rd, int imm) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "li" + HAsm2Asm::space + rd->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    return asm_code;
}

std::string li(Reg *rd, uint32_t fliteral) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "li" + HAsm2Asm::space + rd->get_asm_code() + ", " + std::to_string(fliteral) + HAsm2Asm::newline;
    return asm_code;
}


//& 3 regs alu instructions
std::string add(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("add", rd, rs1, rs2);
    return asm_code;
}

std::string addw(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("addw", rd, rs1, rs2);
    return asm_code;
}

std::string subw(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("subw", rd, rs1, rs2);
    return asm_code;
}

std::string mulw(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("mulw", rd, rs1, rs2);
    return asm_code;
}

std::string mul(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("mul", rd, rs1, rs2);
    return asm_code;
}

std::string divw(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("divw", rd, rs1, rs2);
    return asm_code;
}


std::string remw(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("remw", rd, rs1, rs2);
    return asm_code;
}

std::string slliw(Reg *rd, Reg *rs, int imm) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "slliw" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    return asm_code;
}


std::string slli(Reg *rd, Reg *rs, int imm) {
    std::string code;
    code += HAsm2Asm::space + "slli" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    return code;
}

std::string sraiw(Reg *rd, Reg *rs, int imm) {
     std::string asm_code;
    asm_code += HAsm2Asm::space + "sraiw" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    return asm_code;
}

std::string srai(Reg *rd, Reg *rs, int imm) {
     std::string code;
    code += HAsm2Asm::space + "srai" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    return code;
}

std::string srliw(Reg *rd, Reg *rs, int imm) {
     std::string asm_code;
    asm_code += HAsm2Asm::space + "srliw" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    return asm_code;
}

std::string srli(Reg *rd, Reg *rs, int imm) {
     std::string code;
    code += HAsm2Asm::space + "srli" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm) + HAsm2Asm::newline;
    return code;
}

std::string fadds(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("fadd.s", rd, rs1, rs2);
    return asm_code;
}

std::string fsubs(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("fsub.s", rd, rs1, rs2);
    return asm_code;
}

std::string fmuls(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("fmul.s", rd, rs1, rs2);
    return asm_code;
}

std::string fdivs(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("fdiv.s", rd, rs1, rs2);
    return asm_code;
}

std::string fnegs(Reg *rd) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "fneg.s	" + HAsm2Asm::space + rd->get_asm_code() + ", " + rd->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

//& logical ops
std::string andi(Reg *rd, Reg *rs, int imm) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "andi" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + ", " + std::to_string(imm)+ HAsm2Asm::newline;
    return asm_code;
}

std::string land(Reg *rd, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("and", rd, rs1, rs2);
    return asm_code;
}

//& cmp ops
std::string seqz(Reg *rd, Reg *rs) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "seqz" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

std::string snez(Reg *rd, Reg *rs) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "snez" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

std::string feqs(Reg *rd1, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("feq.s", rd1, rs1, rs2);
    return asm_code;
}

std::string fles(Reg *rd1, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("fle.s", rd1, rs1, rs2);
    return asm_code;
}

std::string flts(Reg *rd1, Reg *rs1, Reg *rs2) {
    std::string asm_code;
    asm_code += __ASM_TRIPLE_REG_OP__("flt.s", rd1, rs1, rs2);
    return asm_code;
}

//& cond branches

std::string beq(Reg *rs1, Reg *rs2, Label *label) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "beq" + HAsm2Asm::space + rs1->get_asm_code() + ", " + rs2->get_asm_code() + ", " + label->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

std::string bne(Reg *rs1, Reg *rs2, Label *label) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "bne" + HAsm2Asm::space + rs1->get_asm_code() + ", " + rs2->get_asm_code() + ", " + label->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

std::string bge(Reg *rs1, Reg *rs2, Label *label) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "bge" + HAsm2Asm::space + rs1->get_asm_code() + ", " + rs2->get_asm_code() + ", " + label->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

std::string blt(Reg *rs1, Reg *rs2, Label *label) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "blt" + HAsm2Asm::space + rs1->get_asm_code() + ", " + rs2->get_asm_code() + ", " + label->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

//& mem ops
std::string sd(Reg *src, Reg *base, int offset) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "sd" + HAsm2Asm::space + src->get_asm_code() + ", " + std::to_string(offset) +"(" + base->get_asm_code() + ")" + HAsm2Asm::newline;
    return asm_code;
}

std::string ld(Reg *dst, Reg *base, int offset) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "ld" + HAsm2Asm::space + dst->get_asm_code() + ", " + std::to_string(offset) +"(" + base->get_asm_code() + ")" + HAsm2Asm::newline;
    return asm_code;
}

std::string sw(Reg *src, Reg *base, int offset) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "sw" + HAsm2Asm::space + src->get_asm_code() + ", " + std::to_string(offset) +"(" + base->get_asm_code() + ")" + HAsm2Asm::newline;
    return asm_code;
}

std::string lw(Reg *dst, Reg *base, int offset) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "lw" + HAsm2Asm::space + dst->get_asm_code() + ", " + std::to_string(offset) +"(" + base->get_asm_code() + ")" + HAsm2Asm::newline;
    return asm_code;
}

std::string fsw(Reg *src, Reg *base, int offset) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "fsw" + HAsm2Asm::space + src->get_asm_code() + ", " + std::to_string(offset) +"(" + base->get_asm_code() + ")" + HAsm2Asm::newline;
    return asm_code;
}

std::string flw(Reg *dst, Reg *base, int offset) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "flw" + HAsm2Asm::space + dst->get_asm_code() + ", " + std::to_string(offset) +"(" + base->get_asm_code() + ")" + HAsm2Asm::newline;
    return asm_code;
}

//& mv ops
std::string mv(Reg *dst, Reg *src) {
    std::string asm_code;
    if(dst->get_id() == src->get_id())
        return asm_code;
    asm_code += HAsm2Asm::space + "mv" + HAsm2Asm::space + dst->get_asm_code() + ", " + src->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
} 

std::string fmvs(Reg *dst, Reg *src) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "fmv.s" + HAsm2Asm::space + dst->get_asm_code() + ", " + src->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

//& cast move
std::string fmvsx(Reg *dst, Reg *src) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "fmv.s.x" + HAsm2Asm::space + dst->get_asm_code() + ", " + src->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

//& unconditional branch
std::string j(Label *label) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "j" + HAsm2Asm::space + label->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}


//& cast ops
std::string fcvtsw(Reg *dst, Reg *src) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "fcvt.s.w" + HAsm2Asm::space + dst->get_asm_code() + ", " + src->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

std::string fcvtws(Reg *dst, Reg *src) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "fcvt.w.s" + HAsm2Asm::space + dst->get_asm_code() + ", " + src->get_asm_code() + ", rtz" + HAsm2Asm::newline;
    return asm_code;
}


//& load address
std::string la(Reg *rd, Label *label) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "la" + HAsm2Asm::space + rd->get_asm_code() + ", " + label->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

//& sext
std::string sextw(Reg *rd, Reg *rs) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "sext.w" + HAsm2Asm::space + rd->get_asm_code() + ", " + rs->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

//& ret
std::string ret() {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "ret" + HAsm2Asm::newline;
    return asm_code;
}

//& call
std::string call(Label *label) {
    std::string asm_code;
    asm_code += HAsm2Asm::space + "call" + HAsm2Asm::space + label->get_asm_code() + HAsm2Asm::newline;
    return asm_code;
}

}