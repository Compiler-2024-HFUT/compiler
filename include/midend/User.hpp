#ifndef USER_HPP
#define USER_HPP

#include "Value.hpp"
#include <vector>

// #include <memory>
// class Value;
class User : public Value {
  public:
    User(Type *ty, const std::string &name = "", unsigned numUses = 0);


    //返回def-use链
    std::vector<Value *> &getDefUseList(){return defUseList;}

    //返回指定位置的定义量，位置从0开始
    Value *getOpe(unsigned i) const;

    //向def-use链中指定位置加入值，索引从0开始
    void setOpe(unsigned i, Value *v);

    //向def-use链中追加值
    void addOpe(Value *v);

    //删除def-use链中指定位置的值
    //？改进：这个的实现后面可否改进
    void removeOpe(unsigned i);

    void removeOpe(unsigned i, unsigned j);

    //返回定义值的数量
    unsigned getNumOpe() const;



    //于use-def链中删除def-use链中使用的操作数值
    void removeUDFromDU();
    
    //设置def-use链的大小
    void setSizeDU(unsigned num) {
        numUses = num;
        defUseList.resize(num, nullptr);
    }

    //删除def-use与use-def链
    void clearOps() {
        numUses = 0;
        removeUDFromDU();
        defUseList.clear();
    }

  private:
    std::vector<Value *> defUseList; // operands of this value，指向值的指针向量，即def-use链
    unsigned numUses;    //操作数数量
};

#endif