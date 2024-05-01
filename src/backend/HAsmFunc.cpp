#include "backend/HAsmFunc.hpp"
#include "backend/HAsmVal.hpp"
#include "backend/HAsm2Asm.hpp"

//#include "logging.hpp"

HAsmBlock *HAsmFunc::create_bb(BasicBlock *bb, Label *label) { 
    auto new_bb = new HAsmBlock(this, bb, label);
    blocks_list_.push_back(new_bb);
    return new_bb;
}

std::string HAsmFunc::get_asm_code() {
    std::string asm_code;
    asm_code += func_header_;
    asm_code += f_->getName() + ":" + HAsm2Asm::newline;
    for(auto bb: blocks_list_) {
        asm_code += bb->get_asm_code();
    }
    asm_code += HAsm2Asm::space + ".size" + HAsm2Asm::space + f_->getName() + ", " + ".-" + f_->getName() + HAsm2Asm::newline;
    return asm_code;
}


std::string HAsmFunc::print() {
    std::string hasm_code;
    hasm_code += func_header_;
    hasm_code += f_->getName() + ":" + HAsm2Asm::newline;
    for(auto bb: blocks_list_) {
        hasm_code += bb->print();
    }
    hasm_code += HAsm2Asm::space + ".size" + HAsm2Asm::space + f_->getName() + ", " + ".-" + f_->getName() + HAsm2Asm::newline;
    return hasm_code;
}