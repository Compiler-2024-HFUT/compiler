#include "backend/RISCVInst.hpp"

//---------------------I_Begin------------------------
//2 reg 
::std::string RISCVInst::seqz(GReg *rd, GReg *rs1){
    return DR("seqz", rd, rs1);
}
::std::string RISCVInst::snez(GReg *rd, GReg *rs1){
    return DR("snez", rd, rs1);
}       
::std::string RISCVInst::mv(GReg *rd, GReg *rs1){
    if(rd->getID()==rs1->getID())
        return "";
    else    
        return DR("mv", rd, rs1);
    
}  
::std::string RISCVInst::sext_w(GReg* rd, GReg *rs1){
    return DR("sext.w", rd, rs1);
}
//3 reg alu
::std::string RISCVInst::add(GReg* rd, GReg* rs1, GReg* rs2){
    return TR("add", rd, rs1, rs2);
}
::std::string RISCVInst::addw(GReg* rd, GReg* rs1, GReg* rs2){
    return TR("addw", rd, rs1, rs2);
}
::std::string RISCVInst::subw(GReg* rd, GReg* rs1, GReg* rs2){
    return TR("subw", rd, rs1, rs2);
}
::std::string RISCVInst::and_(GReg* rd, GReg* rs1, GReg* rs2){
    return TR("and", rd, rs1, rs2);
}
//imm
::std::string RISCVInst::addi(GReg* rd, GReg* rs1, int imm){
    if(imm>=-2048 && imm<=2047)
        return TRI("addi", rd, rs1, imm);
    else{
        GReg* tmp_reg = new GReg(static_cast<int>(RISCV::GPR::s1));
        return DRI("li", tmp_reg, imm)+TR("add", rd, rs1, tmp_reg);
    }
}
::std::string RISCVInst::addiw(GReg* rd, GReg* rs1, int imm){
    if(imm>=-2048 && imm<=2047)
        return TRI("addiw", rd, rs1, imm);
    else{
        GReg* tmp_reg = new GReg(static_cast<int>(RISCV::GPR::s1));
        return DRI("li", tmp_reg, imm)+TR("addw", rd, rs1, tmp_reg);
    }
}
::std::string RISCVInst::andi(GReg* rd, GReg* rs1, int imm){
    return TRI("andi", rd, rs1, imm);
}
::std::string RISCVInst::slli(GReg* rd, GReg* rs1, int imm){
    return TRI("slli", rd, rs1, imm);
}
::std::string RISCVInst::slliw(GReg* rd, GReg* rs1, int imm){
    return TRI("slliw", rd, rs1, imm);
}
::std::string RISCVInst::srli(GReg* rd, GReg* rs1, int imm){
    return TRI("srli", rd, rs1, imm);
}
::std::string RISCVInst::srliw(GReg* rd, GReg* rs1, int imm){
    return TRI("srliw", rd, rs1, imm);
}
::std::string RISCVInst::srai(GReg* rd, GReg* rs1, int imm){
    return TRI("srai", rd, rs1, imm);
}
::std::string RISCVInst::sraiw(GReg* rd, GReg* rs1, int imm){
    return TRI("sraiw", rd, rs1, imm);
}
::std::string RISCVInst::li(GReg* rd, int imm){
    return DRI("li", rd, imm);
}
::std::string RISCVInst::li(GReg* rd, uint32_t imm){
    return DRI("li", rd, imm);
}//fliteral
//branch
::std::string RISCVInst::j(Label *label){
    return J("j", label);
}
::std::string RISCVInst::la(GReg *rd, Label *label){
    return LA("la", rd, label);
}
::std::string RISCVInst::beq(GReg *rs1, GReg *rs2, Label *label){
    return B("beq", rs1, rs2, label);
}         
::std::string RISCVInst::bne(GReg *rs1, GReg *rs2, Label *label){
    return B("bne", rs1, rs2, label);
}         
::std::string RISCVInst::bge(GReg *rs1, GReg *rs2, Label *label){
    return B("bge", rs1, rs2, label);
}         
::std::string RISCVInst::blt(GReg *rs1, GReg *rs2, Label *label){
    return B("blt", rs1, rs2, label);
}
//load store
::std::string RISCVInst::lw(GReg *rd, GReg *base, int offset){
    return LS("lw", rd, base, offset);
}
::std::string RISCVInst::sw(GReg *rs1, GReg *base, int offset){
    return LS("sw", rs1, base, offset);
}
::std::string RISCVInst::ld(GReg *rd, GReg *base, int offset){
    return LS("ld", rd, base, offset);
}
::std::string RISCVInst::sd(GReg *rs1, GReg *base, int offset){
    return LS("sd", rs1, base, offset);
}
//call ret
::std::string RISCVInst::call(Label *label){
    return CALL(label);
}
::std::string RISCVInst::ret(){
    return RET();
}
//---------------------I_End------------------------
//---------------------F_Begin------------------------
//2 reg
::std::string RISCVInst::fmv_s(FReg *rd, FReg *rs1){
        if(rd->getID()==rs1->getID())
        return "";
    else    
        return DR("fmv.s", rd, rs1);
}
::std::string RISCVInst::fmv_s_x(FReg *rd, GReg *rs1){
    return DR("fmv.s.x", rd, rs1);
}
::std::string RISCVInst::fcvt_s_w(FReg *rd, GReg *rs1){
    return DR("fcvt.s.w", rd, rs1);
}
::std::string RISCVInst::fcvt_w_s(GReg *rd, FReg *rs1){
    return DR("fcvt.w.s", rd, rs1);
}
//3 reg alu
::std::string RISCVInst::fadd_s(FReg *rd, FReg *rs1, FReg *rs2){
    return TR("fadd.s", rd, rs1, rs2);
}
::std::string RISCVInst::fsub_s(FReg *rd, FReg *rs1, FReg *rs2){
    return TR("fsub.s", rd, rs1, rs2);
}
::std::string RISCVInst::fmul_s(FReg *rd, FReg *rs1, FReg *rs2){
    return TR("fmul.s", rd, rs1, rs2);
}
::std::string RISCVInst::fdiv_s(FReg *rd, FReg *rs1, FReg *rs2){
    return TR("fdiv.s", rd, rs1, rs2);
}
//branch
::std::string RISCVInst::feq_s(GReg *rd, FReg *rs1, FReg *rs2){
    return TR("feq.s", rd, rs1, rs2);
}
::std::string RISCVInst::fle_s(GReg *rd, FReg *rs1, FReg *rs2){
    return TR("fle.s", rd, rs1, rs2);
}
::std::string RISCVInst::flt_s(GReg *rd, FReg *rs1, FReg *rs2){
    return TR("flt.s", rd, rs1, rs2);
}
//load store
::std::string RISCVInst::flw(FReg *rd, GReg *base, int offset){
    return LS("flw", rd, base, offset);
}
::std::string RISCVInst::fsw(FReg *rs1, GReg *base, int offset){
    return LS("fsw", rs1, base, offset);
}
//---------------------F_End------------------------
//---------------------M_Begin------------------------
::std::string RISCVInst::mul(GReg *rd, GReg *rs1, GReg *rs2){
    return TR("mul", rd, rs1, rs2);
}
::std::string RISCVInst::mulw(GReg *rd, GReg *rs1, GReg *rs2){
    return TR("mulw", rd, rs1, rs2);
}
::std::string RISCVInst::divw(GReg *rd, GReg *rs1, GReg *rs2){
    return TR("divw", rd, rs1, rs2);
}
::std::string RISCVInst::remw(GReg *rd, GReg *rs1, GReg *rs2){
    return TR("remw", rd, rs1, rs2);
}
//---------------------M_End------------------------