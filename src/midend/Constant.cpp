#include <iostream>
#include <memory>
#include <sstream>

#include "midend/Constant.hpp"
#include "midend/Module.hpp"

//& ConstantInt
ConstantInt *ConstantInt::get(int val, Module *m) {
    if (m->cached_int.find(val) != m->cached_int.end())
        return m->cached_int[val].get();
    return (m->cached_int[val] =
                std::unique_ptr<ConstantInt>(new ConstantInt(Type::getInt32Type(m), val)))
        .get();
}

ConstantInt *ConstantInt::get(bool val, Module *m) {
    if (m->cached_bool.find(val) != m->cached_bool.end())
        return m->cached_bool[val].get();
    return (m->cached_bool[val] =
                std::unique_ptr<ConstantInt>(new ConstantInt(Type::getInt1Type(m), val ? 1 : 0)))
        .get();
}


std::string ConstantInt::print() {
    std::string const_ir;
    Type *ty = this->getType();
    if (ty->isIntegerType() && static_cast<IntegerType *>(ty)->getNumBits() == 1) {
        const_ir += (this->getValue() == 0) ? "false" : "true";  //&  int1
    } else {
        const_ir += std::to_string(this->getValue());  //& int32
    }
    return const_ir;
}

//& ConstantFP
ConstantFP *ConstantFP::get(float val, Module *m) {
    if (m->cached_float.find(val) != m->cached_float.end())
        return m->cached_float[val].get();
    return (m->cached_float[val] =
                std::unique_ptr<ConstantFP>(new ConstantFP(Type::getFloatType(m), val)))
        .get();
}

std::string ConstantFP::print() {
    std::stringstream fp_ir_ss;
    std::string fp_ir;
    double val = this->getValue();
    fp_ir_ss << "0x" << std::hex << *(uint64_t *)&val << std::endl;
    fp_ir_ss >> fp_ir;
    return fp_ir;
}

//& ConstantZero
ConstantZero *ConstantZero::get(Type *ty, Module *m) {
    if (not m->cached_zero[ty])
        m->cached_zero[ty] = std::unique_ptr<ConstantZero>(new ConstantZero(ty));
    return m->cached_zero[ty].get();
}

std::string ConstantZero::print() { 
    return "zeroinitializer"; 
}

//& ConstantArray
ConstantArray::ConstantArray(ArrayType *ty, const std::map<int, Value *>&vals, unsigned int size) : Constant(ty, "", size) {
    for (int i = 0; i < size; i++){
        if(vals.find(i) != vals.end())
            setOperand(i, vals.find(i)->second);
        else
            setOperand(i, vals.find(-1)->second);
    }
    array_size = size;
    this->init_val_map = vals;
}

ConstantArray *ConstantArray::get(ArrayType *ty, const std::map<int, Value *>&vals_map, unsigned int size) {
    return new ConstantArray(ty, vals_map, size);
}

std::string ConstantArray::print() {
    std::string const_ir;
    const_ir += "[";
    const_ir += this->getType()->getArrayElementType()->print();
    const_ir += " ";
    const_ir += getElementValue(0)->print();
    for ( int i = 1 ; i < this->getSizeOfArray() ; i++ ){
        const_ir += ", ";
        const_ir += this->getType()->getArrayElementType()->print();
        const_ir += " ";
        const_ir += getElementValue(i)->print();
    }
    const_ir += "]";
    return const_ir;
}
