#ifndef VALUE_HPP
#define VALUE_HPP

#include <list>
#include <string>
#include <functional>

class Type;
class Value;
class User;

struct Use {

  Use(Value *val, unsigned no) : val_(val), arg_no_(no) {}

  friend bool operator==(const Use &lhs, const Use &rhs) {
      return lhs.val_ == rhs.val_ && lhs.arg_no_ == rhs.arg_no_;
  }

  Value *val_;
  unsigned arg_no_;   //& the no. of operand, e.g., func(a, b), a is 0, b is 1
};

class UseHash {
public:
  size_t operator()(const Use &u) const {
    return (std::hash<Value *>()(u.val_)) ^ (std::hash<unsigned>()(u.arg_no_));
  }
};


class Value {
public:
    explicit Value(Type *ty, const std::string &name=""): type_(ty), name_(name) {}
    virtual ~Value() = default;

    Type *getType() const { return type_; } 

    std::list<Use> &getUseList() { return use_list_; }
    int getUseNum() { return use_list_.size(); }
    void addUse(Value* val, unsigned arg_no = 0);

    bool setName(std::string name) {
      if(name_ == "") {
        name_ = name;
        return  true;
      } 
      return false;
    }

    void takeName(Value* v){
        name_ = v->getName();
        v->name_= "";
    }
    
    std::string getName() const { return name_; }

    void replaceAllUseWith(Value *new_val);
    void replaceUseWithWhen(Value *new_val, std::function<bool(User *)> pred); //& replace `value` with `new_val` when the user of value satisfies predicate `pred`
    void removeUse(Value *val);

    __attribute__((always_inline))  bool useOne(){return use_list_.size()==1;}
    __attribute__((always_inline))  bool useEmpty(){return use_list_.empty();}    
    std::string print(){return "";}

private:
    Type *type_;
    std::list<Use> use_list_;   //& store the users who use this value
    std::string name_;          
};

#endif