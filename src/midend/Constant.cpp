#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>

#include "midend/Constant.hpp"
#include "midend/Instruction.hpp"
#include "midend/Type.hpp"
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

Constant* Constant::get(Constant *lhs,Instruction::OpID bin_op,Constant*rhs){
    Constant*ret=0;
    if(ConstantInt* ilhs=dynamic_cast<ConstantInt*>(lhs),*irhs=dynamic_cast<ConstantInt*>(rhs);ilhs&&irhs){
        ret=ConstantInt::getFromBin(ilhs,bin_op,irhs);
    }else if(ConstantFP* flhs=dynamic_cast<ConstantFP*>(lhs),*frhs=dynamic_cast<ConstantFP*>(rhs);flhs&&frhs){
        ret=ConstantFP::getFromBin(flhs,bin_op,frhs);
    }
    return ret;
}

ConstantInt *ConstantInt::getFromBin(ConstantInt *lhs,Instruction::OpID bin_op,ConstantInt*rhs){
    ConstantInt* ret;
    switch (bin_op) {
    case Instruction::OpID::add:
        ret=get(lhs->getValue()+rhs->getValue());
        break;
    case Instruction::OpID::sub:
        ret=get(lhs->getValue()-rhs->getValue());
        break;
    case Instruction::OpID::mul:
        ret=get(lhs->getValue()*rhs->getValue());
        break;
    case Instruction::OpID::sdiv:
        ret=get(lhs->getValue()/rhs->getValue());
        break;
    case Instruction::OpID::srem:
        ret=get(lhs->getValue()%rhs->getValue());
        break;
    case Instruction::OpID::lor:
        ret=get(lhs->getValue()|rhs->getValue());
        break;
    case Instruction::OpID::lxor:
        ret=get(lhs->getValue()^rhs->getValue());
        break;
    case Instruction::OpID::land:
        ret=get(lhs->getValue()&rhs->getValue());
        break;
    case Instruction::OpID::asr:
        ret=get(lhs->getValue()>>rhs->getValue());
        break;
    case Instruction::OpID::shl:
        ret=get(lhs->getValue()<<rhs->getValue());
        break;
    case Instruction::OpID::lsr:
        ret=get((int32_t)(((uint32_t)lhs->getValue())>>rhs->getValue()));
        break;
    default:
        ret=0;
        break;
    }
    return ret;
}
/*enum CmpOp {
    EQ, // ==
    NE, // !=
    GT, // >
    GE, // >=
    LT, // <
    LE  // <=
};
*/
ConstantInt *ConstantInt::getFromCmp(Constant *lhs,CmpOp cmp_op,Constant*rhs){
    ConstantInt*ret;
    if(ConstantInt* ilhs=dynamic_cast<ConstantInt*>(lhs),*irhs=dynamic_cast<ConstantInt*>(rhs);ilhs&&irhs){
        ret=ConstantInt::getFromCmp(ilhs,cmp_op,irhs);
    }else if(ConstantFP* flhs=dynamic_cast<ConstantFP*>(lhs),*frhs=dynamic_cast<ConstantFP*>(rhs);flhs&&frhs){
        ret=ConstantInt::getFromFCmp(flhs,cmp_op,frhs);
    }else 
        ret=0;
    assert(ret!=0);
    return ret;
}

ConstantInt *ConstantInt::getFromICmp(ConstantInt *lhs,CmpOp cmp_op,ConstantInt*rhs){
    ConstantInt* ret;
    switch (cmp_op) {
    case CmpOp::EQ:
        ret=get(lhs->getValue()==rhs->getValue());
        break;
    case CmpOp::NE:
        ret=get(lhs->getValue()!=rhs->getValue());
        break;
    case CmpOp::GT:
        ret=get(lhs->getValue()>rhs->getValue());
        break;
    case CmpOp::GE:
        ret=get(lhs->getValue()>=rhs->getValue());
        break;
    case CmpOp::LT:
        ret=get(lhs->getValue()<rhs->getValue());
        break;
    case CmpOp::LE:
        ret=get(lhs->getValue()<=rhs->getValue());
        break;
    default:
        ret=0;
        break;
    }
    return ret;
}
ConstantInt *ConstantInt::getFromFCmp(ConstantFP *lhs,CmpOp cmp_op,ConstantFP*rhs){
    ConstantInt* ret;
    switch (cmp_op) {
    case CmpOp::EQ:
        ret=get(lhs->getValue()==rhs->getValue());
        break;
    case CmpOp::NE:
        ret=get(lhs->getValue()!=rhs->getValue());
        break;
    case CmpOp::GT:
        ret=get(lhs->getValue()>rhs->getValue());
        break;
    case CmpOp::GE:
        ret=get(lhs->getValue()>=rhs->getValue());
        break;
    case CmpOp::LT:
        ret=get(lhs->getValue()<rhs->getValue());
        break;
    case CmpOp::LE:
        ret=get(lhs->getValue()<=rhs->getValue());
        break;
    default:
        ret=0;
        break;
    }
    return ret;
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
ConstantFP *ConstantFP::getFromBin(ConstantFP *lhs,Instruction::OpID bin_op,ConstantFP*rhs){
    ConstantFP* ret;
    switch (bin_op) {
    case Instruction::OpID::fadd:
        ret=get(lhs->getValue()+rhs->getValue());
        break;
    case Instruction::OpID::fsub:
        ret=get(lhs->getValue()-rhs->getValue());
        break;
    case Instruction::OpID::fmul:
        ret=get(lhs->getValue()*rhs->getValue());
        break;
    case Instruction::OpID::fdiv:
        ret=get(lhs->getValue()/rhs->getValue());
        break;
    default:
        ret=0;
    }
    return ret;
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
