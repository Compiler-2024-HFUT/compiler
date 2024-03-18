#include <cassert>

#include "midend/Type.hpp"
#include "midend/Module.hpp"

Type *Type::getVoidType(Module *m) {
    return m->getVoidType();
}

Type *Type::getLabelType(Module *m) {
    return m->getLabelType();
}

IntegerType *Type::getInt1Type(Module *m) {
    return m->getInt1Type();
}

IntegerType *Type::getInt32Type(Module *m) {
    return m->getInt32Type();
}

PointerType *Type::getInt32PtrType(Module *m) {
    return m->getInt32PtrType();
}

FloatType *Type::getFloatType(Module *m) {
    return m->getFloatType();
}

PointerType *Type::getFloatPtrType(Module *m) {
    return m->getFloatPtrType();
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

IntegerType *IntegerType::get(unsigned num_bits, Module *m) {
    if (num_bits == 1) {
        return m->getInt1Type();
    } else if (num_bits == 32) {
        return m->getInt32Type();
    } else {
        assert(false and "IntegerType::get has error num_bits");
        return nullptr;
    }
}

unsigned IntegerType::getNumBits() { 
    return num_bits_; 
}

//& FloatType

FloatType *FloatType::get(Module *m) {
    return m->getFloatType();
}

//& PointerType 
PointerType *PointerType::get(Type *contained) {
    return contained->getModule()->getPointerType(contained);
}

//& ArrayType
ArrayType::ArrayType(Type *contained, unsigned num_elements)
    : Type(Type::ArrayTyID, contained->getModule()), num_elements_(num_elements) {
    assert(isValidElementType(contained) && "Not a valid type for array element!");
    contained_ = contained;
}

ArrayType *ArrayType::get(Type *contained, unsigned num_elements) {
    return contained->getModule()->getArrayType(contained, num_elements);
}

bool ArrayType::isValidElementType(Type *ty) {
    return ty->isIntegerType() || ty->isArrayType() || ty->isFloatType();
}

//& FunctionType

FunctionType::FunctionType(Type *result, std::vector<Type *> params) : Type(Type::FunctionTyID, nullptr) {
    assert(isValidReturnType(result) && "Invalid return type for function!");
    result_ = result;

    for (auto p : params) {
        assert(isValidArgumentType(p) && "Not a valid type for function argument!");
        args_.push_back(p);
    }
}

FunctionType *FunctionType::get(Type *result, std::vector<Type *> params) {
    return result->getModule()->getFunctionType(result, params);
}

bool FunctionType::isValidReturnType(Type *ty) {
    return ty->isIntegerType() || ty->isVoidType() || ty->isFloatType();
}

bool FunctionType::isValidArgumentType(Type *ty) {
    return ty->isIntegerType() || ty->isPointerType() || ty->isFloatType();
}
