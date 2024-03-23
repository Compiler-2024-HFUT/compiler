#ifndef CONSTANT_HPP
#define CONSTANT_HPP

#include "User.hpp"
#include "Value.hpp"
#include "Type.hpp"
extern Module *global_m_ptr;

class Constant : public User {
public:
    Constant(Type *ty, const std::string &name="", unsigned num_ops = 0) 
        : User(ty, name, num_ops) {}
    ~Constant() = default;
private:

};

class ConstantInt : public Constant {
public:
    static ConstantInt *get(int val, Module *m=global_m_ptr);
    static ConstantInt *get(bool val, Module *m=global_m_ptr);

    static int &getValue(ConstantInt *const_val) { return const_val->val_; }
    int &getValue() { return val_; }

    virtual std::string print() override;

private:
    ConstantInt(Type *ty, int val) : Constant(ty, "", 0), val_(val) {}

private:
    int val_;
};

class ConstantFP : public Constant {

public:
    static ConstantFP *get(float val, Module *m=global_m_ptr);

    float &getValue() { return val_; }
    
    virtual std::string print() override;

private:
    ConstantFP(Type *ty, float val)
        : Constant(ty,"",0), val_(val) {}
private:
    float val_;
};


class ConstantZero : public Constant {
public:
    static ConstantZero *get(Type *ty, Module *m=global_m_ptr);
    
    virtual std::string print() override;
private:
    ConstantZero(Type *ty) : Constant(ty, "", 0) {}
};

class ConstantArray : public Constant {
public:
    ~ConstantArray() = default;

    static ConstantArray *get(ArrayType *ty, const std::vector<Constant *>&vals);

    Constant *getElementValue(int index) { return const_array_[index]; };

    unsigned getSizeOfArray() { return const_array_.size(); }

    virtual std::string print() override;
private:
    ConstantArray(ArrayType *ty, const std::vector<Constant *>&vals);

private:
    std::vector<Constant *> const_array_;
};

#endif