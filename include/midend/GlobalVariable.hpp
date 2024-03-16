#ifndef GLOBALVARIABLE_HPP
#define GLOBALVARIABLE_HPP

#include "Module.hpp"
#include "User.hpp"
#include "Constant.hpp"

class GlobalVariable : public User {
public:
    virtual ~GlobalVariable() = default;

    static GlobalVariable *create(std::string name, Module *m, Type *ty, bool is_const, Constant *init);

    Constant *getInit() { return init_val_; }
    bool isConst() { return is_const_; }

    bool isZeroInitializer() { return dynamic_cast<ConstantZero*>(init_val_) != nullptr; }
    
    std::string print();

private:
    GlobalVariable(std::string name, Module *m, Type *ty, bool is_const, Constant *init = nullptr);

private:
    bool is_const_;
    Constant *init_val_;
};


#endif