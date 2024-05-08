#include <iostream>
#include <memory>
#include <sstream>

#include "midend/Constant.hpp"
#include "midend/Module.hpp"
//& ConstantInt
ConstManager *Constant::manager_{nullptr};
ConstantInt *ConstantInt::get(int val) {
    if (Constant::manager_->cached_int.find(val) != Constant::manager_->cached_int.end())
        return Constant::manager_->cached_int[val].get();
    return (Constant::manager_->cached_int[val] =
                std::unique_ptr<ConstantInt>(new ConstantInt(Type::getInt32Type(), val)))
        .get();
}

ConstantInt *ConstantInt::get(bool val) {
    if (Constant::manager_->cached_bool.find(val) != Constant::manager_->cached_bool.end())
        return Constant::manager_->cached_bool[val].get();
    return (Constant::manager_->cached_bool[val] =
                std::unique_ptr<ConstantInt>(new ConstantInt(Type::getInt1Type(), val ? 1 : 0)))
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
ConstantFP *ConstantFP::get(float val) {
    if (Constant::manager_->cached_float.find(val) != Constant::manager_->cached_float.end())
        return Constant::manager_->cached_float[val].get();
    return (Constant::manager_->cached_float[val] =
                std::unique_ptr<ConstantFP>(new ConstantFP(Type::getFloatType(), val)))
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
ConstantZero *ConstantZero::get(Type *ty) {
    if (not Constant::manager_->cached_zero[ty])
        Constant::manager_->cached_zero[ty] = std::unique_ptr<ConstantZero>(new ConstantZero(ty));
    return Constant::manager_->cached_zero[ty].get();
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
