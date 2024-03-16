#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <cassert>
#include <iterator>
#include <list>
#include <vector>

#include "User.hpp"
#include "BasicBlock.hpp"
#include "Type.hpp"

class Module;
class Argument;
class BasicBlock;
class Type;
class FunctionType;

class Function : public Value {
public:
    Function(FunctionType *ty, const std::string &name, Module *parent);
    ~Function();

    static Function *create(FunctionType *ty, const std::string &name, Module *parent);

    FunctionType *getFunctionType() const { return static_cast<FunctionType *>(getType()); }
    Type *getReturnType() const { return getFunctionType()->getReturnType(); }

    Module *getParent() const { return parent_; }

    unsigned getNumOfArgs() const { return getFunctionType()->getNumOfArgs(); }
    std::list<Argument *>::iterator argBegin() { return arguments_.begin(); }
    std::list<Argument *>::iterator argEnd() { return arguments_.end(); }
    std::list<Argument *> &getArgs() { return arguments_; }

    std::vector<Argument*> &getIArgs() { return i_args_; }
    std::vector<Argument*> &getFArgs() { return f_args_; }

    void addBasicBlock(BasicBlock *bb);
    unsigned getNumBasicBlocks() const { return basic_blocks_.size(); }
    BasicBlock *getEntryBlock() const { return *basic_blocks_.begin(); }
    std::list<BasicBlock *>&getBasicBlocks() { return basic_blocks_; }
    void removeBasicBlock(BasicBlock *bb);

    bool isDeclaration() { return basic_blocks_.empty(); }

    void setInstrName();

    std::string print();
  
private:
    void buildArgs();

private:
    std::list<BasicBlock *> basic_blocks_;  //& basic blocks
    std::list<Argument *> arguments_;       //& arguments  
    std::vector<Argument*> i_args_;
    std::vector<Argument*> f_args_;
    Module *parent_;  
    unsigned seq_cnt_;
};

//// Argument of Function, does not contain actual value
class Argument : public Value {
public:
    //// Argument constructor.
    explicit Argument(Type *ty, const std::string &name="", Function *f=nullptr, 
                    unsigned arg_no = 0)
        : Value(ty, name), parent_(f), arg_no_(arg_no) {}
    virtual ~Argument() {}

    inline const Function *getParent() const { return parent_; }
    inline Function *getParent() { return parent_; }

    //// For example in "void foo(int a, float b)" a is 0 and b is 1.
    unsigned getArgNo() const { 
      assert(parent_ && "can't get number of unparented arg");
      return arg_no_;
    }

    virtual std::string print() override;

private:
    Function *parent_;
    unsigned arg_no_; //& argument No. 
};

#endif