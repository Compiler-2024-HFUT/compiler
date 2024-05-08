#ifndef CONSTANT_HPP
#define CONSTANT_HPP

#include <map>

#include "User.hpp"
#include "Value.hpp"
#include "Type.hpp"
#include <memory>

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
    Constant(Type *ty, const std::string &name="", unsigned num_ops = 0) 
        : User(ty, name, num_ops) {}
    ~Constant() = default;
private:

};
class ConstantInt : public Constant {
public:
    static ConstantInt *get(int val);
    static ConstantInt *get(bool val);

    static int &getValue(ConstantInt *const_val) { return const_val->val_; }
    int &getValue() { return val_; }

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

    float &getValue() { return val_; }
    
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