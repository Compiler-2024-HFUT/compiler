#include "midend/Constant.hpp"
#include "midend/IRBuilder.hpp"
#include "midend/Type.hpp"

IRBuilder::IRBuilder():manager_(){
    Type::builder=this;
    Constant::manager_=&manager_;
    void_ty_ = new Type(Type::VoidTyID);
    label_ty_ = new Type(Type::LabelTyID);
    int1_ty_ = new IntegerType(1);
    int32_ty_ = new IntegerType(32);
    float32_ty_ = new FloatType();
}
Type *IRBuilder::getVoidType() {
    return void_ty_;
}

Type *IRBuilder::getLabelType() {
    return label_ty_;
}

IntegerType *IRBuilder::getInt1Type() {
    return int1_ty_;
}

IntegerType *IRBuilder::getInt32Type() {
    return int32_ty_;
}

FloatType *IRBuilder::getFloatType() {
    return float32_ty_;
}

PointerType *IRBuilder::getInt32PtrType() {
    return getPointerType(int32_ty_);
}

PointerType *IRBuilder::getFloatPtrType() {
    return getPointerType(float32_ty_);
}

PointerType *IRBuilder::getPointerType(Type *contained) {
    if(pointer_map_.find(contained) == pointer_map_.end()) {
        pointer_map_[contained] = new PointerType(contained);
    }
    return pointer_map_[contained];
}

ArrayType *IRBuilder::getArrayType(Type *contained, unsigned num_elements) {
    if(array_map_.find({contained, num_elements}) == array_map_.end()) {
        array_map_[{contained, num_elements}] = new ArrayType(contained, num_elements);
    }
    return array_map_[{contained, num_elements}];
}

FunctionType *IRBuilder::getFunctionType(Type *retty, std::vector<Type *> &args) {
    if (not function_map_.count({retty, args})) {
        function_map_[{retty, args}] = new FunctionType(retty, args);
    }
    return function_map_[{retty, args}];
}

IRBuilder::~IRBuilder() {
    delete void_ty_;
    delete label_ty_;
    delete int1_ty_;
    delete int32_ty_;
    delete float32_ty_;
    for(auto &[t ,p]:pointer_map_){
        delete p;
    }
    for(auto &[t ,a]:array_map_){
        delete a;
    }
    for(auto &[t ,f]:function_map_){
        delete f;
    }
}
