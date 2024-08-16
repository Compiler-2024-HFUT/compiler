#include "backend/Asm.hpp"

 Add* Sequence::createAdd(GReg* rd,Val* rs1,Val* rs2){
    auto inst = new Add(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Subw* Sequence::createSubw(GReg* rd,Val* rs1,Val* rs2){
    auto inst = new Subw(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Mulw* Sequence::createMulw(GReg* rd,Val* rs1,Val* rs2){
    auto inst = new Mulw(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Muld* Sequence::createMuld(GReg* rd,Val* rs1,Val* rs2){
    auto inst = new Muld(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Divw* Sequence::createDivw(GReg* rd,Val* rs1,Val* rs2){
    auto inst = new Divw(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Remw* Sequence::createRemw(GReg* rd,Val* rs1,Val* rs2){
    auto inst = new Remw(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Sraw* Sequence::createSraw(GReg* rd,Val* rs1,IConst* iconst_rs2){
    auto inst = new Sraw(rd, rs1, iconst_rs2, this);
    insts.push_back(inst);
    return inst;
}
 Sllw* Sequence::createSllw(GReg* rd,Val* rs1,IConst* iconst_rs2){
    auto inst = new Sllw(rd, rs1, iconst_rs2, this);
    insts.push_back(inst);
    return inst;
}
 Srlw* Sequence::createSrlw(GReg* rd,Val* rs1,IConst* iconst_rs2){
    auto inst = new Srlw(rd, rs1, iconst_rs2, this);
    insts.push_back(inst);
    return inst;
}
 Sra* Sequence::createSra(GReg* rd,Val* rs1,IConst* iconst_rs2){
    auto inst = new Sra(rd, rs1, iconst_rs2, this);
    insts.push_back(inst);
    return inst;
}
 Sll* Sequence::createSll(GReg* rd,Val* rs1,IConst* iconst_rs2){
    auto inst = new Sll(rd, rs1, iconst_rs2, this);
    insts.push_back(inst);
    return inst;
}
 Srl* Sequence::createSrl(GReg* rd,Val* rs1,IConst* iconst_rs2){
    auto inst = new Srl(rd, rs1, iconst_rs2, this);
    insts.push_back(inst);
    return inst;
}
 Land* Sequence::createLand(GReg* rd,GReg* rs1,IConst* iconst_rs2){
    auto inst = new Land(rd, rs1, iconst_rs2, this);
    insts.push_back(inst);
    return inst;
}
 Fadd_s* Sequence::createFadd_s(FReg* rd,Val* rs1,Val* rs2){
    auto inst = new Fadd_s(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Fsub_s* Sequence::createFsub_s(FReg* rd,Val* rs1,Val* rs2){
    auto inst = new Fsub_s(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Fmul_s* Sequence::createFmul_s(FReg* rd,Val* rs1,Val* rs2){
    auto inst = new Fmul_s(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Fdiv_s* Sequence::createFdiv_s(FReg* rd,Val* rs1,Val* rs2){
    auto inst = new Fdiv_s(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;
}
 Fcvt_w_s* Sequence::createFcvt_w_s(GReg* rd,Val* rs1){
    auto inst = new Fcvt_w_s(rd, rs1,  this);
    insts.push_back(inst);
    return inst;
}
 Fcvt_s_w* Sequence::createFcvt_s_w(FReg* rd,Val* rs1){
    auto inst = new Fcvt_s_w(rd, rs1,  this);
    insts.push_back(inst);
    return inst;
}
 Zext* Sequence::createZext(GReg* rd,GReg* rs1){
    auto inst = new Zext(rd, rs1,  this);
    insts.push_back(inst);
    return inst;
}
 ZextIConst* Sequence::createZextIConst(GReg* rd,int rs1){
    auto inst = new ZextIConst(rd, rs1,  this);
    insts.push_back(inst);
    return inst;
}

 Snez* Sequence::createSnez(GReg* rd,Val* cond){
    auto inst = new Snez(rd, cond, this);
    insts.push_back(inst);
    return inst;
}
 Seqz* Sequence::createSeqz(GReg* rd,Val* cond){
    auto inst = new Seqz(rd,  cond, this);
    insts.push_back(inst);
    return inst;
}
 Feq_s* Sequence::createFeq_s(GReg* rd,Val* cond1,Val* cond2){
    auto inst = new Feq_s(rd, cond1, cond2, this);
    insts.push_back(inst);
    return inst;
}
 Fle_s* Sequence::createFle_s(GReg* rd,Val* cond1,Val* cond2){
    auto inst = new Fle_s(rd,  cond1, cond2,  this);
    insts.push_back(inst);
    return inst;
}
 Flt_s* Sequence::createFlt_s(GReg* rd,Val* cond1,Val* cond2){
    auto inst = new Flt_s(rd, cond1, cond2, this);
    insts.push_back(inst);
    return inst;
}
 Fgt_s* Sequence::createFgt_s(GReg* rd,Val* cond1,Val* cond2){
    auto inst = new Fgt_s(rd, cond1, cond2, this);
    insts.push_back(inst);
    return inst;
}
 Fge_s* Sequence::createFge_s(GReg* rd,Val* cond1,Val* cond2){
    auto inst = new Fge_s(rd, cond1, cond2, this);
    insts.push_back(inst);
    return inst;
}
 Fne_s* Sequence::createFne_s(GReg* rd,Val* cond1,Val* cond2){
    auto inst = new Fne_s(rd, cond1, cond2, this);
    insts.push_back(inst);
    return inst;
}
 Lw* Sequence::createLw(GReg* rd, GReg* base, Val*offset){
    auto inst = new Lw(rd, base, offset,  this);
    insts.push_back(inst);
    return inst;
}
 Lw_label* Sequence::createLw_label(GReg* rd, Label* label){
    auto inst = new Lw_label(rd, label,  this);
    insts.push_back(inst);
    return inst;
}
 Flw* Sequence::createFlw(FReg* rd, GReg* base, Val*offset){
    auto inst = new Flw(rd, base, offset, this);
    insts.push_back(inst);
    return inst;
}
 Flw_label* Sequence::createFlw_label(FReg* rd, Label* label){
    auto inst = new Flw_label(rd, label, this);
    insts.push_back(inst);
    return inst;
}
 Sw* Sequence::createSw(Val* src, GReg* base, Val*offset){
    auto inst = new Sw(src, base, offset,  this);
    insts.push_back(inst);
    return inst;
}
 Sw_label* Sequence::createSw_label(Val* src, Label* label){
    auto inst = new Sw_label(src, label, this);
    insts.push_back(inst);
    return inst;
}
 Fsw* Sequence::createFsw(Val* src, GReg* base, Val*offset){
    auto inst = new Fsw(src, base, offset, this);
    insts.push_back(inst);
    return inst;
}
 Fsw_label* Sequence::createFsw_label(Val* src, Label* label){
    auto inst = new Fsw_label(src, label, this);
    insts.push_back(inst);
    return inst;
}
 Call* Sequence::createCall(Label* label){
    auto inst = new Call(label, this);
    insts.push_back(inst);
    return inst;
}
 La* Sequence::createLa(GReg* rd, Label* label){
    auto inst = new La(rd, label, this);
    insts.push_back(inst);
    return inst;
}
 Mv* Sequence::createMv(GReg* rd, GReg* rs1){
    auto inst = new Mv(rd, rs1, this);
    insts.push_back(inst);
    return inst;
}
 Beq* Sequence::createBeq(Val* cond1, Val* cond2, Label* label){
    auto inst = new Beq(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 Bne* Sequence::createBne(Val* cond1, Val* cond2, Label* label){
    auto inst = new Bne(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 Bge* Sequence::createBge(Val* cond1, Val* cond2, Label* label){
    auto inst = new Bge(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 Blt* Sequence::createBlt(Val* cond1, Val* cond2, Label* label){
    auto inst = new Blt(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 FBeq* Sequence::createFBeq(Val* cond1, Val* cond2, Label* label){
    auto inst = new FBeq(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 FBge* Sequence::createFBge(Val* cond1, Val* cond2, Label* label){
    auto inst = new FBge(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 FBgt* Sequence::createFBgt(Val* cond1, Val* cond2, Label* label){
    auto inst = new FBgt(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 FBle* Sequence::createFBle(Val* cond1, Val* cond2, Label* label){
    auto inst = new FBle(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 FBlt* Sequence::createFBlt(Val* cond1, Val* cond2, Label* label){
    auto inst = new FBlt(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 FBne* Sequence::createFBne(Val* cond1, Val* cond2, Label* label){
    auto inst = new FBne(cond1, cond2, label, this);
    insts.push_back(inst);
    return inst;
}
 Jump* Sequence::createJump(Label* label){
    auto inst = new Jump(label, this);
    insts.push_back(inst);
    return inst;
}
 Ret* Sequence::createRet(){
    auto inst = new Ret(this);
    insts.push_back(inst);
    return inst;
}
 CallerSaveRegs* Sequence::createCallerSaveRegs(::std::vector<::std::pair<IRA*, IRIA*>> caller_iregs_save){
    auto inst = new CallerSaveRegs(caller_iregs_save, this);
    insts.push_back(inst);
    return inst;
}
 CallerSaveRegs* Sequence::createCallerSaveRegs(::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_save){
    auto inst = new CallerSaveRegs(caller_fregs_save, this);
    insts.push_back(inst);
    return inst;
}
 CalleeSaveRegs* Sequence::createCalleeSaveRegs(::std::vector<::std::pair<IRA*, IRIA*>> callee_iregs_save){
    auto inst = new CalleeSaveRegs(callee_iregs_save, this);
    insts.push_back(inst);
    return inst;
}
 CalleeSaveRegs* Sequence::createCalleeSaveRegs(::std::vector<::std::pair<FRA*, IRIA*>> callee_fregs_save){
    auto inst = new CalleeSaveRegs(callee_fregs_save, this);
    insts.push_back(inst);
    return inst;
}
 CallerRestoreRegs* Sequence::createCallerRestoreRegs(::std::vector<::std::pair<IRA*, IRIA*>> caller_iregs_restore){
    auto inst = new CallerRestoreRegs(caller_iregs_restore, this);
    insts.push_back(inst);
    return inst;
}
 CallerRestoreRegs* Sequence::createCallerRestoreRegs(::std::vector<::std::pair<FRA*, IRIA*>> caller_fregs_restore){
    auto inst = new CallerRestoreRegs(caller_fregs_restore, this);
    insts.push_back(inst);
    return inst;
}
 CalleeRestoreRegs* Sequence::createCalleeRestoreRegs(::std::vector<::std::pair<IRA*, IRIA*>> callee_iregs_restore){
    auto inst = new CalleeRestoreRegs(callee_iregs_restore, this);
    insts.push_back(inst);
    return inst;
}
 CalleeRestoreRegs* Sequence::createCalleeRestoreRegs(::std::vector<::std::pair<FRA*, IRIA*>> callee_fregs_restore){
    auto inst = new CalleeRestoreRegs(callee_fregs_restore, this);
    insts.push_back(inst);
    return inst;
}
 CallerParaPass* Sequence::createCallerParaPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> caller_iparas_pass, ::std::vector<::std::pair<AddressMode*, AddressMode*>> caller_fparas_pass){
    auto inst = new CallerParaPass(caller_iparas_pass, caller_fparas_pass, this);
    insts.push_back(inst);
    return inst;
}
 CalleeParaPass* Sequence::createCalleeParaPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> callee_iparas_pass, ::std::vector<::std::pair<AddressMode*, AddressMode*>> callee_fparas_pass){
    auto inst = new CalleeParaPass(callee_iparas_pass,  callee_fparas_pass, this);
    insts.push_back(inst);
    return inst;
}
 CallerSaveResult* Sequence::createCallerSaveResult(GReg* grs, AddressMode* dst){
    auto inst = new CallerSaveResult(grs, dst, this);
    insts.push_back(inst);
    return inst;
}
 CallerSaveResult* Sequence::createCallerSaveResult(FReg* frs, AddressMode* dst){
    auto inst = new CallerSaveResult(frs, dst, this);
    insts.push_back(inst);
    return inst;
}
 CalleeSaveResult* Sequence::createCalleeSaveResult(IRA* idst, Val* src){
    auto inst = new CalleeSaveResult(idst, src, this);
    insts.push_back(inst);
    return inst;
}
 CalleeSaveResult* Sequence::createCalleeSaveResult(FRA* fdst, Val* src){
    auto inst = new CalleeSaveResult(fdst, src, this);
    insts.push_back(inst);
    return inst;
}
 CalleeStackFrameInitialize* Sequence::createCalleeStackFrameInitialize(int stack_initial_size){
    auto inst = new CalleeStackFrameInitialize(stack_initial_size, this);
    insts.push_back(inst);
    return inst;
}
 CalleeStackFrameClear* Sequence::createCalleeStackFrameClear(int stack_size_now){
    auto inst = new CalleeStackFrameClear(stack_size_now, this);
    insts.push_back(inst);
    return inst;
}
 CalleeStackFrameExpand* Sequence::createCalleeStackFrameExpand(int stack_size_expand){
    auto inst = new CalleeStackFrameExpand(stack_size_expand, this);
    insts.push_back(inst);
    return inst;
}
 CalleeStackFrameShrink* Sequence::createCalleeStackFrameShrink(int stack_size_shrink){
    auto inst = new CalleeStackFrameShrink(stack_size_shrink, this);
    insts.push_back(inst);
    return inst;
}
 LoadTmpRegs* Sequence::createLoadTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_load){
    auto inst = new LoadTmpRegs(iregs_tmp_load, this);
    insts.push_back(inst);
    return inst;
}
 LoadTmpRegs* Sequence::createLoadTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_load){
    auto inst = new LoadTmpRegs(fregs_tmp_load, this);
    insts.push_back(inst);
    return inst;
}
 StoreTmpRegs* Sequence::createStoreTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_store){
    auto inst = new StoreTmpRegs(iregs_tmp_store, this);
    insts.push_back(inst);
    return inst;
}
 StoreTmpRegs* Sequence::createStoreTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_store){
    auto inst = new StoreTmpRegs(fregs_tmp_store, this);
    insts.push_back(inst);
    return inst;
}
 AllocaTmpRegs* Sequence::createAllocaTmpRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_load, ::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_store){
    auto inst = new AllocaTmpRegs( iregs_tmp_load,iregs_tmp_store, this);
    insts.push_back(inst);
    return inst;
}
 AllocaTmpRegs* Sequence::createAllocaTmpRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_load, ::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_store){
    auto inst = new AllocaTmpRegs( fregs_tmp_load, fregs_tmp_store, this);
    insts.push_back(inst);
    return inst;
}
 InitializeAllTempRegs* Sequence::createInitializeAllTempRegs(::std::vector<::std::pair<IRA*, IRIA*>> iregs_tmp_restore){
    auto inst = new InitializeAllTempRegs(iregs_tmp_restore, this);
    insts.push_back(inst);
    return inst;
}
 InitializeAllTempRegs* Sequence::createInitializeAllTempRegs(::std::vector<::std::pair<FRA*, IRIA*>> fregs_tmp_restore){
    auto inst = new InitializeAllTempRegs(fregs_tmp_restore, this);
    insts.push_back(inst);
    return inst;
}
 PhiPass* Sequence::createPhiPass(::std::vector<::std::pair<AddressMode*, AddressMode*>> i_phi, ::std::vector<::std::pair<AddressMode*, AddressMode*>> f_phi){
    auto inst = new PhiPass(i_phi, f_phi, this);
    insts.push_back(inst);
    return inst;
}

Sh2Add* Sequence::createSh2Add(GReg* rd,GReg* rs1,GReg* rs2){
    auto inst = new Sh2Add(rd, rs1, rs2, this);
    insts.push_back(inst);
    return inst;   
}

LoadIImm* Sequence::createLoadIImm(GReg* grd, IConst* i_val){
    auto inst = new LoadIImm(grd, i_val, this);
    insts.push_back(inst);
    return inst;   
}

LoadFImm* Sequence::createLoadFImm(FReg* frd, FConst* f_val){
    auto inst = new LoadFImm(frd, f_val, this);
    insts.push_back(inst);
    return inst;   
}

Fmv_w_x* Sequence::createFmv_w_x (FReg* freg, GReg* g_val){
    auto inst = new Fmv_w_x(freg, g_val, this);
    insts.push_back(inst);
    return inst;   
}

Fmv_x_w* Sequence::createFmv_x_w(GReg* greg, FReg* f_val){
    auto inst = new Fmv_x_w(greg, f_val, this);
    insts.push_back(inst);
    return inst;   
}