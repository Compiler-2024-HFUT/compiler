#include <cassert>

#include "midend/Value.hpp"
#include "midend/User.hpp"

void Value::addUse(Value *val, unsigned arg_no) {
    use_list_.push_back(Use(val, arg_no));
}

void Value::replaceAllUseWith(Value *new_val) {
    for (auto use : use_list_) {
        auto val = dynamic_cast<User *>(use.val_);
        assert(val && "exist a value(but not user) use old value");
        val->setOperand(use.arg_no_, new_val);
    }
    use_list_.clear();
}

void Value::replaceUseWithWhen(Value *new_val, std::function<bool(User *)> pred) {
    for (auto it = use_list_.begin(); it != use_list_.end();) {
        auto use = *it;
        auto val = dynamic_cast<User *>(use.val_);
        assert(val && "exist a value(but not user) use old value");
        if (not pred(val)) {
            ++it;
            continue;
        }
        val->setOperand(use.arg_no_, new_val);
        it = use_list_.erase(it);
    }
}

void Value::removeUse(Value *val) {
    auto is_val = [val](const Use &use) { return use.val_ == val; };
    use_list_.remove_if(is_val);
}
