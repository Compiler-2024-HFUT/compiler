#include <cassert>

#include "midend/User.hpp"

User::User(Type *ty, const std::string &name, unsigned num_ops) 
  : Value(ty, name){

    operands_.resize(num_ops, nullptr);
}

Value *User::getOperand(unsigned i) const {
    return operands_[i];
}

void User::replaceOperand(unsigned i,Value*v) {
    operands_[i]->removeUse(this);
    assert(i < operands_.size() && "set_operand out of index");
    operands_[i] = v;
    v->addUse(this, i);
    // for (int i=0; i < this->getNumOperands(); i++){
    //     auto value = this->getOperand(i);
    //     value->removeUse(this);
    //     value->addUse(this,i);
    // }
}

void User::setOperand(unsigned i, Value *v) {
    assert(i < operands_.size() && "set_operand out of index");
    operands_[i] = v;
    v->addUse(this, i);
}

void User::addOperand(Value *v) {
    v->addUse(this, operands_.size());
    operands_.push_back(v);
    // num_ops_++;
}

std::vector<Value*>& User::getOperands() {
    return operands_;
}

unsigned User::getNumOperands() const noexcept {
    return operands_.size();
}
void User::removeAllOperand(){
    for(auto v:operands_){
        v->removeUse(this);
    }
    operands_.clear();
    // num_ops_=0;
}
void User::removeOperands(int index1, int index2) {
    for (int i = index1; i <= index2; i++) {
        operands_[i]->removeUse(this);
    }
    operands_.erase(operands_.begin() + index1, operands_.begin() + index2 + 1);
    // num_ops_ = operands_.size();
}

void User::removeUseOfOps() {
    for (auto op : operands_) {
        op->removeUse(this);
    }
}



