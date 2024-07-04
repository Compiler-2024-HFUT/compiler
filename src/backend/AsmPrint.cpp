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
inline ::std::string FRIA::print(){
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
      //  squence_def+=inst->print();
    

    return squence_def;
}



