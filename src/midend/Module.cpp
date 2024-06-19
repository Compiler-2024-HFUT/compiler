#include <memory>

#include "analysis/InfoManager.hpp"
#include "midend/IRBuilder.hpp"
#include "midend/Module.hpp"

Module::Module(std::string name) : module_name_(name) ,builder_(std::make_unique<IRBuilder>()),
info_man_(std::make_unique<InfoManager>(this)),
//init instr_id2string 
instr_id2string_{
    {Instruction::OpID::ret, "ret"},
    {Instruction::OpID::br, "br"},
    {Instruction::OpID::add, "add"},
    {Instruction::OpID::sub, "sub"},
    {Instruction::OpID::mul, "mul"},
    {Instruction::OpID::mul64, "mul64"},
    {Instruction::OpID::sdiv, "sdiv"},
    {Instruction::OpID::srem, "srem"},
    {Instruction::OpID::fadd, "fadd"},
    {Instruction::OpID::fsub, "fsub"},
    {Instruction::OpID::fmul, "fmul"},
    {Instruction::OpID::fdiv, "fdiv"},
    {Instruction::OpID::alloca, "alloca"},
    {Instruction::OpID::load, "load"},
    {Instruction::OpID::store, "store"},
    {Instruction::OpID::memset, "memset"},
    {Instruction::OpID::cmp, "icmp"},
    {Instruction::OpID::fcmp, "fcmp"},
    {Instruction::OpID::phi, "phi"},
    {Instruction::OpID::call, "call"},
    {Instruction::OpID::getelementptr, "getelementptr"},
    {Instruction::OpID::land, "and"},
    {Instruction::OpID::lor, "or"},
    {Instruction::OpID::lxor, "xor"},
    {Instruction::OpID::asr, "ashr"},
    {Instruction::OpID::shl, "shl"},
    {Instruction::OpID::lsr, "lshr"},
    {Instruction::OpID::asr64, "asr64"},
    {Instruction::OpID::shl64, "shl64"},
    {Instruction::OpID::lsr64, "lsr64"},
    {Instruction::OpID::zext, "zext"},
    {Instruction::OpID::sitofp, "sitofp"},
    {Instruction::OpID::fptosi, "fptosi"},
    {Instruction::OpID::cmpbr, "cmpbr"},
    {Instruction::OpID::fcmpbr, "fcmpbr"},
    {Instruction::OpID::loadoffset, "loadoffset"},
    {Instruction::OpID::storeoffset, "storeoffset"},
    }
{
}

Function* Module::getMainFunction() {
    return *(functions_list_.rbegin());
}
void Module::deleteFunction(Function*f) {
    functions_list_.remove(f);
}

Module::~Module() {
}

void Module::addFunction(Function *f) {
    functions_list_.push_back(f);
}

void Module::addGlobalVariable(GlobalVariable *g) {
    globals_list_.push_back(g);
}

void Module::setPrintName() {
    for (auto &func : this->getFunctions()) {
        func->setInstrName();
    }
}

std::string Module::print() {
    std::string module_ir;
    for (auto &global_val : this->globals_list_) {
        module_ir += global_val->print();
        module_ir += "\n";
    }
    module_ir += "\n";
    for (auto &func : this->functions_list_) {
        module_ir += func->print();
        module_ir += "\n";
    }
    return module_ir;
}
__attribute__((always_inline)) InfoManager *Module::getInfoMan(){return info_man_.get();}