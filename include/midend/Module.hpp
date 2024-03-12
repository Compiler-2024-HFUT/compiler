//编译单元Module，对应一个源文件
#ifndef MODULE_HPP
#define MODULE_HPP

#include <list>
#include <map>
#include <string>
#include <memory>

#include "Function.hpp"
#include "Value.hpp"
#include "Instruction.hpp"
class GlobalVariable;

class Pairsearch {
  public:
      template <typename T>
      std::size_t operator()(const std::pair<T, Module *> val) const {
          auto lhs = std::hash<T>()(val.first);
          auto rhs = std::hash<uintptr_t>()(reinterpretcast<uintptr_t>(val.second));
          return lhs ^ rhs;
      }
};

class Module {
  public:
    //enum IRLeval { HIR, MIRMEM, MIRSSA, LIR };

    Module(std::string name);


    //Module的构造函数会创建Module的私有属性中的若干种类型的内存表示
    //下面的函数即为返回这些类型的内存表示

    Type *getVoidType(Module *m);
    Type *getLabelType(Module *m);
    IntType *getInt1Type(Module *m);
    IntType *getInt32Type(Module *m);
    PtrType *getInt32PtrType(Module *m);
    FloatType *getFloatType(Module *m);
    PtrType *getFloatPtrType(Module *m);
    PtrType *getPtrType(Type *Ty);
    ArrayType *getArrayType(Type *Ty, unsigned numelements);
    FuncType *getFuncType(Type *retTy, std::vector<Type *>&argsTy);

    //向函数表中加入函数f
    void addFunc(Function *f);

    //从函数表中删除函数f
    void removeFunc(Function *f) { funcList.remove(f); }

    //删除全局变量表中的全局变量v
    void removeGlobalVariable(GlobalVariable *v) { globalList.remove(v); }

    //返回函数表
    std::list<Function *> &getFuncList() { return funcList; }

    //返回全局变量表
    std::list<GlobalVariable *> &getGlobalList() { return globalList; }

    //返回符号表
    std::map<std::string, Value *> &getSymbolTable() { return symbolTable; }

    //返回Module名
    std::string getModuleName() { return moduleName; }

    //返回该Module对应的源文件名
    std::string getSourceFileName() { return sourceFileName; }

    void setFileName(std::string name) { sourceFileName = name; }

    //向全局变量表中加入全局变量g
    void addGlobalVariable(GlobalVariable *g);

    //获得指定指令对应的字符串形式
    std::string getInstrString(Instruction::OPID instr) { return instr2string[instr]; }

    //打印HIR
  // void HighIRprint();



    void setPrintName();

    //返回main函数
    Function *getMainFunc() {
      for (auto f : funcList) {
        if (f->getName() == "main") {
          return f;
        }
      }
      // assert(! "Can't find main");
    }

    //返回指定函数名的函数
    Function *getFunc(std::string name) {
      for (auto f : funcList) {
        if (f->getName() == name) {
          return f;
        }
      }
    
    }
    std::unordered_map<std::pair<int, Module *>, std::unique_ptr<ConstantInt>, Pairsearch> cachedInt;
    std::unordered_map<std::pair<bool, Module *>, std::unique_ptr<ConstantInt>, Pairsearch> cachedBool;
    std::unordered_map<std::pair<float, Module *>, std::unique_ptr<ConstantFP>, Pairsearch> cachedFloat;
    std::unordered_map<Type *, std::unique_ptr<ConstantZero>> cachedZero;

    //设置该Module的当前IR级别为指定IR级别
    //void setIRLevel(IRLeval level) { irlevel = level; }

    //返回该Module当前的IR级别
    //IRLeval getIRLevel() { return irlevel; }

    //判断该Module的IR是否是SSA形式的MIR
    //bool isMIRSSALevel() { return irlevel == MIRSSA; }
        virtual std::string print();

  private:
    std::list<GlobalVariable *> globalList;                     //该Module的全局变量表
    std::list<Function *> funcList; //该Module的函数表
    std::map<std::string, Value *> symbolTable; //符号表，元素是Value型，由名字查value
    
    std::map<Instruction::OPID, std::string> instr2string;  //从指令码到字符串

    std::map<Type *, PtrType *> ptrMap;
    std::map<std::pair<Type*, int>, ArrayType*> arrayMap;
    std::map<std::pair<Type*, std::vector<Type*>>, FuncType*> funcMap;
  
    std::string moduleName;      //Module名
    std::string sourceFileName; // 该Module对应的源文件名，用于测试与debug

  // IRLeval irlevel = HIR;  //HIR级IR
  //当Module被创建时，其构造函数将创建以下5种类型的内存表示
    IntType *int1Ty;    //1位整型
    IntType *int32Ty;   //32位整型
    Type *labelTy;          //标号
    Type *voidTy;           //void
    PtrType *int32ptrTy;    //32位整型指针
};

#endif // SYSYCMODULEH