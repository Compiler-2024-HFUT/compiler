#include "backend/Asm.hpp"


 ::std::string IConst::print(){
    return ::std::to_string(val);
}
 ::std::string FConst::print(){
    return ::std::to_string(*((uint32_t*)(&val)));
}
 ::std::string GReg::print(){
    return RISCV::reg2String(reg);
}
 ::std::string FReg::print(){
    return RISCV::freg2String(reg);
}
 ::std::string Mem::print(){
    return ::std::to_string(offset)+"("+RISCV::reg2String(reg)+")";
}
 ::std::string Label::print(){
    return label;
}
 ::std::string IConstPool::print(){
    return ::std::to_string(i_const_pool);
}
 ::std::string FConstPool::print(){
    return ::std::to_string(*((uint32_t*)(&f_const_pool)));
}
 ::std::string IRA::print(){
    return RISCV::reg2String(reg);
}
 ::std::string FRA::print(){
    return RISCV::freg2String(reg);
}
 ::std::string IRIA::print(){
    return ::std::to_string(offset)+"("+RISCV::reg2String(reg)+")";
}

 ::std::string AsmUnit::print(){
    asm_context+=AsmString::space+
                ".file"+
                AsmString::space+
                AsmString::dq+
                module->getFileName()+
                AsmString::dq+
                AsmString::newline+
                AsmString::space+
                ".option"+
                AsmString::space+
                "pic"+
                AsmString::newline+
                AsmString::space+
                ".attribute"+
                AsmString::space+
                "arch"+
                AsmString::comma+
                AsmString::dq+
                "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0_zba1p0_zbb1p0"+
                AsmString::dq+
                AsmString::newline+
                AsmString::space+
                ".attribute"+
                AsmString::space+
                "unaligned_access"+
                AsmString::comma+
                "0"+
                AsmString::newline+
"memset_i:\n\
        ble     a1,zero,.L1\n\
        slli    a5,a1,32\n\
        srli    a2,a5,30\n\
        li      a1,0\n\
        tail    memset@plt\n\
.L1:\n\
        ret\n\
.size   memset_i, .-memset_i\n\
memset_f:\n\
        ble     a1,zero,.L4\n\
        slli    a5,a1,32\n\
        srli    a2,a5,30\n\
        li      a1,0\n\
        tail    memset@plt\n\
.L4:\n\
        ret\n\
.size   memset_f, .-memset_f\n\n";
    ::std::vector<::std::string> data_section;
    for(auto i:module->getGlobalVariables()){
        ::std::string data_def;
        ::std::string data_name = i->getName();
        if(dynamic_cast<ConstantZero*>(i->getInit())){
            if(i->getType()->getPointerElementType()->isArrayType()){
                data_def+=AsmString::space+".global"+AsmString::space+data_name+AsmString::newline;
                data_def+=AsmString::space+".bss"+AsmString::newline;
                data_def+=AsmString::space+".align"+AsmString::space+::std::to_string(2)+AsmString::newline;
                data_def+=AsmString::space+".type"+AsmString::space+data_name+AsmString::comma+"@object"+AsmString::newline;
                data_def+=AsmString::space+".size"+AsmString::space+data_name+AsmString::comma+::std::to_string(i->getType()->getPointerElementType()->getSize())+AsmString::newline;
                data_def+=data_name+AsmString::colon+AsmString::newline;
                data_def+=AsmString::space+".zero"+AsmString::space+::std::to_string(i->getType()->getPointerElementType()->getSize())+AsmString::newline;
            }
            else{
                data_def+=AsmString::space+".global"+AsmString::space+data_name+AsmString::newline;
                data_def+=AsmString::space+".section"+AsmString::space+".sdata"+AsmString::comma+AsmString::dq+"aw"+AsmString::dq+AsmString::newline;
                data_def+=AsmString::space+".align"+AsmString::space+::std::to_string(2)+AsmString::newline;
                data_def+=AsmString::space+".type"+AsmString::space+data_name+AsmString::comma+"@object"+AsmString::newline;
                data_def+=AsmString::space+".size"+AsmString::space+data_name+AsmString::comma+::std::to_string(i->getType()->getPointerElementType()->getSize())+AsmString::newline;
                data_def+=data_name+AsmString::colon+AsmString::newline;
                data_def+=AsmString::space+".word";
                if(i->getInit()->getType()->isFloatType())
                    data_def+=AsmString::space+::std::to_string(*((uint32_t*)(&(static_cast<ConstantFP*>(i->getInit())->getValue()))))+AsmString::newline;
                else
                    data_def+=AsmString::space+::std::to_string(static_cast<ConstantInt*>(i->getInit())->getValue())+AsmString::newline;
            }
        }
        else{
            if(i->getType()->getPointerElementType()->isArrayType()){
                data_def+=AsmString::space+".global"+AsmString::space+data_name+AsmString::newline;
                data_def+=AsmString::space+".data"+AsmString::newline;
                data_def+=AsmString::space+".align"+AsmString::space+::std::to_string(2)+AsmString::newline;
                data_def+=AsmString::space+".type"+AsmString::space+data_name+AsmString::comma+"@object"+AsmString::newline;
                data_def+=AsmString::space+".size"+AsmString::space+data_name+AsmString::comma+::std::to_string(i->getType()->getPointerElementType()->getSize())+AsmString::newline;
                data_def+=data_name+AsmString::colon+AsmString::newline;
                auto array = dynamic_cast<ConstantArray*>(i->getInit());
                if(array->getType()->getArrayElementType()->isFloatType())
                    for(int p=0; p<array->getSizeOfArray(); p++)
                        data_def+=AsmString::space+".word"+AsmString::space+::std::to_string(*((uint32_t*)(&((static_cast<ConstantFP*>(array->getElementValue(p)))->getValue()))))+AsmString::newline;
                else    
                    for(int p=0; p<array->getSizeOfArray(); p++)
                        data_def+=AsmString::space+".word"+AsmString::space+::std::to_string((static_cast<ConstantInt*>(array->getElementValue(p)))->getValue())+AsmString::newline;
            }
            else{
                data_def+=AsmString::space+".global"+AsmString::space+data_name+AsmString::newline;
                data_def+=AsmString::space+".section"+AsmString::space+".sdata"+AsmString::comma+AsmString::dq+"aw"+AsmString::dq+AsmString::newline;
                data_def+=AsmString::space+".align"+AsmString::space+::std::to_string(2)+AsmString::newline;
                data_def+=AsmString::space+".type"+AsmString::space+data_name+AsmString::comma+"@object"+AsmString::newline;
                data_def+=AsmString::space+".size"+AsmString::space+data_name+AsmString::comma+::std::to_string(i->getType()->getPointerElementType()->getSize())+AsmString::newline;
                data_def+=data_name+AsmString::colon+AsmString::newline;
                data_def+=AsmString::space+".word";
                if(i->getInit()->getType()->isFloatType())
                    data_def+=AsmString::space+::std::to_string(*((uint32_t*)(&(static_cast<ConstantFP*>(i->getInit())->getValue()))))+AsmString::newline;
                else    
                    data_def+=AsmString::space+::std::to_string(static_cast<ConstantInt*>(i->getInit())->getValue())+AsmString::newline;


            }
        }
        data_section.push_back(data_def);
    }
    if(!data_section.empty()){
        asm_context+=AsmString::space+".text"+AsmString::newline;
        for(auto data_def:data_section)
            asm_context+=data_def;
    }

    if(!Subroutines.empty()){
        asm_context+=AsmString::space+".text"+AsmString::newline;
        for(auto subr:Subroutines)
            asm_context+=subr->print();
    }
    
    return asm_context;
}
 ::std::string Subroutine::print(){
    ::std::string subroutine_def;
    subroutine_def+=AsmString::space+".align"+AsmString::space+::std::to_string(1)+AsmString::newline;
    subroutine_def+=AsmString::space+".global"+AsmString::space+func->getName()+AsmString::newline;
    subroutine_def+=AsmString::space+".type"+AsmString::space+func->getName()+AsmString::comma+"@function"+AsmString::newline;
    subroutine_def+=func->getName()+AsmString::colon+AsmString::newline;
    if(!Sequences.empty()){
        for(auto s:Sequences)
            subroutine_def+=s->print();
    }
    subroutine_def+=AsmString::space+".size"+AsmString::space+func->getName()+AsmString::comma+".-"+func->getName()+AsmString::newline;
    return subroutine_def;
}
 ::std::string Sequence::print(){
    ::std::string squence_def;
    if(label->getLabel()!="")
        squence_def+=label->getLabel()+AsmString::colon+AsmString::newline;
    for(auto inst: insts)
        squence_def+=inst->print();
    

    return squence_def;
}

 ::std::string Add::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    auto iconst_rs2 = dynamic_cast<IConst*>(rs2);
    if(iconst_rs1 && iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst()+iconst_rs2->getIConst());
    else if(iconst_rs1)
        return RISCVInst::addi(rd, dynamic_cast<GReg*>(rs2), iconst_rs1->getIConst());
    else if(iconst_rs2)
        return RISCVInst::addi(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
    else
        return RISCVInst::add(rd, dynamic_cast<GReg*>(rs1), dynamic_cast<GReg*>(rs2));
}

 ::std::string Subw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    auto iconst_rs2 = dynamic_cast<IConst*>(rs2);
    if(iconst_rs1 && iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst()-iconst_rs2->getIConst());
    else if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst())+
               RISCVInst::subw(rd, rd, dynamic_cast<GReg*>(rs2));
    else if(iconst_rs2)
        return RISCVInst::addi(rd, dynamic_cast<GReg*>(rs1), -iconst_rs2->getIConst());
    else
        return RISCVInst::subw(rd, dynamic_cast<GReg*>(rs1), dynamic_cast<GReg*>(rs2));
}

 ::std::string Mulw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    auto iconst_rs2 = dynamic_cast<IConst*>(rs2);
    if(iconst_rs1 && iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst()*iconst_rs2->getIConst());
    else if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst())+
               RISCVInst::mulw(rd, rd, dynamic_cast<GReg*>(rs2));
    else if(iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs2->getIConst())+
               RISCVInst::mulw(rd, rd, dynamic_cast<GReg*>(rs1));
    else
        return RISCVInst::mulw(rd, dynamic_cast<GReg*>(rs1), dynamic_cast<GReg*>(rs2));
}


 ::std::string Muld::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    auto iconst_rs2 = dynamic_cast<IConst*>(rs2);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst())+
               RISCVInst::mul(rd, rd, dynamic_cast<GReg*>(rs2));
    else if(iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs2->getIConst())+
               RISCVInst::mul(rd, dynamic_cast<GReg*>(rs1), rd);
    else
        return RISCVInst::mul(rd, dynamic_cast<GReg*>(rs1), dynamic_cast<GReg*>(rs2));
}

 ::std::string Divw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    auto iconst_rs2 = dynamic_cast<IConst*>(rs2);
    if(iconst_rs1 && iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst()/iconst_rs2->getIConst());
    else if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst())+
               RISCVInst::divw(rd, rd, dynamic_cast<GReg*>(rs2));
    else if(iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs2->getIConst())+
               RISCVInst::divw(rd, dynamic_cast<GReg*>(rs1), rd);
    else
        return RISCVInst::divw(rd, dynamic_cast<GReg*>(rs1), dynamic_cast<GReg*>(rs2));
}

 ::std::string Remw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    auto iconst_rs2 = dynamic_cast<IConst*>(rs2);
    if(iconst_rs1 && iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst()%iconst_rs2->getIConst());
    else if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst())+
               RISCVInst::remw(rd, rd, dynamic_cast<GReg*>(rs2));
    else if(iconst_rs2)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs2->getIConst())+
               RISCVInst::remw(rd, dynamic_cast<GReg*>(rs1), rd);
    else
        return RISCVInst::remw(rd, dynamic_cast<GReg*>(rs1), dynamic_cast<GReg*>(rs2));
}

 ::std::string Sraw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())>>(iconst_rs2->getIConst()));
    else
        return RISCVInst::sraiw(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

 ::std::string Sllw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())<<(iconst_rs2->getIConst()));
    else
        return RISCVInst::slliw(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

 ::std::string Srlw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())>>(iconst_rs2->getIConst()));
    else
        return RISCVInst::srliw(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

 ::std::string Sra::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())>>(iconst_rs2->getIConst()));
    else
        return RISCVInst::srai(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

 ::std::string Sll::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())<<(iconst_rs2->getIConst()));
    else
        return RISCVInst::slli(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

 ::std::string Srl::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())>>(iconst_rs2->getIConst()));
    else
        return RISCVInst::srli(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

 ::std::string Land::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs2->getIConst()<-2048 || iconst_rs2->getIConst()>2047)
        return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs2->getIConst()))+
               RISCVInst::and_(rd, rs1, new GReg(static_cast<int>(RISCV::GPR::ra)));
    else
        return RISCVInst::andi(rd, rs1, iconst_rs2->getIConst());
}

 ::std::string Fadd_s::print(){
    auto fconst_rs1 = dynamic_cast<FConst*>(rs1);
    auto fconst_rs2 = dynamic_cast<FConst*>(rs2);
    if(fconst_rs1 && fconst_rs2){
        auto tmp = fconst_rs1->getFConst()+fconst_rs2->getFConst();
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&tmp))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)));
    }
    else if(fconst_rs1){
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs1->getFConst())))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fadd_s(rd, rd, dynamic_cast<FReg*>(rs2));
    }
    else if(fconst_rs2){
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs2->getFConst())))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fadd_s(rd, dynamic_cast<FReg*>(rs1), rd);
    }
    else{
        return RISCVInst::fadd_s(rd, dynamic_cast<FReg*>(rs1), dynamic_cast<FReg*>(rs2));
    }
}

 ::std::string Fsub_s::print(){
    auto fconst_rs1 = dynamic_cast<FConst*>(rs1);
    auto fconst_rs2 = dynamic_cast<FConst*>(rs2);
    if(fconst_rs1 && fconst_rs2){
        auto tmp = fconst_rs1->getFConst()-fconst_rs2->getFConst();
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&tmp))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)));
    }
    else if(fconst_rs1){
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs1->getFConst())))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fsub_s(rd, rd, dynamic_cast<FReg*>(rs2));
    }
    else if(fconst_rs2){
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs2->getFConst())))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fsub_s(rd, dynamic_cast<FReg*>(rs1), rd);
    }
    else{
        return RISCVInst::fsub_s(rd, dynamic_cast<FReg*>(rs1), dynamic_cast<FReg*>(rs2));
    }
}

 ::std::string Fmul_s::print(){
    auto fconst_rs1 = dynamic_cast<FConst*>(rs1);
    auto fconst_rs2 = dynamic_cast<FConst*>(rs2);
    if(fconst_rs1 && fconst_rs2){
        auto tmp = fconst_rs1->getFConst()*fconst_rs2->getFConst();
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&tmp))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)));
    }
    else if(fconst_rs1){
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs1->getFConst())))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fmul_s(rd, rd, dynamic_cast<FReg*>(rs2));
    }
    else if(fconst_rs2){
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs2->getFConst())))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fmul_s(rd, dynamic_cast<FReg*>(rs1), rd);
    }
    else{
        return RISCVInst::fmul_s(rd, dynamic_cast<FReg*>(rs1), dynamic_cast<FReg*>(rs2));
    }
}

 ::std::string Fdiv_s::print(){
    auto fconst_rs1 = dynamic_cast<FConst*>(rs1);
    auto fconst_rs2 = dynamic_cast<FConst*>(rs2);
    if(fconst_rs1 && fconst_rs2){
        auto tmp = fconst_rs1->getFConst()/fconst_rs2->getFConst();
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&tmp))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)));
    }
    else if(fconst_rs1){
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs1->getFConst())))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fdiv_s(rd, rd, dynamic_cast<FReg*>(rs2));
    }
    else if(fconst_rs2){
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs2->getFConst())))+
               RISCVInst::fmv_s_x(rd, new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fdiv_s(rd, dynamic_cast<FReg*>(rs1), rd);
    }
    else{
        return RISCVInst::fdiv_s(rd, dynamic_cast<FReg*>(rs1), dynamic_cast<FReg*>(rs2));
    }
}

 ::std::string Fcvt_w_s::print(){
    auto fconst_rs1 = dynamic_cast<FConst*>(rs1);
    if(fconst_rs1)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs1->getFConst())))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fcvt_w_s(rd, new FReg(static_cast<int>(RISCV::FPR::fs1)));
    else
        return RISCVInst::fcvt_w_s(rd, dynamic_cast<FReg*>(rs1));
}

 ::std::string Fcvt_s_w::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst())+
               RISCVInst::fcvt_s_w(rd, new GReg(static_cast<int>(RISCV::GPR::ra)));
    else
        return RISCVInst::fcvt_s_w(rd, dynamic_cast<GReg*>(rs1));
}

 ::std::string Zext::print(){
    return RISCVInst::andi(rd, rs1, 1);
}

 ::std::string ZextIConst::print(){
    return RISCVInst::li(rd, flag);
}
 ::std::string Snez::print(){
    auto iconst_cond = dynamic_cast<IConst*>(cond);
    auto fconst_cond = dynamic_cast<FConst*>(cond);
    auto ireg = dynamic_cast<GReg*>(cond);
    if(iconst_cond || fconst_cond){
        if(iconst_cond->getIConst()!=0 || fconst_cond->getFConst()!=0)
            return RISCVInst::seqz(rd, new GReg(static_cast<int>(RISCV::GPR::zero)));
        else    
            return RISCVInst::snez(rd, new GReg(static_cast<int>(RISCV::GPR::zero)));
    }
    else if(ireg)   
        return RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), ireg)+
               RISCVInst::snez(rd, new GReg(static_cast<int>(RISCV::GPR::ra)));
}

 ::std::string Seqz::print(){
    auto iconst_cond = dynamic_cast<IConst*>(cond);
    auto ireg = dynamic_cast<GReg*>(cond);
    if(iconst_cond){
        if(iconst_cond->getIConst()==0)
            return RISCVInst::seqz(rd, new GReg(static_cast<int>(RISCV::GPR::zero)));
        else    
            return RISCVInst::snez(rd, new GReg(static_cast<int>(RISCV::GPR::zero)));
    }
    else if(ireg)   
        return RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), ireg)+
               RISCVInst::seqz(rd, new GReg(static_cast<int>(RISCV::GPR::ra)));
}

 ::std::string Feq_s::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst() == fconst2->getFConst())
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 1);
        else
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 0);
    }
    else if(fconst1)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::feq_s(rd, new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2));
    else if(fconst2)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::feq_s(rd, dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)));
    else
        return RISCVInst::feq_s(rd, dynamic_cast<FReg*>(cond1), dynamic_cast<FReg*>(cond2));
    
}

 ::std::string Fle_s::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst() <= fconst2->getFConst())
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 1);
        else
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 0);
    }
    else if(fconst1)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fle_s(rd, new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2));
    else if(fconst2)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fle_s(rd, dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)));
    else
        return RISCVInst::fle_s(rd, dynamic_cast<FReg*>(cond1), dynamic_cast<FReg*>(cond2));
    
}

 ::std::string Flt_s::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst() < fconst2->getFConst())
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 1);
        else
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 0);
    }
    else if(fconst1)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::flt_s(rd, new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2));
    else if(fconst2)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::flt_s(rd, dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)));
    else
        return RISCVInst::flt_s(rd, dynamic_cast<FReg*>(cond1), dynamic_cast<FReg*>(cond2));
    
}

 ::std::string Fge_s::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst() >= fconst2->getFConst())
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 1);
        else
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 0);
    }
    else if(fconst1)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fle_s(rd, dynamic_cast<FReg*>(cond2), new FReg(static_cast<int>(RISCV::FPR::fs1)));
    else if(fconst2)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fle_s(rd, new FReg(static_cast<int>(RISCV::FPR::fs1)),dynamic_cast<FReg*>(cond1));
    else
        return RISCVInst::fle_s(rd,  dynamic_cast<FReg*>(cond2), dynamic_cast<FReg*>(cond1));
    
}


 ::std::string Fgt_s::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst() > fconst2->getFConst())
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 1);
        else
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 0);
    }
    else if(fconst1)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::flt_s(rd, dynamic_cast<FReg*>(cond2), new FReg(static_cast<int>(RISCV::FPR::fs1)));
    else if(fconst2)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::flt_s(rd, new FReg(static_cast<int>(RISCV::FPR::fs1)),dynamic_cast<FReg*>(cond1));
    else
        return RISCVInst::flt_s(rd,  dynamic_cast<FReg*>(cond2), dynamic_cast<FReg*>(cond1));
    
}

 ::std::string Fne_s::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst() != fconst2->getFConst())
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 1);
        else
            return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), 0);
    }
    else if(fconst1)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
               RISCVInst::seqz(rd, new GReg(static_cast<int>(RISCV::GPR::s1)));
    else if(fconst2)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
               RISCVInst::seqz(rd, new GReg(static_cast<int>(RISCV::GPR::s1)));

    else
        return RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), dynamic_cast<FReg*>(cond2))+
               RISCVInst::seqz(rd, new GReg(static_cast<int>(RISCV::GPR::s1)));
    
}

 ::std::string Lw::print(){
    auto iconst_offset = dynamic_cast<IConst*>(offset);
    if(iconst_offset){
        int iconst_offset_value = iconst_offset->getIConst()*4;
        if(iconst_offset_value<-2048 || iconst_offset_value>2047)
            return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), base, iconst_offset_value)+
                   RISCVInst::lw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0); 
        else    
            return RISCVInst::lw(rd, base, iconst_offset_value);
    }
    else{
        return 
               RISCVInst::sh2add(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), base)+
               RISCVInst::lw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

 ::std::string Lw_label::print(){
    return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
           RISCVInst::lw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
}

 ::std::string Flw::print(){
    auto iconst_offset = dynamic_cast<IConst*>(offset);
    if(iconst_offset){
        int iconst_offset_value = iconst_offset->getIConst()*4;
        if(iconst_offset_value<-2048 || iconst_offset_value>2047)
            return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), base, iconst_offset_value)+
                   RISCVInst::flw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0); 
        else    
            return RISCVInst::flw(rd, base, iconst_offset_value);
    }
    else{
        return RISCVInst::sh2add(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), base)+
               RISCVInst::flw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

 ::std::string Flw_label::print(){
    return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
           RISCVInst::flw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
}

 ::std::string Sw::print(){
    auto iconst_src = dynamic_cast<IConst*>(src);
    auto iconst_offset = dynamic_cast<IConst*>(offset);
    if(iconst_src){
        if(iconst_offset){
            int iconst_offset_value = iconst_offset->getIConst()*4;
            if(iconst_offset_value<-2048 || iconst_offset_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), base, iconst_offset_value)+
                       RISCVInst::sw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0);
            else
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src->getIConst())+
                       RISCVInst::sw(new GReg(static_cast<int>(RISCV::GPR::ra)), base, iconst_offset_value);
        }
        else
            return RISCVInst::sh2add(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), base)+
                   RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src->getIConst())+
                   RISCVInst::sw(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
    else{
        if(iconst_offset){
            int iconst_offset_value = iconst_offset->getIConst()*4;
            if(iconst_offset_value<-2048 || iconst_offset_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), base, iconst_offset_value)+
                       RISCVInst::sw(dynamic_cast<GReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            else
                return RISCVInst::sw(dynamic_cast<GReg*>(src), base, iconst_offset_value);
        }
        else
            return RISCVInst::sh2add(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), base)+
                   RISCVInst::sw(dynamic_cast<GReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

  ::std::string Sw_label::print(){
    auto iconst_src = dynamic_cast<IConst*>(src);
    if(iconst_src){
        return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
               RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src->getIConst())+
               RISCVInst::sw(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
    else{
        return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
               RISCVInst::sw(dynamic_cast<GReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

 ::std::string Fsw::print(){
    auto fconst_src = dynamic_cast<FConst*>(src);
    auto iconst_offset = dynamic_cast<IConst*>(offset);
    if(fconst_src){
        if(iconst_offset){
            int iconst_offset_value = iconst_offset->getIConst()*4;
            if(iconst_offset_value<-2048 || iconst_offset_value>2047)
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), base, iconst_offset_value)+
                       RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            else
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), base, iconst_offset_value);
        }
        else
            return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src->getFConst()))+
                   RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                   RISCVInst::sh2add(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), base)+
                   RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
    else{
        if(iconst_offset){
            int iconst_offset_value = iconst_offset->getIConst()*4;
            if(iconst_offset_value<-2048 || iconst_offset_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), base, iconst_offset_value)+
                       RISCVInst::fsw(dynamic_cast<FReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            else
                return RISCVInst::fsw(dynamic_cast<FReg*>(src), base, iconst_offset_value);
        }
        else
            return RISCVInst::sh2add(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), base)+
                   RISCVInst::fsw(dynamic_cast<FReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

  ::std::string Fsw_label::print(){
    auto fconst_src = dynamic_cast<FConst*>(src);
    if(fconst_src){
        return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
               RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src->getFConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
    else{
        return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
               RISCVInst::fsw(dynamic_cast<FReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

 ::std::string Call::print(){
    return RISCVInst::call(label);
}

 ::std::string La::print(){
    return RISCVInst::la(rd, label);
}

 ::std::string Mv::print(){
    return RISCVInst::mv(rd, rs1);
}

 ::std::string Beq::print(){
    auto iconst1 = dynamic_cast<IConst*>(cond1);
    auto iconst2 = dynamic_cast<IConst*>(cond2);
    if(iconst1 && iconst2){
        if(iconst1->getIConst()==iconst2->getIConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(iconst1){
        auto imem2 = dynamic_cast<Mem*>(cond2);
        if(imem2){
            int offset_value = imem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset_value)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(iconst2){
        auto imem1 = dynamic_cast<Mem*>(cond1);
        if(imem1){
            int offset_value = imem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem1->getRegId()), offset_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            }
            else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem1->getRegId()), offset_value)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            }
        }
        else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
        }
    }
    else{
        auto imem1 = dynamic_cast<Mem*>(cond1);
        auto imem2 = dynamic_cast<Mem*>(cond2);
        if(imem1 && imem2){
            int offset1_value = imem1->getOffset();
            int offset2_value = imem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(imem1){
            int offset1_value = imem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+    
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);      
        }
        else if(imem2){
            int offset2_value = imem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem2->getRegId()), offset2_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            else
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem2->getRegId()), offset2_value)+    
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);   
        }
        else{
                return RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label); 

        }
    }
}


 ::std::string Bne::print(){
    auto iconst1 = dynamic_cast<IConst*>(cond1);
    auto iconst2 = dynamic_cast<IConst*>(cond2);
    if(iconst1 && iconst2){
        if(iconst1->getIConst()!=iconst2->getIConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(iconst1){
        auto imem2 = dynamic_cast<Mem*>(cond2);
        if(imem2){
            int offset_value = imem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset_value)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(iconst2){
        auto imem1 = dynamic_cast<Mem*>(cond1);
        if(imem1){
            int offset_value = imem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem1->getRegId()), offset_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            }
            else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem1->getRegId()), offset_value)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            }
        }
        else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
        }
    }
    else{
        auto imem1 = dynamic_cast<Mem*>(cond1);
        auto imem2 = dynamic_cast<Mem*>(cond2);
        if(imem1 && imem2){
            int offset1_value = imem1->getOffset();
            int offset2_value = imem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(imem1){
            int offset1_value = imem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+    
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);      
        }
        else if(imem2){
            int offset2_value = imem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem2->getRegId()), offset2_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            else
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem2->getRegId()), offset2_value)+    
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);   
        }
        else{
                return RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label); 

        }
    }
}

 ::std::string Bge::print(){
    auto iconst1 = dynamic_cast<IConst*>(cond1);
    auto iconst2 = dynamic_cast<IConst*>(cond2);
    if(iconst1 && iconst2){
        if(iconst1->getIConst()>=iconst2->getIConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(iconst1){
        auto imem2 = dynamic_cast<Mem*>(cond2);
        if(imem2){
            int offset_value = imem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset_value)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(iconst2){
        auto imem1 = dynamic_cast<Mem*>(cond1);
        if(imem1){
            int offset_value = imem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem1->getRegId()), offset_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            }
            else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem1->getRegId()), offset_value)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            }
        }
        else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
        }
    }
    else{
        auto imem1 = dynamic_cast<Mem*>(cond1);
        auto imem2 = dynamic_cast<Mem*>(cond2);
        if(imem1 && imem2){
            int offset1_value = imem1->getOffset();
            int offset2_value = imem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(imem1){
            int offset1_value = imem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+    
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);      
        }
        else if(imem2){
            int offset2_value = imem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem2->getRegId()), offset2_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            else
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem2->getRegId()), offset2_value)+    
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);   
        }
        else{
                return RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::bge(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label); 

        }
    }
}

 ::std::string Blt::print(){
    auto iconst1 = dynamic_cast<IConst*>(cond1);
    auto iconst2 = dynamic_cast<IConst*>(cond2);
    if(iconst1 && iconst2){
        if(iconst1->getIConst()<iconst2->getIConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(iconst1){
        auto imem2 = dynamic_cast<Mem*>(cond2);
        if(imem2){
            int offset_value = imem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset_value)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst1->getIConst())+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(iconst2){
        auto imem1 = dynamic_cast<Mem*>(cond1);
        if(imem1){
            int offset_value = imem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem1->getRegId()), offset_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            }
            else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem1->getRegId()), offset_value)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            }
        }
        else{
                return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst2->getIConst())+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
        }
    }
    else{
        auto imem1 = dynamic_cast<Mem*>(cond1);
        auto imem2 = dynamic_cast<Mem*>(cond2);
        if(imem1 && imem2){
            int offset1_value = imem1->getOffset();
            int offset2_value = imem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(imem2->getRegId()), offset2_value)+
                        RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0)+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                        RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(imem1){
            int offset1_value = imem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem1->getRegId()), offset1_value)+    
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);      
        }
        else if(imem2){
            int offset2_value = imem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem2->getRegId()), offset2_value)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);
            else
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem2->getRegId()), offset2_value)+    
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), label);   
        }
        else{
                return RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(cond1))+
                       RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<GReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), label); 

        }
    }
}

 ::std::string FBeq::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst()==fconst2->getFConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(fconst1){
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem2){
            int offset_value = fmem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(fconst2){
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        if(fmem1){
            int offset_value = fmem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else{
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem1 && fmem2){
            int offset1_value = fmem1->getOffset();
            int offset2_value = fmem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem1){
            int offset1_value = fmem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem2){
            int offset2_value = fmem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else{
                return RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);

        }
    }
}



 ::std::string FBge::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst()>=fconst2->getFConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(fconst1){
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem2){
            int offset_value = fmem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond2), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(fconst2){
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        if(fmem1){
            int offset_value = fmem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else{
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem1 && fmem2){
            int offset1_value = fmem1->getOffset();
            int offset2_value = fmem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem1){
            int offset1_value = fmem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond2), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond2), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem2){
            int offset2_value = fmem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else{
                return RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond2), dynamic_cast<FReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);

        }
    }
}


 ::std::string FBgt::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst()>fconst2->getFConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(fconst1){
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem2){
            int offset_value = fmem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond2), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(fconst2){
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        if(fmem1){
            int offset_value = fmem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else{
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem1 && fmem2){
            int offset1_value = fmem1->getOffset();
            int offset2_value = fmem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem1){
            int offset1_value = fmem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond2), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond2), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem2){
            int offset2_value = fmem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else{
                return RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond2), dynamic_cast<FReg*>(cond1))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);

        }
    }
}

 ::std::string FBle::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst()<=fconst2->getFConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(fconst1){
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem2){
            int offset_value = fmem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(fconst2){
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        if(fmem1){
            int offset_value = fmem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else{
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem1 && fmem2){
            int offset1_value = fmem1->getOffset();
            int offset2_value = fmem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem1){
            int offset1_value = fmem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem2){
            int offset2_value = fmem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else{
                return RISCVInst::fle_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);

        }
    }
}

 ::std::string FBlt::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst()<fconst2->getFConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(fconst1){
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem2){
            int offset_value = fmem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(fconst2){
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        if(fmem1){
            int offset_value = fmem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else{
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem1 && fmem2){
            int offset1_value = fmem1->getOffset();
            int offset2_value = fmem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem1){
            int offset1_value = fmem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem2){
            int offset2_value = fmem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else{
                return RISCVInst::flt_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::blt(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);

        }
    }
}

 ::std::string FBne::print(){
    auto fconst1 = dynamic_cast<FConst*>(cond1);
    auto fconst2 = dynamic_cast<FConst*>(cond2);
    if(fconst1 && fconst2){
        if(fconst1->getFConst()!=fconst2->getFConst())
            return RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
        else
            return RISCVInst::bne(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::zero)), label);
    }
    else if(fconst1){
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem2){
            int offset_value = fmem2->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset_value)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst1->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else if(fconst2){
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        if(fmem1){
            int offset_value = fmem1->getOffset();
            if(offset_value>2047 || offset_value<-2048){
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs0)), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
            else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem1->getRegId()), offset_value)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            }
        }
        else{
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst2->getFConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
    }
    else{
        auto fmem1 = dynamic_cast<Mem*>(cond1);
        auto fmem2 = dynamic_cast<Mem*>(cond2);
        if(fmem1 && fmem2){
            int offset1_value = fmem1->getOffset();
            int offset2_value = fmem2->getOffset();
            bool offset1_is_valid, offset2_is_valid;
            if(offset1_value <-2048 || offset1_value>2047)  offset1_is_valid = false;
            else offset1_is_valid = true;
            if(offset2_value <-2048 || offset2_value>2047)  offset2_is_valid = false;
            else offset2_is_valid = true;

            if(offset1_is_valid && offset2_is_valid) 
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset1_is_valid)
                return  RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else if(offset2_is_valid)
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else 
                return  RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                        RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                        RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), new FReg(static_cast<int>(RISCV::FPR::fs0)))+
                        RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem1){
            int offset1_value = fmem1->getOffset();
            if(offset1_value<-2048 || offset1_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem1->getRegId()), offset1_value)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), new FReg(static_cast<int>(RISCV::FPR::fs1)), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else if(fmem2){
            int offset2_value = fmem2->getOffset();
            if(offset2_value<-2048 || offset2_value>2047)
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
            else
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(fmem2->getRegId()), offset2_value)+
                       RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), new FReg(static_cast<int>(RISCV::FPR::fs1)))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);
        }
        else{
                return RISCVInst::feq_s(new GReg(static_cast<int>(RISCV::GPR::s1)), dynamic_cast<FReg*>(cond1), dynamic_cast<FReg*>(cond2))+
                       RISCVInst::beq(new GReg(static_cast<int>(RISCV::GPR::zero)), new GReg(static_cast<int>(RISCV::GPR::s1)), label);

        }
    }
}

 ::std::string Jump::print(){
    return RISCVInst::j(label);
}

 ::std::string Ret::print(){
    return RISCVInst::ret();
}











 ::std::string CallerSaveRegs::print(){
    ::std::string asm_insts;
    for(auto i:caller_iregs_save)
        asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(i.first->getReg())), new GReg(static_cast<int>(i.second->getReg())), i.second->getOffset());
    
    for(auto f:caller_fregs_save)
        asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(f.first->getReg())), new GReg(static_cast<int>(f.second->getReg())), f.second->getOffset());
    return asm_insts;
}

 ::std::string CalleeSaveRegs::print(){
    ::std::string asm_insts;
    for(auto i:callee_iregs_save)
        asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(i.first->getReg())), new GReg(static_cast<int>(i.second->getReg())), i.second->getOffset());
    
    for(auto f:callee_fregs_save)
        asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(f.first->getReg())), new GReg(static_cast<int>(f.second->getReg())), f.second->getOffset());
    return asm_insts;
}

 ::std::string CallerRestoreRegs::print(){
    ::std::string asm_insts;
    for(auto i:caller_iregs_restore)
        asm_insts+=RISCVInst::ld(new GReg(static_cast<int>(i.first->getReg())), new GReg(static_cast<int>(i.second->getReg())), i.second->getOffset());
    
    for(auto f:caller_fregs_restore)
        asm_insts+=RISCVInst::flw(new FReg(static_cast<int>(f.first->getReg())), new GReg(static_cast<int>(f.second->getReg())), f.second->getOffset());
    return asm_insts;
}

 ::std::string CalleeRestoreRegs::print(){
    ::std::string asm_insts;
    for(auto i:callee_iregs_restore)
        asm_insts+=RISCVInst::ld(new GReg(static_cast<int>(i.first->getReg())), new GReg(static_cast<int>(i.second->getReg())), i.second->getOffset());
    
    for(auto f:callee_fregs_restore)
        asm_insts+=RISCVInst::flw(new FReg(static_cast<int>(f.first->getReg())), new GReg(static_cast<int>(f.second->getReg())), f.second->getOffset());
    return asm_insts;
}

 ::std::string CallerParaPass::print(){
    ::std::string asm_insts;
    for(auto i:caller_iparas_pass){
        auto src_para = i.second;
        auto target_para = i.first;

        auto iconst_src_para = dynamic_cast<IConstPool*>(src_para);
        auto ira_src_para = dynamic_cast<IRA*>(src_para);
        auto iria_src_para = dynamic_cast<IRIA*>(src_para);

        auto ira_target_para = dynamic_cast<IRA*>(target_para);
        auto iria_target_para = dynamic_cast<IRIA*>(target_para);

        if(iconst_src_para){
            if(ira_target_para)
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(ira_target_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src_para->getIConstPool());
            else if(iria_target_para){
                int offset_value = iria_target_para->getOffset();
                if(offset_value<-2048||offset_value>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target_para->getReg())), offset_value)+
                               RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src_para->getIConstPool())+
                               RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src_para->getIConstPool())+
                               RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(iria_target_para->getReg())), offset_value);
                }
            }
        }
        else if(ira_src_para){
            if(ira_target_para)
                asm_insts+=RISCVInst::mv(new GReg(static_cast<int>(ira_target_para->getReg())), new GReg(static_cast<int>(ira_src_para->getReg())));
            else if(iria_target_para){
                int offset_value = iria_target_para->getOffset();
                if(offset_value<-2048||offset_value>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target_para->getReg())), offset_value)+
                               RISCVInst::sd(new GReg(static_cast<int>(ira_src_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(ira_src_para->getReg())), new GReg(static_cast<int>(iria_target_para->getReg())), offset_value);
                }
            }
        }
        else{
            int src_offset_value = iria_src_para->getOffset();
         
            if(src_offset_value<-2048||src_offset_value>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_src_para->getReg())), src_offset_value)+
                           RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                asm_insts+=RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_src_para->getReg())), src_offset_value);
            }
            if(ira_target_para){
                asm_insts+=RISCVInst::mv(new GReg(static_cast<int>(ira_target_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)));
            }
            else if(iria_target_para){
                   int target_offset_value = iria_target_para->getOffset();
                if(target_offset_value<-2048||target_offset_value>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(iria_target_para->getReg())), target_offset_value)+
                               RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0);
                }
                else{
                    asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target_para->getReg())), target_offset_value);
                }
            }

        }
    }


    for(auto f:caller_fparas_pass){
        auto src_para = f.second;
        auto target_para = f.first;

        auto fconst_src_para = dynamic_cast<FConstPool*>(src_para);
        auto fra_src_para = dynamic_cast<FRA*>(src_para);
        auto iria_src_para = dynamic_cast<IRIA*>(src_para);

        auto fra_target_para = dynamic_cast<FRA*>(target_para);
        auto iria_target_para = dynamic_cast<IRIA*>(target_para);

        if(fconst_src_para){
            if(fra_target_para)
                asm_insts+=RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src_para->getFConstPool()))+
                           RISCVInst::fmv_s_x(new FReg(static_cast<int>(fra_target_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::s1)));
            else if(iria_target_para){
                int offset_value = iria_target_para->getOffset();
                if(offset_value<-2048||offset_value>2047){
                    asm_insts+=RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src_para->getFConstPool()))+
                               RISCVInst::fmv_s_x(new FReg(static_cast<int>(fra_target_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                               RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target_para->getReg())), offset_value)+
                               RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src_para->getFConstPool()))+
                               RISCVInst::fmv_s_x(new FReg(static_cast<int>(fra_target_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                               RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(iria_target_para->getReg())), offset_value);
                }
            }
        }
        else if(fra_src_para){
            if(fra_target_para)
                asm_insts+=RISCVInst::fmv_s(new FReg(static_cast<int>(fra_target_para->getReg())), new FReg(static_cast<int>(fra_src_para->getReg())));
            else if(iria_target_para){
                int offset_value = iria_target_para->getOffset();
                if(offset_value<-2048||offset_value>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target_para->getReg())), offset_value)+
                               RISCVInst::fsw(new FReg(static_cast<int>(fra_src_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(fra_src_para->getReg())), new GReg(static_cast<int>(iria_target_para->getReg())), offset_value);
                }
            }
        }
        else{
            int src_offset_value = iria_src_para->getOffset();
         
            if(src_offset_value<-2048||src_offset_value>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_src_para->getReg())), src_offset_value)+
                           RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                asm_insts+=RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(iria_src_para->getReg())), src_offset_value);
            }
            if(fra_target_para){
                asm_insts+=RISCVInst::fmv_s(new FReg(static_cast<int>(fra_target_para->getReg())), new FReg(static_cast<int>(RISCV::FPR::fs1)));
            }
            else if(iria_target_para){
                   int target_offset_value = iria_target_para->getOffset();
                if(target_offset_value<-2048||target_offset_value>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target_para->getReg())), target_offset_value)+
                               RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(iria_target_para->getReg())), target_offset_value);
                }
            }

        }
    }

    return asm_insts;
}

 ::std::string CalleeParaPass::print(){
    ::std::string asm_insts;
    for(auto i:callee_iparas_pass){
        auto src_para = i.second;
        auto target_para = i.first;
        auto ira_src_para = dynamic_cast<IRA*>(src_para);
        auto ira_target_para = dynamic_cast<IRA*>(target_para);
        auto iria_src_para = dynamic_cast<IRIA*>(src_para);
        auto iria_target_para = dynamic_cast<IRIA*>(target_para);

        if(ira_src_para && ira_target_para){
            asm_insts+=RISCVInst::mv(new GReg(static_cast<int>(ira_target_para->getReg())), new GReg(static_cast<int>(ira_src_para->getReg())));
        }
        else if(ira_src_para && iria_target_para){
            int offset_target = iria_target_para->getOffset();
            if(offset_target<-2048 || offset_target>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target_para->getReg())), offset_target)+
                           RISCVInst::sd(new GReg(static_cast<int>(ira_src_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            } 
            else{
                asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(ira_src_para->getReg())), new GReg(static_cast<int>(iria_target_para->getReg())), offset_target);
            }
        }
        else if(iria_src_para && ira_target_para){
            int offset_src = iria_src_para->getOffset();
            if(offset_src<-2048||offset_src>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_src_para->getReg())), offset_src)+
                           RISCVInst::ld(new GReg(static_cast<int>(ira_target_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)),0);
            }
            else{
                asm_insts+=RISCVInst::ld(new GReg(static_cast<int>(ira_target_para->getReg())), new GReg(static_cast<int>(iria_src_para->getReg())), offset_src);
            }
        }

    }
    for(auto f:callee_fparas_pass){
        auto src_para = f.second;
        auto target_para = f.first;
        auto fra_src_para = dynamic_cast<FRA*>(src_para);
        auto fra_target_para = dynamic_cast<FRA*>(target_para);
        auto iria_src_para = dynamic_cast<IRIA*>(src_para);
        auto iria_target_para = dynamic_cast<IRIA*>(target_para);

        if(fra_src_para && fra_target_para){
            asm_insts+=RISCVInst::fmv_s(new FReg(static_cast<int>(fra_target_para->getReg())), new FReg(static_cast<int>(fra_src_para->getReg())));
        }
        else if(fra_src_para && iria_target_para){
            int offset_target = iria_target_para->getOffset();
            if(offset_target<-2048 || offset_target>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target_para->getReg())), offset_target)+
                           RISCVInst::fsw(new FReg(static_cast<int>(fra_src_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            } 
            else{
                asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(fra_src_para->getReg())), new GReg(static_cast<int>(iria_target_para->getReg())), offset_target);
            }
        }
        else if(iria_src_para && fra_target_para){
            int offset_src = iria_src_para->getOffset();
            if(offset_src<-2048||offset_src>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_src_para->getReg())), offset_src)+
                           RISCVInst::flw(new FReg(static_cast<int>(fra_target_para->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)),0);
            }
            else{
                asm_insts+=RISCVInst::flw(new FReg(static_cast<int>(fra_target_para->getReg())), new GReg(static_cast<int>(iria_src_para->getReg())), offset_src);
            }
        }
    }


    return asm_insts;
}

 ::std::string CallerSaveResult::print(){

    if(grs!=nullptr){
        auto iria_dst = dynamic_cast<IRIA*>(dst);
        auto ira_dst = dynamic_cast<IRA*>(dst);
        if(iria_dst){
            int offset = iria_dst->getOffset();
            if(offset<-2048||offset>2047){
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_dst->getReg())), offset)+
                       RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::a0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                return RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::a0)), new GReg(static_cast<int>(iria_dst->getReg())), offset);
            }
        }
        else{
                return RISCVInst::mv(new GReg(static_cast<int>(ira_dst->getReg())), new GReg(static_cast<int>(RISCV::GPR::a0)));
        }
    }
    else if(frs!=nullptr){
        auto iria_dst = dynamic_cast<IRIA*>(dst);
        auto fra_dst = dynamic_cast<FRA*>(dst);
        if(iria_dst){
            int offset = iria_dst->getOffset();
            if(offset<-2048||offset>2047){
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_dst->getReg())), offset)+
                       RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fa0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                return RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fa0)), new GReg(static_cast<int>(iria_dst->getReg())), offset);
            }
        }
        else{
                return RISCVInst::fmv_s(new FReg(static_cast<int>(fra_dst->getReg())), new FReg(static_cast<int>(RISCV::FPR::fa0)));
        }
    }



}


 ::std::string CalleeSaveResult::print(){

    if(idst!=nullptr){
        auto iconst_src = dynamic_cast<IConst*>(src);
        auto greg_src = dynamic_cast<GReg*>(src);
        auto imem_src = dynamic_cast<Mem*>(src);
        if(iconst_src){
            return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::a0)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src->getIConst());
        }
        else if(imem_src){
            int offset = imem_src->getOffset();
            if(offset<-2048||offset>2047){
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(imem_src->getRegId()), offset)+
                       RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::a0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                return RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::a0)), new GReg(imem_src->getRegId()), offset);
            }
        }
        else if(greg_src){
            return RISCVInst::mv(new GReg(static_cast<int>(RISCV::GPR::a0)), greg_src);
        }

    }
    else if(fdst!=nullptr){
        auto fconst_src = dynamic_cast<FConst*>(src);
        auto freg_src = dynamic_cast<FReg*>(src);
        auto fmem_src = dynamic_cast<Mem*>(src);
        if(fconst_src){
            return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src->getFConst()))+
                   RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fa0)), new GReg(static_cast<int>(RISCV::GPR::s1)));
        }
        else if(fmem_src){
            int offset = fmem_src->getOffset();
            if(offset<-2048||offset>2047){
                return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(fmem_src->getRegId()), offset)+
                       RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fa0)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                return RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fa0)), new GReg(fmem_src->getRegId()), offset);
            }
        }
        else if(freg_src){
            return RISCVInst::fmv_s(new FReg(static_cast<int>(RISCV::FPR::fa0)), freg_src);
        }
    }



}

 ::std::string CalleeStackFrameInitialize::print(){
    return RISCVInst::mv(new GReg(static_cast<int>(RISCV::GPR::s0)), new GReg(static_cast<int>(RISCV::GPR::sp)))+
           RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::sp)), new GReg(static_cast<int>(RISCV::GPR::sp)), -stack_initial_size);
}

 ::std::string CalleeStackFrameClear::print(){
    return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::sp)), new GReg(static_cast<int>(RISCV::GPR::sp)), stack_size_now);
}

 ::std::string CalleeStackFrameExpand::print(){
    return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::sp)), new GReg(static_cast<int>(RISCV::GPR::sp)), stack_size_expand);
}

 ::std::string CalleeStackFrameShrink::print(){
    return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::sp)), new GReg(static_cast<int>(RISCV::GPR::sp)), stack_size_shrink);
}

 ::std::string LoadTmpRegs::print(){
    ::std::string asm_insts;
    for(auto i: iregs_tmp_load){
        auto ira = i.first;
        auto iria = i.second;
        int offset = iria->getOffset();
        if(offset<-2048||offset>2047){
            asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                       RISCVInst::ld(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
        }
        else{
            asm_insts+=RISCVInst::ld(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
        }
    }   

    for(auto f:fregs_tmp_load){
        auto fra = f.first;
        auto iria = f.second;
        int offset = iria->getOffset();
        if(offset<-2048||offset>2047){
            asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                       RISCVInst::flw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
        }
        else{
            asm_insts+=RISCVInst::flw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
        }
    }


    return asm_insts;
}

 ::std::string StoreTmpRegs::print(){
    ::std::string asm_insts;
    for(auto i: iregs_tmp_store){
        auto ira = i.first;
        auto iria = i.second;
        int offset = iria->getOffset();
        if(offset<-2048||offset>2047){
            asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                       RISCVInst::sd(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
        }
        else{
            asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
        }
    }   

    for(auto f:fregs_tmp_store){
        auto fra = f.first;
        auto iria = f.second;
        int offset = iria->getOffset();
        if(offset<-2048||offset>2047){
            asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                       RISCVInst::fsw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
        }
        else{
            asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
        }
    }


    return asm_insts;
}

 ::std::string AllocaTmpRegs::print(){
    ::std::string asm_insts;
        for(auto is:iregs_tmp_store){
            auto ira = is.first;
            auto iria = is.second;
            int offset = iria->getOffset();
            if(offset<-2048||offset>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                           RISCVInst::sd(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
            }

        }

        for(auto fs:fregs_tmp_store){
            auto fra = fs.first;
            auto iria = fs.second;
            int offset = iria->getOffset();
            if(offset<-2048||offset>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                           RISCVInst::fsw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
            }
        }

        for(auto il:iregs_tmp_load){
            auto ira = il.first;
            auto iria = il.second;
            int offset = iria->getOffset();
            if(offset<-2048||offset>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                           RISCVInst::ld(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                asm_insts+=RISCVInst::ld(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
            }

        }
        for(auto fl:fregs_tmp_load){
            auto fra = fl.first;
            auto iria = fl.second;
            int offset = iria->getOffset();
            if(offset<-2048||offset>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                           RISCVInst::flw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                asm_insts+=RISCVInst::flw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
            }

        }


    return asm_insts;
}

 ::std::string InitializeAllTempRegs::print(){
    ::std::string asm_insts;
    for(auto i: iregs_tmp_restore){
        auto ira = i.first;
        auto iria = i.second;
        int offset = iria->getOffset();
        if(offset<-2048||offset>2047){
            asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                       RISCVInst::ld(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
        }
        else{
            asm_insts+=RISCVInst::ld(new GReg(static_cast<int>(ira->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
        }
    }   

    for(auto f:fregs_tmp_restore){
        auto fra = f.first;
        auto iria = f.second;
        int offset = iria->getOffset();
        if(offset<-2048||offset>2047){
            asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria->getReg())), offset)+
                       RISCVInst::flw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
        }
        else{
            asm_insts+=RISCVInst::flw(new FReg(static_cast<int>(fra->getReg())), new GReg(static_cast<int>(iria->getReg())), offset);
        }
    }


    return asm_insts;
}

 ::std::string PhiPass::print(){
    ::std::string asm_insts;
    for(auto i: i_phi){
        auto src = i.second;
        auto target = i.first;
        auto iconst_src = dynamic_cast<IConstPool*>(src);
        auto ireg_src = dynamic_cast<IRA*>(src);
        auto iria_src = dynamic_cast<IRIA*>(src);
        auto ireg_target = dynamic_cast<IRA*>(target);
        auto iria_target = dynamic_cast<IRIA*>(target);
        if(iconst_src){
            if(ireg_target){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(ireg_target->getReg())), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src->getIConstPool());
            }
            else if(iria_target){
                int offset = iria_target->getOffset();
                if(offset<-2048||offset>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target->getReg())), offset)+
                               RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src->getIConstPool())+
                               RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_src->getIConstPool())+
                               RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(iria_target->getReg())), offset);
                }
            }

        }
        else if(ireg_src){
            if(ireg_target){
                asm_insts+=RISCVInst::mv(new GReg(static_cast<int>(ireg_target->getReg())), new GReg(static_cast<int>(ireg_src->getReg())));
            }
            else if(iria_target){
                int offset = iria_target->getOffset();
                if(offset<-2048||offset>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target->getReg())), offset)+
                               RISCVInst::sd(new GReg(static_cast<int>(ireg_src->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(ireg_src->getReg())), new GReg(static_cast<int>(iria_target->getReg())), offset);
                }
            }
        }
        else{
            int src_offset = iria_src->getOffset();
            if(src_offset<-2048||src_offset>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)),new GReg(static_cast<int>(iria_src->getReg())), src_offset)+
                           RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                asm_insts+=RISCVInst::ld(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_src->getReg())), src_offset);
            }
           
            if(ireg_target){
                asm_insts+=RISCVInst::mv(new GReg(static_cast<int>(ireg_target->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)));
            }
            else if(iria_target){
                 int target_offset = iria_target->getOffset();
                if(target_offset<-2048||target_offset>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::s1)), new GReg(static_cast<int>(iria_target->getReg())), target_offset)+
                               RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::s1)), 0);
                }
                else{
                    asm_insts+=RISCVInst::sd(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target->getReg())), target_offset);
                }
            }
        }
    }

    for(auto f:f_phi){
        auto src = f.second;
        auto target = f.first;
        auto fconst_src = dynamic_cast<FConstPool*>(src);
        auto freg_src = dynamic_cast<FRA*>(src);
        auto iria_src = dynamic_cast<IRIA*>(src);
        auto freg_target = dynamic_cast<FRA*>(target);
        auto iria_target = dynamic_cast<IRIA*>(target);
        if(fconst_src){
            if(freg_target){
                asm_insts+=RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src->getFConstPool()))+
                           RISCVInst::fmv_s_x(new FReg(static_cast<int>(freg_target->getReg())), new GReg(static_cast<int>(RISCV::GPR::s1)));
            }
            else if(iria_target){
                int offset = iria_target->getOffset();
                if(offset<-2048||offset>2047){
                    asm_insts+=RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src->getFConstPool()))+
                               RISCVInst::fmv_s_x(new FReg(static_cast<int>(freg_target->getReg())), new GReg(static_cast<int>(RISCV::GPR::s1)));RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target->getReg())), offset)+
                               RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target->getReg())), offset)+
                               RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(fconst_src->getFConstPool()))+
                               RISCVInst::fmv_s_x(new FReg(static_cast<int>(freg_target->getReg())), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                               RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(iria_target->getReg())), offset);
                }
            }

        }
        else if(freg_src){
            if(freg_target){
                asm_insts+=RISCVInst::fmv_s(new FReg(static_cast<int>(freg_target->getReg())), new FReg(static_cast<int>(freg_src->getReg())));
            }       
            else if(iria_target){
                int offset = iria_target->getOffset();
                if(offset<-2048||offset>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target->getReg())), offset)+
                               RISCVInst::fsw(new FReg(static_cast<int>(freg_src->getReg())), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(freg_src->getReg())), new GReg(static_cast<int>(iria_target->getReg())), offset);
                }
            }
        }
        else{
            int src_offset = iria_src->getOffset();
            if(src_offset<-2048||src_offset>2047){
                asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)),new GReg(static_cast<int>(iria_src->getReg())), src_offset)+
                           RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            }
            else{
                asm_insts+=RISCVInst::flw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(iria_src->getReg())), src_offset);
            }
            
            if(freg_target){
                asm_insts+=RISCVInst::fmv_s(new FReg(static_cast<int>(freg_target->getReg())), new FReg(static_cast<int>(RISCV::FPR::fs1)));
            }
            else if(iria_target){
                int target_offset = iria_target->getOffset();
                if(target_offset<-2048||target_offset>2047){
                    asm_insts+=RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(iria_target->getReg())), target_offset)+
                               RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
                }
                else{
                    asm_insts+=RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(iria_target->getReg())), target_offset);
                }
            }
        }
    }

    return asm_insts;
}


 ::std::string Sh2Add::print(){
    return RISCVInst::sh2add(rd, rs1, rs2);
}

::std::string LoadIImm::print(){
    return RISCVInst::li(grd, i_val->getIConst());
}

::std::string LoadFImm::print(){
    return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(f_val->getFConst())))+
           RISCVInst::fmv_s_x(frd, new GReg(static_cast<int>(RISCV::GPR::s1)));
}


::std::string Fmv_w_x::print(){
    return RISCVInst::fmv_w_x(frd, g_val);
}

::std::string Fmv_x_w::print(){
    return RISCVInst::fmv_x_w(grd, f_val);
}