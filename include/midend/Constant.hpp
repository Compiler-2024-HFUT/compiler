#ifndef CONSTANT_HPP
#define CONSTANT_HPP

#include <cassert>
#include <cstdint>
#include <map>

#include "User.hpp"
#include "Value.hpp"
#include "midend/Instruction.hpp"
#include <memory>

class Type;
class ArrayType;
class ConstantInt;
class ConstantFP;
class ConstantZero;
struct ConstManager{
public:
    std::unordered_map<int,std::unique_ptr<ConstantInt>> cached_int;
    std::unordered_map<bool, std::unique_ptr<ConstantInt>> cached_bool;
    std::unordered_map<float, std::unique_ptr<ConstantFP>> cached_float;
    std::unordered_map<Type *, std::unique_ptr<ConstantZero>> cached_zero;
};
class Constant : public User {
public:
    static ConstManager *manager_;
    static Constant *get(Constant *lhs,Instruction::OpID bin_op,Constant*rhs);
    Constant(Type *ty, const std::string &name="", unsigned num_ops = 0) 
        : User(ty, name, num_ops) {}
    ~Constant() = default;
private:

};
class ConstantInt : public Constant {
public:
    static ConstantInt *get(int val);
    static ConstantInt *get(bool val);

    static ConstantInt *getFromBin(ConstantInt *lhs,Instruction::OpID bin_op,ConstantInt*rhs);
    static ConstantInt *getFromCmp(Constant *lhs,CmpOp,Constant*rhs);
    static ConstantInt *getFromICmp(ConstantInt *lhs,CmpOp,ConstantInt*rhs);
    static ConstantInt *getFromFCmp(ConstantFP *lhs,CmpOp,ConstantFP*rhs);
    static int getValue(ConstantInt *const_val) { return const_val->val_; }
    int getValue() { return val_; }

    virtual std::string print() override;
    virtual ~ConstantInt(){}

private:
    ConstantInt(Type *ty, int val) : Constant(ty, "", 0), val_(val) {}

private:
    int val_;
};

class ConstantFP : public Constant {

public:
    static ConstantFP *get(float val);
    static ConstantFP *getFromBin(ConstantFP *lhs,Instruction::OpID bin_op,ConstantFP*rhs);

    float getValue() { return val_; }
    
    virtual std::string print() override;
    virtual ~ConstantFP(){}

private:
    ConstantFP(Type *ty, float val)
        : Constant(ty,"",0), val_(val) {}
private:
    float val_;
};


class ConstantZero : public Constant {
public:
    static ConstantZero *get(Type *ty);
    
    virtual std::string print() override;
private:
    ConstantZero(Type *ty) : Constant(ty, "", 0) {}
};

class ConstantArray : public Constant {
public:
    ~ConstantArray() = default;

    static ConstantArray *get(ArrayType *ty, const std::map<int, Value *>&vals_map, unsigned int size);

    Constant *getElementValue(int index) { 
        if(init_val_map[index]){
            return dynamic_cast<Constant*>(init_val_map[index]);
        }
        return dynamic_cast<Constant*>(init_val_map[-1]);
    };

    unsigned getSizeOfArray() { return array_size; }

    virtual std::string print() override;
private:
    ConstantArray(ArrayType *ty, const std::map<int, Value *>&vals, unsigned int size);

private:
    std::map<int, Value *> init_val_map;
    int array_size;
};



#endif