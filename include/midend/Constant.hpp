

#ifndef CONSTANT_HPP
#define CONSTANT_HPP
#include "Type.hpp"
#include "User.hpp"
#include "Value.hpp"

// class User;
class Constant : public User {
  private:
  // int value;
  public:
    Constant(Type *ty, const std::string &name = "", unsigned numops = 0)
      : User(ty, name, numops) {}



};


//零常量
class ConstantZero : public Constant {
  public:
    static ConstantZero *makeConstantZero(Type *ty, Module *m);
    
    virtual std::string print() override;
  private:
    ConstantZero(Type *ty) : Constant(ty, "", 0) {}
};


//整型常量
class ConstantInt : public Constant {
  private:
    int value;
    ConstantInt(Type *ty, int val) : Constant(ty, "", 0), value(val) {}

  public:
    //返回常量值
    static int getValue(ConstantInt *constval) { return constval->value; }

    //返回常量值
    int getValue() { return value; }

    //设置常量值
    void setValue(int val) { value = val; }

    //创建32位整型常量的内存表示
    static ConstantInt *makeInt32Contsant(int val, Module *m);
    static ConstantInt *makeboolContsant(bool val, Module *m);
    virtual std::string print() override;
};


//浮点常量
class ConstantFP : public Constant {

  public:
    static ConstantFP *makeConstantFP(float val, Module *m);

    float getvalue() { return value; }

    virtual std::string print() override;
    //返回常量值
    static int getValue(ConstantFP *constval) { return constval->value; }
    //设置常量值
    void setValue(int val) { value = val; }

    ConstantFP(Type *ty, float val)
            : Constant(ty,"",0), value(val) {}
            
  private:
    float value;
};

//数组常量
class ConstantArray : public Constant {
  private:
    std::vector<Constant *> constArray;    //一个存放指向常量的指针的向量，指针既可以指向整型常量（1维数组），也可以指向数组常量（多维数组）

    ConstantArray(ArrayType *ty, const std::vector<Constant *> &val);

  public:

    //返回指定索引的元素值，索引从0开始
    Constant *getElementValue(int index) { return constArray[index]; }

    //返回数组元素数量
    unsigned getNumElements() const { return constArray.size(); }

    //创建数组的内存表示
    static ConstantArray *makeConstantArray(ArrayType *ty, const std::vector<Constant *> &val);

    virtual std::string print() override;
};

#endif 