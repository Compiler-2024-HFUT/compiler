#ifndef TYPE_HPP
#define TYPE_HPP

#include <string>
#include <vector>
#include <iterator>

class Module;
class IntegerType;
class FloatType;
class FunctionType;
class ArrayType;
class PointerType;

class Type {
public:
    enum TypeID {
      VoidTyID,         //// Void
      LabelTyID,        //// Labels, e.g., BasicBlock
      IntegerTyID,      //// Integers, include 32 bits and 1 bit
      FloatTyID,        //// Float
      FunctionTyID,     //// Functions
      ArrayTyID,        //// Arrays
      PointerTyID,      //// Pointer
    };

public:
    explicit Type(TypeID tid, Module *m): tid_(tid), m_(m) {};
    ~Type() = default;

    TypeID getTypeId() const { return tid_; }

    bool isVoidType() const { return getTypeId() == VoidTyID; }
    bool isLabelType() const { return getTypeId() == LabelTyID; }
    bool isIntegerType() const { return getTypeId() == IntegerTyID; }
    bool isFloatType() const { return getTypeId() == FloatTyID; }
    bool isFunction_type() const { return getTypeId() == FunctionTyID; }
    bool isArrayType() const { return getTypeId() == ArrayTyID; }
    bool isPointerType() const { return getTypeId() == PointerTyID; }

    static bool isEqType(Type *ty1, Type *ty2) { return ty1 == ty2; };

    static Type *getVoidType(Module *m);
    static Type *getLabelType(Module *m);
    static IntegerType *getInt1Type(Module *m);
    static IntegerType *getInt32Type(Module *m);
    static PointerType *getInt32PtrType(Module *m);
    static FloatType *getFloatType(Module *m);
    static PointerType *getFloatPtrType(Module *m);
    static PointerType *getPointerType(Type *contained);
    static ArrayType *getArrayType(Type *contained, unsigned num_elements);
    
    Type *getPointerElementType();
    Type *getArrayElementType();

    int getSize();

    Module *getModule() { return m_; }

    std::string print();

private:
    TypeID tid_;
    Module *m_;
};


class IntegerType : public Type {
public:
    explicit IntegerType(unsigned num_bits, Module *m): Type(Type::IntegerTyID, m), num_bits_(num_bits) {}
    
    static IntegerType *get(unsigned num_bits, Module *m);

    unsigned getNumBits();

private:
    unsigned num_bits_;  
};

class FloatType : public Type {
public:
    FloatType(Module *m) : Type(Type::FloatTyID, m) {}
    static FloatType *get(Module *m);

private:
};

class PointerType : public Type {
public:
    PointerType(Type *contained): Type(Type::PointerTyID, contained->getModule()), contained_(contained) {}

    static PointerType *get(Type *contained); 
    
    Type *getElementType() const { return contained_; }

private:
    Type *contained_; //& The element type of the pointer
};


class ArrayType : public Type {
public:
    ArrayType(Type *contained, unsigned num_elements);

    static ArrayType *get(Type *contained, unsigned num_elements);

    static bool isValidElementType(Type *ty);

    Type *getElementType() const { return contained_; }
    unsigned getNumOfElements() const { return num_elements_; }

private:
    Type *contained_;             //& The element type of the array.
    unsigned num_elements_;       //& Number of elements in the array.
};


class FunctionType : public Type {
public:    
    FunctionType(Type *result, std::vector<Type *> params);

    static FunctionType *get(Type *result, std::vector<Type *> params);

    static bool isValidReturnType(Type *ty);
    static bool isValidArgumentType(Type *params);

    unsigned getNumOfArgs() const { return args_.size(); };
    std::vector<Type *>::iterator paramBegin() { return args_.begin(); }
    std::vector<Type *>::iterator paramEnd() { return args_.end(); }

    Type *getParamType(unsigned i) const { return args_[i]; };
    Type *getReturnType() const { return result_; };

private:
    Type *result_;
    std::vector<Type *> args_;
};


#endif