#include <cassert>

#include "../../include/midend/User.hpp"

User::User(Type *ty, const std::string &name, unsigned num_ops) 
  : Value(ty, name), num_ops_(num_ops) {

    operands_.resize(num_ops_, nullptr);
}

Value *User::getOperand(unsigned i) const {
    return operands_[i];
}

void User::setOperand(unsigned i, Value *v) {
    assert(i < num_ops_ && "set_operand out of index");
    operands_[i] = v;
    v->addUse(this, i);
}

void User::addOperand(Value *v) {
    operands_.push_back(v);
    v->addUse(this, num_ops_);
    num_ops_++;
}

std::vector<Value*>& User::getOperands() {
    return operands_;
}

unsigned User::getNumOperands() const {
    return num_ops_;
}

void User::removeOperands(int index1, int index2) {
    for (int i = index1; i <= index2; i++) {
        operands_[i]->removeUse(this);
    }
    operands_.erase(operands_.begin() + index1, operands_.begin() + index2 + 1);
    num_ops_ = operands_.size();
}

void User::removeUseOfOps() {
    for (auto op : operands_) {
        op->removeUse(this);
    }
}



