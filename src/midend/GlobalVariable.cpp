#include "midend/Module.hpp"
#include "midend/GlobalVariable.hpp"


GlobalVariable::GlobalVariable(std::string name, Module *m, Type *ty, bool is_const, Constant *init)
    : User(ty, name, init != nullptr), is_const_(is_const), init_val_(init) {
    m->addGlobalVariable(this);
    if (init) {
        this->setOperand(0, init);
    }
} 

GlobalVariable *GlobalVariable::create(std::string name, Module *m, Type *ty,
                                       bool is_const,
                                       Constant *init = nullptr) {
  return new GlobalVariable(name, m, PointerType::get(ty), is_const, init);
}

std::string GlobalVariable::print() {
    std::string global_val_ir;
    global_val_ir += "@" + this->getName();
    global_val_ir += " = ";
    global_val_ir += (this->isConst() ? "constant " : "global ");
    global_val_ir += this->getType()->getPointerElementType()->print();
    global_val_ir += " ";
    global_val_ir += this->getInit()->print();
    global_val_ir += "\n";
    return global_val_ir;
}