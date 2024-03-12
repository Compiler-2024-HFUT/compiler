#ifndef GLOBALVARIABLE_HPP
#define GLOBALVARIABLE_HPP

#include "User.hpp"
#include "Module.hpp"
class Constant;
class GlobalVariable : public User {
  private:
    bool is_const;       //指示全局变量是否是常量 
    Constant *initval;          //表示全局变量的初始值
    GlobalVariable(std::string name, Module *m, Type *ty, bool isconst,
                    Constant *init = nullptr);

  public:
    //创建全局变量的内存表示
    GlobalVariable *makeGlobalVariable(std::string name, Module *m, Type *ty,
                                    bool isconst, Constant *init);

    //返回该全局变量的值
    Constant *getInit() { return initval; }
    bool isConst() { return is_const; }
    bool isZeroInit() { return dynamic_cast<ConstantZero*>(initval) != nullptr; }
    std::string print();
};
#endif 
