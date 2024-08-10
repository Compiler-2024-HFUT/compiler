#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <vector>

#include "User.hpp"
#include "Type.hpp"

class Module;
class BasicBlock;
class Function;
class IRVisitor;
class Instruction : public User {
public:
    enum class OpID {
      //& Terminator Instructions
      ret,
      br,
      //& Standard binary operators
      add,
      sub,
      mul,
      mul64,
      sdiv,
      srem,
      //& float binary operators
      fadd,
      fsub,
      fmul,
      fdiv,
      //& Memory operators
      alloca,
      load,
      store,
      memset,
      //& Other operators
      cmp,
      fcmp,
      phi,
      call,
      getelementptr,
      //& Logical operators
      land,
      lor,
      lxor,
      //& Shift operators
      asr,
      shl,
      lsr,
      asr64,
      shl64,
      lsr64,
      //& Zero extend
      zext,
      //& type cast bewteen float and singed integer
      fptosi,
      sitofp,
      //& LIR operators
      cmpbr,
      fcmpbr,
      loadoffset,
      storeoffset,
      select,
      loadimm,
    };

public:
    OpID getInstrType() { return op_id_; }

    static std::string getInstrOpName(OpID id) {
      switch(id) {
        case OpID::ret: return "ret"; break;
        case OpID::br: return "br"; break;

        case OpID::add: return "add"; break;
        case OpID::sub: return "sub"; break;
        case OpID::mul: return "mul"; break;
        case OpID::mul64: return "mulh64"; break;
        case OpID::sdiv: return "sdiv"; break;
        case OpID::srem: return "srem"; break;

        case OpID::fadd: return "fadd"; break;
        case OpID::fsub: return "fsub"; break;
        case OpID::fmul: return "fmul"; break;
        case OpID::fdiv: return "fdiv"; break;

        case OpID::alloca: return "alloca"; break;
        case OpID::load: return "load"; break;
        case OpID::store: return "store"; break;
        case OpID::memset: return "memset"; break;

        case OpID::cmp: return "cmp"; break;
        case OpID::fcmp: return "fcmp"; break;
        case OpID::phi: return "phi"; break;
        case OpID::call: return "call"; break;
        case OpID::getelementptr: return "getelementptr"; break;

        case OpID::land: return "and"; break;
        case OpID::lor: return "or"; break;
        case OpID::lxor: return "xor"; break;

        case OpID::asr: return "ashr"; break;
        case OpID::shl: return "shl"; break;
        case OpID::lsr: return "lshr"; break; 
        case OpID::asr64: return "asr64"; break;
        case OpID::shl64: return "shl64"; break;
        case OpID::lsr64: return "lsr64"; break; 

        case OpID::zext: return "zext"; break;

        case OpID::fptosi: return "fptosi"; break; 
        case OpID::sitofp: return "sitofp"; break; 

        case OpID::cmpbr: return "cmpbr"; break;
        case OpID::fcmpbr: return "fcmpbr"; break;
        case OpID::loadoffset: return "loadoffset"; break;
        case OpID::storeoffset: return "storeoffset"; break;
        case OpID::select:return "select"; break;
        case OpID::loadimm:return "loadimm"; break;
        default: return ""; break; 
      }
    }

public:
    //& create instruction, auto insert to bb, ty here is result type
    Instruction(Type *ty, OpID id, unsigned num_ops, BasicBlock *parent);
    Instruction(Type *ty, OpID id, unsigned num_ops);

    inline const BasicBlock *getParent() const { return parent_; }
    inline BasicBlock *getParent() { return parent_; }
    void setParent(BasicBlock *parent) { this->parent_ = parent; }

    //// Return the function this instruction belongs to.
    Function *getFunction();
    
    bool isVoid() {
        return ((op_id_ == OpID::ret) || (op_id_ == OpID::br) || (op_id_ == OpID::store) || (op_id_ == OpID::cmpbr) || (op_id_ == OpID::fcmpbr) || (op_id_ == OpID::storeoffset) || (op_id_ == OpID::memset) ||
                (op_id_ == OpID::call && this->getType()->isVoidType()));
    }

    bool isRet() { return op_id_ == OpID::ret; } 
    bool isBr() { return op_id_ ==  OpID::br; } 

    bool isAdd() { return op_id_ ==  OpID::add; } 
    bool isSub() { return op_id_ ==  OpID::sub; }
    bool isMul() { return op_id_ ==  OpID::mul; } 
    bool isMul64() { return op_id_ == OpID::mul64; }
    bool isDiv() { return op_id_ ==  OpID::sdiv; }
    bool isRem() { return op_id_ ==  OpID::srem; } 

    bool isFAdd() { return op_id_ ==  OpID::fadd; } 
    bool isFSub() { return op_id_ ==  OpID::fsub; } 
    bool isFMul() { return op_id_ ==  OpID::fmul; } 
    bool isFDiv() { return op_id_ ==  OpID::fdiv; } 

    bool isAlloca() { return op_id_ ==  OpID::alloca; } 
    bool isLoad() { return op_id_ ==  OpID::load; } 
    bool isStore() { return op_id_ ==  OpID::store; } 
    bool isMemset() { return op_id_ == OpID::memset; }

    bool isCmp() { return op_id_ ==  OpID::cmp; }
    bool isFCmp() { return op_id_ ==  OpID::fcmp; } 
    bool isPhi() { return op_id_ ==  OpID::phi; } 
    bool isCall() { return op_id_ ==  OpID::call; }
    bool isGep(){ return op_id_ ==  OpID::getelementptr; } 

    bool isAnd() { return op_id_ ==  OpID::land; } 
    bool isOr() { return op_id_ ==  OpID::lor; }
    bool isXor() { return op_id_ == OpID::lxor; } 

    bool isAsr() { return op_id_ ==  OpID::asr; }
    bool isLsl() { return op_id_ ==  OpID::shl; } 
    bool isLsr() { return op_id_ ==  OpID::lsr; } 
    bool isAsr64() { return op_id_ ==  OpID::asr64; }
    bool isLsl64() { return op_id_ ==  OpID::shl64; } 
    bool isLsr64() { return op_id_ ==  OpID::lsr64; }

    bool isZext() { return op_id_ ==  OpID::zext; } 
    
    bool isFptosi(){ return op_id_ ==  OpID::fptosi; } 
    bool isSitofp(){ return op_id_ ==  OpID::sitofp; } 

    bool isCmpBr() { return op_id_ == OpID::cmpbr; }
    bool isFCmpBr() { return op_id_ == OpID::fcmpbr; }
    bool isLoadOffset() { return op_id_ == OpID::loadoffset; }
    bool isStoreOffset() { return op_id_ == OpID::storeoffset; }

    bool isExtendBr() { return (op_id_ ==  OpID::br || op_id_ == OpID::cmpbr || op_id_ == OpID::fcmpbr); }
    bool isExtendCondBr() const { return getNumOperands() == 3 || getNumOperands() == 4; }


    bool isIntBinary() {
        return (isAdd() || isSub() || isMul() || isDiv() || isRem() || isMul64() ||
                isAnd() || isOr() || isXor() || 
                isAsr() || isLsl() || isLsr() || isAsr64() || isLsl64() || isLsr64()) &&
               (getNumOperands() == 2);
    }

    bool isFloatBinary() {
        return (isFAdd() || isFSub() || isFMul() || isFDiv()) && (getNumOperands() == 2);
    }

    bool isBinary() {
        return isIntBinary() || isFloatBinary();
    }

    bool isTerminator() { return isBr() || isRet() || isCmpBr() || isFCmpBr(); }
    bool isWriteMem(){return isStore() || isStoreOffset(); }

    bool isSelect(){return op_id_==OpID::select;}
    bool isLoadImm(){return op_id_==OpID::loadimm;}

    // void setId(int id) { id_ = id; }
    // int getId() { return id_; }
     void setId(int id) { id_ = id; }
     int getId() { return id_; }

    virtual Instruction *copyInst(BasicBlock *bb) = 0;

    //后端遍历
    virtual void accept(IRVisitor &visitor) = 0;

private:
    OpID op_id_;
    // unsigned num_ops_;
    BasicBlock* parent_;
    //似乎没有用到
     int id_;
};

class BinaryInst : public Instruction {
public:

    static BinaryInst *create(OpID id,Value *v1, Value *v2);

    static BinaryInst *createAdd(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createSub(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createMul(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createMul64(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createSDiv(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createSRem(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createFAdd(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createFSub(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createFMul(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createFDiv(Value *v1, Value *v2, BasicBlock *bb);

    static BinaryInst *createAnd(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createOr(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createXor(Value *v1, Value *v2, BasicBlock *bb);

    static BinaryInst *createAsr(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createLsl(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createLsr(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createAsr64(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createLsl64(Value *v1, Value *v2, BasicBlock *bb);
    static BinaryInst *createLsr64(Value *v1, Value *v2, BasicBlock *bb);

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final {
        return new BinaryInst(getType(), getInstrType(), getOperand(0), getOperand(1), bb);
    }

    bool isNeg();

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    BinaryInst(Type *ty, OpID id, Value *v1, Value *v2, BasicBlock *bb); 
    BinaryInst(Type *ty, OpID id, Value *v1, Value *v2); 

    //~ void assert_valid();
};


enum CmpOp {
    EQ, // ==
    NE, // !=
    GT, // >
    GE, // >=
    LT, // <
    LE  // <=
};
class CmpInst : public Instruction {
public:
    static CmpInst *createCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);

    CmpOp getCmpOp() { return cmp_op_; }

    void negation();

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final {
        return new CmpInst(getType(), cmp_op_, getOperand(0), getOperand(1), bb);
    }
    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    CmpInst(Type *ty, CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb); 
    //~ void assert_valid();
    
private:
    CmpOp cmp_op_;
    
};

class FCmpInst : public Instruction {
  public:
    static FCmpInst *createFCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);

    CmpOp getCmpOp() { return cmp_op_; }

    void negation();

    virtual std::string print() override;
    Instruction *copyInst(BasicBlock *bb) override final {
        return new FCmpInst(getType(), cmp_op_, getOperand(0), getOperand(1), bb);
    }
    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

  private:
    FCmpInst(Type *ty, CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);

    //~ void assert_valid();

  private:
    CmpOp cmp_op_;
};

class CallInst : public Instruction {
public:
    static CallInst *createCall(Function *func, std::vector<Value *>args, BasicBlock *bb);
    
    FunctionType *getFunctionType() const { return static_cast<FunctionType *>(getOperand(0)->getType()); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        std::vector<Value *> args;
        for (auto i = 1; i < getNumOperands(); i++){
            args.push_back(getOperand(i));
        }
        auto new_inst = new CallInst(getFunctionType()->getReturnType(),args,bb);
        new_inst->setOperand(0, getOperand(0));
        return new_inst;
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

protected:
    CallInst(Function *func, std::vector<Value *>args, BasicBlock *bb);
    CallInst(Type *ret_ty, std::vector<Value *> args, BasicBlock *bb);
};

class BranchInst : public Instruction {
public:
    static BranchInst *createCondBr(Value *cond, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb);
    static BranchInst *createBr(BasicBlock *if_true, BasicBlock *bb);

   
    bool isCondBr() const { return getNumOperands() == 3; }
    // bool is_extend_cond_br() const { return get_num_operands() == 3 || get_num_operands() == 4; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        if (getNumOperands() == 1){
            auto new_inst = new BranchInst(bb);
            new_inst->setOperand(0, getOperand(0));
            return new_inst;
        } else {
            auto new_inst = new BranchInst(getOperand(0),bb);
            new_inst->setOperand(1, getOperand(1));
            new_inst->setOperand(2, getOperand(2));
            return new_inst;
        }
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    BranchInst(Value *cond, BasicBlock *if_true, BasicBlock *if_false,
               BasicBlock *bb);
    BranchInst(BasicBlock *if_true, BasicBlock *bb);
    BranchInst(BasicBlock *bb);
    BranchInst(Value *cond, BasicBlock *bb);
};

class ReturnInst : public Instruction {
public:
    static ReturnInst *createRet(Value *val, BasicBlock *bb);
    static ReturnInst *createVoidRet(BasicBlock *bb);

    bool isVoidRet() const { return getNumOperands() == 0; }

    Type * getRetType() const { return getOperand(0)->getType(); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        if (isVoidRet()){
            return new ReturnInst(bb);
        } else {
            return new ReturnInst(getOperand(0),bb);
        }
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    ReturnInst(Value *val, BasicBlock *bb);
    ReturnInst(BasicBlock *bb);
};

class GetElementPtrInst : public Instruction {
public:
    static Type *getElementType(Value *ptr, std::vector<Value *> idxs);
    static GetElementPtrInst *createGep(Value *ptr, std::vector<Value *> idxs, BasicBlock *bb);
    Type *getElementType() const { return element_ty_; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        std::vector<Value *> idxs;
        for (auto i = 1; i < getNumOperands(); i++) {
            idxs.push_back(getOperand(i));
        }
        return new GetElementPtrInst(getOperand(0),idxs,bb);
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    GetElementPtrInst(Value *ptr, std::vector<Value *> idxs, BasicBlock *bb);

private:
    Type *element_ty_;
};


class StoreInst : public Instruction {
public:
    static StoreInst *createStore(Value *val, Value *ptr, BasicBlock *bb);

    Value *getRVal() { return this->getOperand(0); }
    Value *getLVal() { return this->getOperand(1); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new StoreInst(getOperand(0),getOperand(1),bb);
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    StoreInst(Value *val, Value *ptr, BasicBlock *bb);
};

//& 加速使用全0初始化数组的代码优化分析和代码生成
class MemsetInst : public Instruction {
public:
    static MemsetInst *createMemset(Value *ptr, BasicBlock *bb);

    Value *getLVal() { return this->getOperand(0); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new MemsetInst(getOperand(0),bb);
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    MemsetInst(Value *ptr, BasicBlock *bb);
};

class LoadInst : public Instruction {
public:
    static LoadInst *createLoad(Type *ty, Value *ptr, BasicBlock *bb);
    
    Value * getLVal() { return this->getOperand(0); }

    Type *getLoadType() const { return static_cast<PointerType *>(getOperand(0)->getType())->getElementType(); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new LoadInst(getType(),getOperand(0),bb);
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    LoadInst(Type *ty, Value *ptr, BasicBlock *bb);
};


class AllocaInst : public Instruction {
public:
    static AllocaInst *createAlloca(Type *ty, BasicBlock *bb);

    Type *getAllocaType() const { return alloca_ty_; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new AllocaInst(alloca_ty_,bb);
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    AllocaInst(Type *ty, BasicBlock *bb);

private:
    Type *alloca_ty_;
};

class ZextInst : public Instruction {
public:
    static ZextInst *createZext(Value *val, Type *ty, BasicBlock *bb);

    Type *getDestType() const { return getType(); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new ZextInst( getOperand(0),getDestType(),bb);
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    ZextInst( Value *val, Type *ty, BasicBlock *bb);

private:
};

class SiToFpInst : public Instruction {
public:
    static SiToFpInst *createSiToFp(Value *val, Type *ty, BasicBlock *bb);

    Type *getDestType() const { return getType(); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new SiToFpInst( getOperand(0), getDestType(), bb);
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    SiToFpInst( Value *val, Type *ty, BasicBlock *bb);

};

class FpToSiInst : public Instruction {
public:
    static FpToSiInst *createFpToSi(Value *val, Type *ty, BasicBlock *bb);

    Type *getDestType() const { return getType(); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new FpToSiInst( getOperand(0), getDestType(), bb);
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    FpToSiInst( Value *val, Type *ty, BasicBlock *bb);

};

class PhiInst : public Instruction {
public:
    static PhiInst *createPhi(Type *ty, BasicBlock *bb);

    // Value *getLVal() { return l_val_; }
    // void setLVal(Value *l_val) { l_val_ = l_val; }

    void addPhiPairOperand(Value *val, Value *pre_bb) {
        this->addOperand(val);
        this->addOperand(pre_bb);
    }

    void removePhiPairOperand(Value *pre_bb) {
        for(int i = 1; i < getNumOperands(); i += 2) {
            if(getOperand(i) == pre_bb) {
                removeOperands(i-1, i);
                break;
            }
        }

        std::vector<Value*> &ops = this->getOperands();
        this->removeUseOfOps();
        for(int i = 0; i < ops.size(); i++) {
            this->getOperand(i)->addUse(this, i);
        }
    }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = createPhi(getType(), bb);
        for (auto op : getOperands()){
            new_inst->addOperand(op);
        }
        return new_inst;
    }
    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
     PhiInst(OpID op, std::vector<Value *> vals, std::vector<BasicBlock *> val_bbs, Type *ty, BasicBlock *bb);

// private:
    // Value *l_val_;
};

class CmpBrInst: public Instruction {

public:
    static CmpBrInst *createCmpBr(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb);

    CmpOp getCmpOp() { return cmp_op_; }

    bool isCmpBr() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = new CmpBrInst(cmp_op_,getOperand(0),getOperand(1),bb);
        new_inst->setOperand(2, getOperand(2));
        new_inst->setOperand(3, getOperand(3));
        return new_inst;
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    CmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb);
    CmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);
private:
    CmpOp cmp_op_;

};


class FCmpBrInst : public Instruction {
public:
    static FCmpBrInst *createFCmpBr(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb);

    CmpOp getCmpOp() { return cmp_op_; }

    bool isFCmpBr() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = new FCmpBrInst(cmp_op_,getOperand(0),getOperand(1),bb);
        new_inst->setOperand(2, getOperand(2));
        new_inst->setOperand(3, getOperand(3));
        return new_inst;
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;


private:
    FCmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb);
    FCmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);
private:
    CmpOp cmp_op_;
};

class LoadOffsetInst: public Instruction {
public:
    static LoadOffsetInst *createLoadOffset(Type *ty, Value *ptr, Value *offset, BasicBlock *bb);

    Value *getLVal() { return this->getOperand(0); }
    Value *getOffset() { return this->getOperand(1); }

    Type *getLoadType() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = new LoadOffsetInst(getType(), getOperand(0), bb);
        new_inst->setOperand(1, getOperand(1));
        return new_inst;
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    LoadOffsetInst(Type *ty, Value *ptr, Value *offset, BasicBlock *bb);
    LoadOffsetInst(Type *ty, Value *ptr, BasicBlock *bb);
};


class StoreOffsetInst: public Instruction {

public:
    static StoreOffsetInst *createStoreOffset(Value *val, Value *ptr, Value *offset, BasicBlock *bb);

    Type *getStoreType() const;

    Value *getRVal() { return this->getOperand(0); }
    Value *getLVal() { return this->getOperand(1); }
    Value *getOffset() { return this->getOperand(2); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = new StoreOffsetInst(getOperand(0), getOperand(1), bb);
        new_inst->setOperand(2, getOperand(2));
        return new_inst;
    }

    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    StoreOffsetInst(Value *val, Value *ptr, Value *offset, BasicBlock *bb);
    StoreOffsetInst(Value *val, Value *ptr, BasicBlock *bb);
};

class SelectInst: public Instruction {

public:
    static SelectInst *createSelect(Type*type,Value *cond, Value *true_val, Value *false_val, BasicBlock *bb);

    // Type *getSelectType() const{return getType();}

    __attribute__((always_inline)) Value *getCond() const { return getOperand(0); }
    __attribute__((always_inline)) Value *getTrue() const { return getOperand(1); }
    __attribute__((always_inline)) Value *getFalse() const { return getOperand(2); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return  new SelectInst(this->getType(),getOperand(0),getOperand(1), getOperand(2), bb);
    }

    virtual void accept(IRVisitor &visitor) final{}

private:
    SelectInst(Type*type,Value *cond, Value *true_val, Value *false_val, BasicBlock *bb);
};

class LoadImmInst: public Instruction {

public:
    static LoadImmInst *createLoadImm(Type*type,Value *cons, BasicBlock *bb);

    // Type *getSelectType() const{return getType();}

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return  new LoadImmInst(this->getType(),getOperand(0),bb);
    }

    virtual void accept(IRVisitor &visitor) final;

private:
    LoadImmInst(Type*type,Value*cons,  BasicBlock *bb);
};

#endif





