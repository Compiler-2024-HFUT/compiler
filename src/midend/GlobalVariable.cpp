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
    return    
        "@" + this->getName()+ " = "+ (this->isConst() ? "constant " : "global ")+ 
        this->getType()->getPointerElementType()->print()+ " "+ this->getInit()->print()+ "\n";
}