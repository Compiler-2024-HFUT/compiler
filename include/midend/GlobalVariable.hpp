#ifndef GLOBALVARIABLE_HPP
#define GLOBALVARIABLE_HPP

#include "Constant.hpp"
#include "Module.hpp"
#include "User.hpp"

class GlobalVariable : public User {
  private:
    bool isConst;       //指示全局变量是否是常量 
    Constant *initval;          //表示全局变量的初始值
    GlobalVariable(std::string name, Module *m, Type *ty, bool isconst,
                    Constant *init = nullptr);

  public:
    //创建全局变量的内存表示
    GlobalVariable *makeGlobalVariable(std::string name, Module *m, Type *ty,
                                    bool isconst, Constant *init);

    //返回该全局变量的值
    Constant *getInit() { return initval; }
    bool isConst() { return isConst; }
    bool isZeroInit() { return dynamic_cast<ConstantZero*>(initval) != nullptr; }
    std::string print();
};
#endif 
