//函数类，里面包含若干个BB
#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <cassert>
#include <cstddef>
#include <iterator>
#include <list>
#include <map>

#include "BasicBlock.hpp"
#include "User.hpp"
#include "Module.hpp"
#include "Type.hpp"

#include "User.hpp"
#include <unordered_map>



//函数类
class Function : public Value {
  public:
    Function(FuncType *ty, const std::string &name, Module *parent);

    static Function *makeFunc(FuncType *ty, const std::string &name,Module *parent);

    //返回函数类型
    FuncType *getFuncType() const { return static_cast<FuncType *>(getType()); }

    //返回函数的返回值类型
    Type *getResultType() const;

    //向BB表中加BB
    void addBasicBlock(BasicBlock *bb);

    //这个函数的作用是在给定位置之后插入一个新的基本块，用于在函数中控制流的改变，例如在特定条件下执行不同的代码块。
    void addBasicBlockAfter(std::list<BasicBlock *>::iterator afterpos,
                            BasicBlock *bb);

    //返回函数的参数个数
    unsigned getNumArgs() const;


    //返回BB个数，注意只是BB个数，除了BB（顺序块），还有其他块！！！
    unsigned getNumBasicBlocks() const { return basicBlocks.size(); }

    //返回该函数的Module
    Module *getParent() const { return parent; }

    //返回BB表
    std::list<BasicBlock *> &getBasicBlocks() { return basicBlocks; }

    //返回参数表
    std::list<Argument *> &getArgs() { return arguments; }

    //返回该函数的首块（第一个块）
    BasicBlock *getEntryBlock() { return *basicBlocks.begin(); }

    //argbegin()返回参数列表的起始迭代器，argend()返回参数列表的结束迭代器，这样可以方便地遍历整个参数列表。
    std::list<Argument *>::iterator argbegin() { return arguments.begin(); }
    std::list<Argument *>::iterator argend() { return arguments.end(); }

    //删除指定BB
    void removeBasicBlock(BasicBlock *bb);

    bool declaration() { return basicBlocks.empty(); }

    void setInstrString();

  // void HighIRprint();
    std::string print();
    

  private:
    //创建参数的内存表示
    void makeArgs();


    std::list<BasicBlock *> basicBlocks; //BB表
    std::list<Argument *> arguments;      //参数表
    Module *parent;                      //存放该函数的Module，函数定义在Module里
    int printCnt;                       //记录打印该函数的次数
    std::vector<Argument*> iArgs;
    std::vector<Argument*> fArgs;

  //std::list<BaseBlock *> baseblocks; //base block表，因为一个函数里可能还有if、while块，而BB只是顺序块，所以这里是base block

 // bool multithreading = false;        //多线程默认关

//public:
  //从base blocks中删除指定base blocks
  //void removeBaseBlock(BaseBlock *bb) { baseblocks.remove(bb); }

  //向base blocks 中加入base block
  //void addBaseBlock(BaseBlock *basebb);

  //向base  blocks插入base block
 // void insertBaseBlock(std::list<BaseBlock *>::iterator iter,
  //                     BaseBlock *basebb) {
   // baseblocks.insert(iter, basebb);
   // basebb->clearFather();//为什么？？？？？？？？？？？？
 // }

  //返回base blocks
  //std::list<BaseBlock *> &getBaseBlocks() { return baseblocks; }

  //设置多线程状态
  //void setMultithreading(bool stat) { multithreading = stat; }
  
  //返回多线程状态
  //bool getMultithreading() { return multithreading; }
  
  //清空计数器
  //void clearCnt() { printcnt = 0; }
};








// Argument of Function, does not contain actual value
class Argument : public Value {
  public:
    /// Argument constructor.
    Argument(Type *ty, const std::string &name = "",
                      Function *f = nullptr, unsigned argpos = 0)
        : Value(ty, name), parent(f), argPos(argpos) {}

    inline const Function *getParent() const { return parent; }
    inline Function *getParent() { return parent; }

    /// For example in "void foo(int a, float b)" a is 0 and b is 1.
    unsigned getArgNo() const {
        assert(parent&& "can't get number of unparented arg");
        return argPos;
    }


    virtual std::string print() override;

    //设置数组边界为参数给出的数组边界
    void setArrayBound(std::vector<Value *> &arraybound) {
      arrayBound.assign(arraybound.begin(), arraybound.end());
    }

    //返回数组边界
    std::vector<Value *> &getArrayBound() { return arrayBound; }

  private:
    Function *parent;  //指向当前Fucntion的父对象
    unsigned argPos; // argument No.
    std::vector<Value *> arrayBound;  //数组边界，我猜测可能是数组的各维的最大值（边界）


};

#endif // SYSYCFUNCTIONH