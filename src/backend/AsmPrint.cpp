#include "backend/Asm.hpp"


inline ::std::string IConst::print(){
    return ::std::to_string(val);
}
inline ::std::string FConst::print(){
    return ::std::to_string(*((uint32_t*)(&val)));
}
inline ::std::string GReg::print(){
    return RISCV::reg2String(reg);
}
inline ::std::string FReg::print(){
    return RISCV::freg2String(reg);
}
inline ::std::string Mem::print(){
    return ::std::to_string(offset)+"("+RISCV::reg2String(reg)+")";
}
inline ::std::string Label::print(){
    return label;
}
inline ::std::string IConstPool::print(){
    return ::std::to_string(i_const_pool);
}
inline ::std::string FConstPool::print(){
    return ::std::to_string(*((uint32_t*)(&f_const_pool)));
}
inline ::std::string IRA::print(){
    return RISCV::reg2String(reg);
}
inline ::std::string FRA::print(){
    return RISCV::freg2String(reg);
}
inline ::std::string IRIA::print(){
    return ::std::to_string(offset)+"("+RISCV::reg2String(reg)+")";
}

inline ::std::string AsmUnit::print(){
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
                AsmString::newline;
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
inline ::std::string Subroutine::print(){
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
inline ::std::string Sequence::print(){
    ::std::string squence_def;
    if(label->getLabel()!="")
        squence_def+=label->getLabel()+AsmString::colon+AsmString::newline;
    for(auto inst: insts)
        squence_def+=inst->print();
    

    return squence_def;
}

inline ::std::string Add::print(){
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

inline ::std::string Subw::print(){
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

inline ::std::string Mulw::print(){
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


inline ::std::string Muld::print(){
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

inline ::std::string Divw::print(){
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

inline ::std::string Remw::print(){
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

inline ::std::string Sraw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())>>(iconst_rs2->getIConst()));
    else
        return RISCVInst::sraiw(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

inline ::std::string Sllw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())<<(iconst_rs2->getIConst()));
    else
        return RISCVInst::slliw(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

inline ::std::string Srlw::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())>>(iconst_rs2->getIConst()));
    else
        return RISCVInst::srliw(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

inline ::std::string Sra::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())>>(iconst_rs2->getIConst()));
    else
        return RISCVInst::srai(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

inline ::std::string Sll::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())<<(iconst_rs2->getIConst()));
    else
        return RISCVInst::slli(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

inline ::std::string Srl::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addi(rd, new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs1->getIConst())>>(iconst_rs2->getIConst()));
    else
        return RISCVInst::srli(rd, dynamic_cast<GReg*>(rs1), iconst_rs2->getIConst());
}

inline ::std::string Land::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs2->getIConst()<-2048 || iconst_rs2->getIConst()>2047)
        return RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), (iconst_rs2->getIConst()))+
               RISCVInst::and_(rd, rs1, new GReg(static_cast<int>(RISCV::GPR::ra)));
    else
        return RISCVInst::andi(rd, rs1, iconst_rs2->getIConst());
}

inline ::std::string Fadd_s::print(){
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

inline ::std::string Fsub_s::print(){
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

inline ::std::string Fmul_s::print(){
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

inline ::std::string Fdiv_s::print(){
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

inline ::std::string Fcvt_w_s::print(){
    auto fconst_rs1 = dynamic_cast<FConst*>(rs1);
    if(fconst_rs1)
        return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)(&(fconst_rs1->getFConst())))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fcvt_w_s(rd, new FReg(static_cast<int>(RISCV::FPR::fs1)));
    else
        return RISCVInst::fcvt_w_s(rd, dynamic_cast<FReg*>(rs1));
}

inline ::std::string Fcvt_s_w::print(){
    auto iconst_rs1 = dynamic_cast<IConst*>(rs1);
    if(iconst_rs1)
        return RISCVInst::addiw(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::zero)), iconst_rs1->getIConst())+
               RISCVInst::fcvt_s_w(rd, new GReg(static_cast<int>(RISCV::GPR::ra)));
    else
        return RISCVInst::fcvt_s_w(rd, dynamic_cast<GReg*>(rs1));
}

inline ::std::string Zext::print(){
    return RISCVInst::addi(rd, rs1, 1);
}

inline ::std::string Snez::print(){
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

inline ::std::string Seqz::print(){
    auto iconst_cond = dynamic_cast<IConst*>(cond);
    auto fconst_cond = dynamic_cast<FConst*>(cond);
    auto ireg = dynamic_cast<GReg*>(cond);
    if(iconst_cond || fconst_cond){
        if(iconst_cond->getIConst()==0 || fconst_cond->getFConst()==0)
            return RISCVInst::seqz(rd, new GReg(static_cast<int>(RISCV::GPR::zero)));
        else    
            return RISCVInst::snez(rd, new GReg(static_cast<int>(RISCV::GPR::zero)));
    }
    else if(ireg)   
        return RISCVInst::sext_w(new GReg(static_cast<int>(RISCV::GPR::ra)), ireg)+
               RISCVInst::seqz(rd, new GReg(static_cast<int>(RISCV::GPR::ra)));
}

inline ::std::string Feq_s::print(){
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

inline ::std::string Fle_s::print(){
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

inline ::std::string Flt_s::print(){
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

inline ::std::string Fge_s::print(){
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


inline ::std::string Fgt_s::print(){
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

inline ::std::string Fne_s::print(){
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

inline ::std::string Lw::print(){
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
        return RISCVInst::slliw(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), 2)+
               RISCVInst::add(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), base)+
               RISCVInst::lw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

inline ::std::string Lw_label::print(){
    return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
           RISCVInst::lw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
}

inline ::std::string Flw::print(){
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
        return RISCVInst::slliw(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), 2)+
               RISCVInst::add(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), base)+
               RISCVInst::flw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

inline ::std::string Flw_label::print(){
    return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
           RISCVInst::flw(rd, new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
}

inline ::std::string Sw::print(){
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
            return RISCVInst::slliw(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), 2)+
                   RISCVInst::add(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), base)+
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
            return RISCVInst::slliw(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), 2)+
                   RISCVInst::add(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), base)+
                   RISCVInst::sw(dynamic_cast<GReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

inline  ::std::string Sw_label::print(){
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

inline ::std::string Fsw::print(){
    auto iconst_src = dynamic_cast<IConst*>(src);
    auto iconst_offset = dynamic_cast<IConst*>(offset);
    if(iconst_src){
        if(iconst_offset){
            int iconst_offset_value = iconst_offset->getIConst()*4;
            if(iconst_offset_value<-2048 || iconst_offset_value>2047)
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(iconst_src->getIConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::addi(new GReg(static_cast<int>(RISCV::GPR::ra)), base, iconst_offset_value)+
                       RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
            else
                return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(iconst_src->getIConst()))+
                       RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                       RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), base, iconst_offset_value);
        }
        else
            return RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(iconst_src->getIConst()))+
                   RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
                   RISCVInst::slliw(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), 2)+
                   RISCVInst::add(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), base)+
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
            return RISCVInst::slliw(new GReg(static_cast<int>(RISCV::GPR::ra)), dynamic_cast<GReg*>(offset), 2)+
                   RISCVInst::add(new GReg(static_cast<int>(RISCV::GPR::ra)), new GReg(static_cast<int>(RISCV::GPR::ra)), base)+
                   RISCVInst::fsw(dynamic_cast<FReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

inline  ::std::string Fsw_label::print(){
    auto iconst_src = dynamic_cast<IConst*>(src);
    if(iconst_src){
        return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
               RISCVInst::li(new GReg(static_cast<int>(RISCV::GPR::s1)), *(uint32_t*)&(iconst_src->getIConst()))+
               RISCVInst::fmv_s_x(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::s1)))+
               RISCVInst::fsw(new FReg(static_cast<int>(RISCV::FPR::fs1)), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
    else{
        return RISCVInst::la(new GReg(static_cast<int>(RISCV::GPR::ra)), label)+
               RISCVInst::fsw(dynamic_cast<FReg*>(src), new GReg(static_cast<int>(RISCV::GPR::ra)), 0);
    }
}

inline ::std::string Call::print(){
    return RISCVInst::call(label);
}

inline ::std::string La::print(){
    return RISCVInst::la(rd, label);
}

inline ::std::string Mv::print(){
    return RISCVInst::mv(rd, rs1);
}

inline ::std::string Beq::print(){
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


inline ::std::string Bne::print(){
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

inline ::std::string Bge::print(){
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

inline ::std::string Blt::print(){
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

inline ::std::string FBeq::print(){
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



inline ::std::string FBge::print(){
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


inline ::std::string FBgt::print(){
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

inline ::std::string FBle::print(){
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

inline ::std::string FBlt::print(){
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

inline ::std::string FBne::print(){
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

inline ::std::string Jump::print(){
    return RISCVInst::j(label);
}

inline ::std::string Ret::print(){
    return RISCVInst::ret();
}