#include "backend/HAsmModule.hpp"

#include "backend/HAsmVal.hpp"
#include "backend/HAsm2Asm.hpp"

HAsmFunc *HAsmModule::create_func(Function *func) {
    auto new_func = new HAsmFunc(this, func);
    funcs_list_.push_back(new_func);
    return new_func;
}

std::string HAsmModule::get_asm_code() {
    std::string asm_code;

    asm_code += hasm_header_;
    if(!hasm_data_section_def_.empty()) {
        asm_code += HAsm2Asm::space + ".text" + HAsm2Asm::newline;
        for(auto data_def: hasm_data_section_def_) {
            asm_code += data_def;
        }
    }
    if(!funcs_list_.empty()) {
        asm_code += HAsm2Asm::space + ".text" + HAsm2Asm::newline;
        for(auto func: funcs_list_) {
            asm_code += func->get_asm_code();
        }
    }
    return asm_code;
}


std::string HAsmModule::print() {
    std::string hasm_code;

    hasm_code += hasm_header_;
    if(!hasm_data_section_def_.empty()) {
        hasm_code += HAsm2Asm::space + ".text" + HAsm2Asm::newline;
        for(auto data_def: hasm_data_section_def_) {
            hasm_code += data_def;
        }
    }
    if(!funcs_list_.empty()) {
        hasm_code += HAsm2Asm::space + ".text" + HAsm2Asm::newline;
        for(auto func: funcs_list_) {
            hasm_code += func->print();
        }
    }
    return hasm_code;
}