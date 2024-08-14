#include <cassert>

#include "midend/Type.hpp"
#include "midend/IRBuilder.hpp"
IRBuilder* Type::builder=nullptr;
Type *Type::getVoidType() {
    return Type::builder->getVoidType();
}

Type *Type::getLabelType() {
    return Type::builder->getLabelType();
}

IntegerType *Type::getInt1Type() {
    return Type::builder->getInt1Type();
}

IntegerType *Type::getInt32Type() {
    return Type::builder->getInt32Type();
}

PointerType *Type::getInt32PtrType() {
    return Type::builder->getInt32PtrType();
}

FloatType *Type::getFloatType() {
    return Type::builder->getFloatType();
}

PointerType *Type::getFloatPtrType() {
    return Type::builder->getFloatPtrType();
}

PointerType *Type::getPointerType(Type *contained) {
    return PointerType::get(contained); 
}

ArrayType *Type::getArrayType(Type *contained, unsigned num_elements) {
    return ArrayType::get(contained, num_elements);
}

Type *Type::getPointerElementType() {
    if (this->isPointerType())
        return static_cast<PointerType *>(this)->getElementType();
    else
        return nullptr;
}

Type *Type::getArrayElementType() {
    if (this->isArrayType())
        return static_cast<ArrayType *>(this)->getElementType();
    else
        return nullptr;
}


int Type::getSize() {
    if (this->isIntegerType()) {
        auto bits = static_cast<IntegerType *>(this)->getNumBits() / 8;
        return bits > 0 ? bits : 1;
    }
    if (this->isArrayType()) {
        auto element_size = static_cast<ArrayType *>(this)->getElementType()->getSize();
        auto num_elements = static_cast<ArrayType *>(this)->getNumOfElements();
        return element_size * num_elements;
    }
    if (this->isPointerType()) {
        //! RV64 指针类型8字节
        return 8;
    }
    if (this->isFloatType()) {
        return 4;
    }
    return 0;
}

std::string Type::print() {
    std::string type_ir;
    switch (this->getTypeId()) {
        case VoidTyID: type_ir += "void"; break;
        case LabelTyID: type_ir += "label"; break;
        case IntegerTyID:
            type_ir += "i";
            type_ir += std::to_string(static_cast<IntegerType *>(this)->getNumBits());
            break;
        case FunctionTyID:
            type_ir += static_cast<FunctionType *>(this)->getReturnType()->print();
            type_ir += " (";
            for (int i = 0; i < static_cast<FunctionType *>(this)->getNumOfArgs(); i++) {
                if (i)
                    type_ir += ", ";
                type_ir += static_cast<FunctionType *>(this)->getParamType(i)->print();
            }
            type_ir += ")";
            break;
        case PointerTyID:
            type_ir += this->getPointerElementType()->print();
            type_ir += "*";
            break;
        case ArrayTyID:
            type_ir += "[";
            type_ir += std::to_string(static_cast<ArrayType *>(this)->getNumOfElements());
            type_ir += " x ";
            type_ir += static_cast<ArrayType *>(this)->getElementType()->print();
            type_ir += "]";
            break;
        case FloatTyID: type_ir += "float"; break;
    default: break;
    }
    return type_ir;
}

//& IntegerType 

IntegerType *IntegerType::get(unsigned num_bits) {
    if (num_bits == 1) {
        return Type::builder->getInt1Type();
    } else if (num_bits == 32) {
        return Type::builder->getInt32Type();
    } else {
        assert(false and "IntegerType::get has error num_bits");
        return nullptr;
    }
}

unsigned IntegerType::getNumBits() { 
    return num_bits_; 
}

//& FloatType

FloatType *FloatType::get() {
    return Type::builder->getFloatType();
}

//& PointerType 
PointerType *PointerType::get(Type *contained) {
    return Type::builder->getPointerType(contained);
}

//& ArrayType
ArrayType::ArrayType(Type *contained, unsigned num_elements)
    : Type(Type::ArrayTyID), num_elements_(num_elements) {
    assert(isValidElementType(contained) && "Not a valid type for array element!");
    contained_ = contained;
}

ArrayType *ArrayType::get(Type *contained, unsigned num_elements) {
    return Type::builder->getArrayType(contained, num_elements);
}

bool ArrayType::isValidElementType(Type *ty) {
    return ty->isIntegerType() || ty->isArrayType() || ty->isFloatType();
}

//& FunctionType

FunctionType::FunctionType(Type *result, std::vector<Type *> params) : Type(Type::FunctionTyID) {
    assert(isValidReturnType(result) && "Invalid return type for function!");
    result_ = result;

    for (auto p : params) {
        assert(isValidArgumentType(p) && "Not a valid type for function argument!");
        args_.push_back(p);
    }
}

FunctionType *FunctionType::get(Type *result, std::vector<Type *> params) {
    return Type::builder->getFunctionType(result, params);
}

bool FunctionType::isValidReturnType(Type *ty) {
    return ty->isIntegerType() || ty->isVoidType() || ty->isFloatType()||ty->isPointerType() ;
}

bool FunctionType::isValidArgumentType(Type *ty) {
    return ty->isIntegerType() || ty->isPointerType() || ty->isFloatType();
}
