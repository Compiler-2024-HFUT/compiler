#ifndef TYPE_HPP
#define TYPE_HPP

#include "midend/BasicBlock.hpp"
#include <string>
#include <vector>
#include <iterator>


class IntType;
class PtrType;
class FloatType;
class PtrType;
class PtrType;
class ArrayType;
class Module;
class Type{  
  public:
    enum TypeId{
        //基本类型
        VOIDTYPEID,   //void
        LABELTYPEID,  //标号
        INTTYPEID,    //整型，包括1bit和32bit
        FLOATTYPEID,
        
        //组合类型
        FUNCTYPEID,   //函数
        ARRAYTYPEID,  //数组
        PTRTYPEID,    //指针
    };

    Type(TypeId id);

    //返回类型是哪种
    TypeId getTypeID() const { return typeId; }

    //判断类型是否是特定类型
    bool isVoidType() const { return getTypeID() == VOIDTYPEID; }
    bool isLabelType() const { return getTypeID() == LABELTYPEID; }
    bool isIntegerType() const { return getTypeID() == INTTYPEID; }
    bool isFloatType() const { return getTypeID() == FLOATTYPEID; }
    bool isFunctionType() const { return getTypeID() == FUNCTYPEID; }
    bool isArrayType() const { return getTypeID() == ARRAYTYPEID; }
    bool isPointerType() const { return getTypeID() == PTRTYPEID; }

    //对于整型，额外判定是1位还是32位
    bool isInt1();
    bool isInt32();

    bool isEqType(Type *type1, Type *type2) { return type1 == type2; };
    
    //下面函数返回Module中若干种类型的内存表示，详情见Module类
    static Type *getVoidType(Module *m);
    static Type *getLabelType(Module *m);
    static IntType *getInt1Type(Module *m);
    static IntType *getInt32Type(Module *m);
    static PtrType *getInt32PtrType(Module *m);
    static FloatType *getFloatType(Module *m);
    static PtrType *getFloatPtrType(Module *m);
    static PtrType *getPtrType(Type *Ty);
    static ArrayType *getArrayType(Type *Ty, unsigned numelements);
 
    //返回指针或数组元素的类型
    static Type *getPtrElementType();
    static Type *getArrayElementType();

    //获得整型、数组类型或指针类型的值大小
    //整型：1位或32位
    //数组类型：数组元素数量*元素大小
    //指针类型：4或指针元素类型大小
    int getSize();

    std::string print();

  private:
    TypeId typeId;
};

//整型
class IntType : public Type {
  public:
    IntType(unsigned numBits);

    IntType *makeIntType(unsigned numBits);

    //返回该整型值的位数：1位或32位
    unsigned getBits();

  private:
    unsigned bits;
};

//浮点型
class FloatType : public Type {
  public:
    FloatType();
    FloatType *makeFloatType();

};


//指针类型
class PtrType : public Type {
  public:
    PtrType(Type *ptrTy);

    //返回指针指向元素的类型
    Type *getElementType() const { return ptrType; }

    //创建指针类型的内存表示
    PtrType *makePtrType(Type *ptrTy);

  private:
    Type *ptrType; //指针指向内容的类型
};

//数组类型
class ArrayType : public Type {
  public:
    ArrayType(Type *elementTy, unsigned numelements);
    //检查数组元素类型是否合法：整型或数组类型（即多维数组）
    bool isValidElementType(Type *ty);
    //创建数组类型的内存表示
    ArrayType *makeArrayType(Type *elementTy, unsigned numelements);
    //返回数组的元素类型
    Type *getElementType() const { return elementType; }
    
    //返回数组的元素数量
    unsigned getNumElements() const { return numElements; }
    
    //返回数组的内情信息。返回一个向量，其元素是每一维度的大小
    //比如：a[2][3][4]，则返回<2,3,4>
    std::vector<unsigned> getDims() const;

  private:
    Type *elementType;       //数组元素的类型
    unsigned numElements; //数组的元素数量
};

//函数类型
class FuncType : public Type {
  public:
    FuncType(Type *resultTy, std::vector<Type *> paramsTy);

    //检查函数的返回值类型是否合法：void或整型
    bool isValidReturnType(Type *ty);

    //检查函数的参数类型是否合法：整型或指针类型
    bool isValidArgumentType(Type *ty);

    //创建一个函数类型的内存表示
    static FuncType *makeFuncType(Type *resultTy, std::vector<Type *> paramsTy);

    //返回参数列表中元素的数量
    unsigned getNumArgs() const { return argsType.size(); };

    //返回参数列表中指定位置处参数的类型（整型或指针类型）
    Type *getArgType(unsigned i) const { return argsType[i]; };

    //返回返回值类型（void或整型）
    Type *getResultType() const { return resultType; };

    std::vector<Type *>::iterator parambegin() { return argsType.begin(); }
    std::vector<Type *>::iterator paramend() { return argsType.end(); }

  private:
    Type *resultType;              //返回值类型
    std::vector<Type *> argsType;  //参数列表
};
#endif