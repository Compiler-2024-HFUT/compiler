#ifndef BUILDER_HPP
#define BUILDER_HPP

#include <vector>
#include <map>

#include "Type.hpp"
#include "Value.hpp"
#include "Function.hpp"
#include "midend/Constant.hpp"


class GlobalVariable;
class Function;
class Instruction;

class IRBuilder {
public:
    explicit IRBuilder();
    ~IRBuilder();

    Type *getVoidType();
    Type *getLabelType();
    IntegerType *getInt1Type();
    IntegerType *getInt32Type();
    FloatType *getFloatType();
    PointerType *getInt32PtrType();
    PointerType *getFloatPtrType();
    PointerType *getPointerType(Type *contained);
    ArrayType *getArrayType(Type *contained, unsigned num_elements);
    FunctionType *getFunctionType(Type *retty, std::vector<Type *>&args);

    void addFunction(Function *f);

    Function *getMainFunction();

    void addGlobalVariable(GlobalVariable *g);

private:
    Type *label_ty_;
    Type *void_ty_;
    IntegerType *int1_ty_;
    IntegerType *int32_ty_;
    FloatType *float32_ty_;

private:
    ConstManager manager_;

private:
    std::map<Type *, PointerType *> pointer_map_;
    std::map<std::pair<Type*, int>, ArrayType*> array_map_;
    std::map<std::pair<Type*, std::vector<Type*>>, FunctionType*> function_map_;
};

#endif