#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <list>
#include <functional>

class Type;
class User;

class Use{
  public:
    Value *val;      //值类
    unsigned argPos; //值作为函数参数时的索引，从0开始？
    Use(Value *val, unsigned pos) : val(val), argPos(pos) {}  
  
    //两个Use的值和位置都相同时返回true
    friend bool operator==(const Use &ls, const Use &rs) {
    return ls.val == rs.val && ls.argPos == rs.argPos;
  }
};


class Value {
  public:
    Value(Type *ty, const std::string &name = ""); 

    //返回Value的类型
    Type *getType() const { return type; }

    //返回use-def链
    std::list<Use> &getUseDefList() { return useDefList; }

    //向use-def链中添加新Value
    void addUseDefList(Value *val, unsigned argPos = 0);

    //命名Value
    bool setName(std::string name) {
        if (name == "") {
            name = name;
            return true;
        }
            return false;
    }

    //返回名字
    std::string getName() const{return name;}

    //打印的虚函数
  virtual std::string print() = 0;

    //用新值替换同位置的旧值（无条件替换）
    void replaceUse(Value *newVal);

    //条件替换
    void replaceUseCond(Value *newVal, std::function<bool(User *)> cond); 

    //删除use-def链中指定位置pos的值
    void removeUse(Value *val, unsigned argPos);

    // bool isValidVar() {(type->isIntegerTy() &&
    // dynamiccast<ConstantInt*>(this)) || type->isPointerTy();}

  private:
    Type *type;              //类型
    std::list<Use> useDefList; //记录使用该Value的对象，此即为use-def链
    std::string name;        //名字

};

//该类的功能是建立Use的哈希表，用于查找
class UseSearch {
  public:
    //该函数说明了一个Use的哈希值的建立方法：使用一个Use的值的哈希与位置的哈希进行异或运算生成该Use在哈希表中的值
    //？改进：我们后面可以看看LLVM是不是也是采用这种方法，我们是否可以修改哈希的生成方法
    size_t operator()(const Use &u) const {
        return (std::hash<Value *>()(u.val)) ^ (std::hash<unsigned>()(u.argPos));
    }
};

#endif