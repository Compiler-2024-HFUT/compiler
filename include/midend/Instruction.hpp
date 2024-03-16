#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <vector>
#include <memory>

#include "User.hpp"
#include "Type.hpp"
#include "Constant.hpp"
#include "BasicBlock.hpp"

extern std::unique_ptr<Module> module_sole;

class BasicBlock;
class Function;

class Instruction : public User {
public:
    enum OpID {
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
      storeoffset
    };

public:
    OpID getInstrType() { return op_id_; }

    static std::string getInstrOpName(OpID id) {
      switch(id) {
        case ret: return "ret"; break;
        case br: return "br"; break;

        case add: return "add"; break;
        case sub: return "sub"; break;
        case mul: return "mul"; break;
        case mul64: return "mulh64"; break;
        case sdiv: return "sdiv"; break;
        case srem: return "srem"; break;

        case fadd: return "fadd"; break;
        case fsub: return "fsub"; break;
        case fmul: return "fmul"; break;
        case fdiv: return "fdiv"; break;

        case alloca: return "alloca"; break;
        case load: return "load"; break;
        case store: return "store"; break;
        case memset: return "memset"; break;

        case cmp: return "cmp"; break;
        case fcmp: return "fcmp"; break;
        case phi: return "phi"; break;
        case call: return "call"; break;
        case getelementptr: return "getelementptr"; break;

        case land: return "and"; break;
        case lor: return "or"; break;
        case lxor: return "xor"; break;

        case asr: return "ashr"; break;
        case shl: return "shl"; break;
        case lsr: return "lshr"; break; 
        case asr64: return "asr64"; break;
        case shl64: return "shl64"; break;
        case lsr64: return "lsr64"; break; 

        case zext: return "zext"; break;

        case fptosi: return "fptosi"; break; 
        case sitofp: return "sitofp"; break; 

        case cmpbr: return "cmpbr"; break;
        case fcmpbr: return "fcmpbr"; break;
        case loadoffset: return "loadoffset"; break;
        case storeoffset: return "storeoffset"; break;

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
    
    Module *getModule();

    bool isVoid() {
        return ((op_id_ == ret) || (op_id_ == br) || (op_id_ == store) || (op_id_ == cmpbr) || (op_id_ == fcmpbr) || (op_id_ == storeoffset) || (op_id_ == memset) ||
                (op_id_ == call && this->getType()->isVoidType()));
    }

    bool isRet() { return op_id_ == ret; } 
    bool isBr() { return op_id_ ==  br; } 

    bool isAdd() { return op_id_ ==  add; } 
    bool isSub() { return op_id_ ==  sub; }
    bool isMul() { return op_id_ ==  mul; } 
    bool isMul64() { return op_id_ == mul64; }
    bool isDiv() { return op_id_ ==  sdiv; }
    bool isRem() { return op_id_ ==  srem; } 

    bool isFAdd() { return op_id_ ==  fadd; } 
    bool isFSub() { return op_id_ ==  fsub; } 
    bool isFMul() { return op_id_ ==  fmul; } 
    bool isFDiv() { return op_id_ ==  fdiv; } 

    bool isAlloca() { return op_id_ ==  alloca; } 
    bool isLoad() { return op_id_ ==  load; } 
    bool isStore() { return op_id_ ==  store; } 
    bool isMemset() { return op_id_ == memset; }

    bool isCmp() { return op_id_ ==  cmp; }
    bool isFCmp() { return op_id_ ==  fcmp; } 
    bool isPhi() { return op_id_ ==  phi; } 
    bool isCall() { return op_id_ ==  call; }
    bool isGep(){ return op_id_ ==  getelementptr; } 

    bool isAnd() { return op_id_ ==  land; } 
    bool isOr() { return op_id_ ==  lor; }
    bool isXor() { return op_id_ == lxor; } 

    bool isAsr() { return op_id_ ==  asr; }
    bool isLsl() { return op_id_ ==  shl; } 
    bool isLsr() { return op_id_ ==  lsr; } 
    bool isAsr64() { return op_id_ ==  asr64; }
    bool isLsl64() { return op_id_ ==  shl64; } 
    bool isLsr64() { return op_id_ ==  lsr64; }

    bool isZext() { return op_id_ ==  zext; } 
    
    bool isFptosi(){ return op_id_ ==  fptosi; } 
    bool isSitofp(){ return op_id_ ==  sitofp; } 

    bool isCmpBr() { return op_id_ == cmpbr; }
    bool isFCmpBr() { return op_id_ == fcmpbr; }
    bool isLoadOffset() { return op_id_ == loadoffset; }
    bool isStoreOffset() { return op_id_ == storeoffset; }

    bool isExtendBr() { return (op_id_ ==  br || op_id_ == cmpbr || op_id_ == fcmpbr); }
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

    void setId(int id) { id_ = id; }
    int getId() { return id_; }

    virtual Instruction *copyInst(BasicBlock *bb) = 0;

private:
    OpID op_id_;
    unsigned num_ops_;
    BasicBlock* parent_;
    int id_;
};

class BinaryInst : public Instruction {
public:
    static BinaryInst *createAdd(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createSub(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createMul(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createMul64(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createSDiv(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createSRem(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createFAdd(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createFSub(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createFMul(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createFDiv(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());

    static BinaryInst *createAnd(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createOr(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createXor(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());

    static BinaryInst *createAsr(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createLsl(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createLsr(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createAsr64(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createLsl64(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());
    static BinaryInst *createLsr64(Value *v1, Value *v2, BasicBlock *bb, Module *m = module_sole.get());

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final {
        return new BinaryInst(getType(), getInstrType(), getOperand(0), getOperand(1), bb);
    }

private:
    BinaryInst(Type *ty, OpID id, Value *v1, Value *v2, BasicBlock *bb); 

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
    static CmpInst *createCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb, Module *m);

    CmpOp getCmpOp() { return cmp_op_; }

    void negation();

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final {
        return new CmpInst(getType(), cmp_op_, getOperand(0), getOperand(1), bb);
    }

private:
    CmpInst(Type *ty, CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb); 
    //~ void assert_valid();
    
private:
    CmpOp cmp_op_;
    
};

class FCmpInst : public Instruction {
  public:
    static FCmpInst *createFCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb, Module *m);

    CmpOp getCmpOp() { return cmp_op_; }

    void negation();

    virtual std::string print() override;
    Instruction *copyInst(BasicBlock *bb) override final {
        return new FCmpInst(getType(), cmp_op_, getOperand(0), getOperand(1), bb);
    }

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

private:
    GetElementPtrInst(Value *ptr, std::vector<Value *> idxs, BasicBlock *bb);

private:
    Type *element_ty_;
};


class StoreInst : public Instruction {
public:
    static StoreInst *createStore(Value *val, Value *ptr, BasicBlock *bb);

    Value *getRval() { return this->getOperand(0); }
    Value *getLval() { return this->getOperand(1); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new StoreInst(getOperand(0),getOperand(1),bb);
    }

private:
    StoreInst(Value *val, Value *ptr, BasicBlock *bb);
};

//& 加速使用全0初始化数组的代码优化分析和代码生成
class MemsetInst : public Instruction {
public:
    static MemsetInst *create_memset(Value *ptr, BasicBlock *bb);

    Value *get_lval() { return this->getOperand(0); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new MemsetInst(getOperand(0),bb);
    }

private:
    MemsetInst(Value *ptr, BasicBlock *bb);
};

class LoadInst : public Instruction {
public:
    static LoadInst *create_load(Type *ty, Value *ptr, BasicBlock *bb);
    
    Value * get_lval() { return this->getOperand(0); }

    Type *get_load_type() const { return static_cast<PointerType *>(getOperand(0)->getType())->getElementType(); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new LoadInst(getType(),getOperand(0),bb);
    }

private:
    LoadInst(Type *ty, Value *ptr, BasicBlock *bb);
};


class AllocaInst : public Instruction {
public:
    static AllocaInst *create_alloca(Type *ty, BasicBlock *bb);

    Type *get_alloca_type() const { return alloca_ty_; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new AllocaInst(alloca_ty_,bb);
    }

private:
    AllocaInst(Type *ty, BasicBlock *bb);

private:
    Type *alloca_ty_;
};

class ZextInst : public Instruction {
public:
    static ZextInst *create_zext(Value *val, Type *ty, BasicBlock *bb);

    Type *get_dest_type() const { return dest_ty_; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new ZextInst(getInstrType(),getOperand(0),dest_ty_,bb);
    }

private:
    ZextInst(OpID op, Value *val, Type *ty, BasicBlock *bb);

private:
  Type *dest_ty_;
};

class SiToFpInst : public Instruction {
public:
    static SiToFpInst *create_sitofp(Value *val, Type *ty, BasicBlock *bb);

    Type *get_dest_type() const { return dest_ty_; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new SiToFpInst(getInstrType(), getOperand(0), get_dest_type(), bb);
    }

private:
    SiToFpInst(OpID op, Value *val, Type *ty, BasicBlock *bb);

private:
    Type *dest_ty_;
};

class FpToSiInst : public Instruction {
public:
    static FpToSiInst *create_fptosi(Value *val, Type *ty, BasicBlock *bb);

    Type *get_dest_type() const { return dest_ty_; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new FpToSiInst(getInstrType(), getOperand(0), get_dest_type(), bb);
    }

private:
    FpToSiInst(OpID op, Value *val, Type *ty, BasicBlock *bb);

private:
    Type *dest_ty_;
};

class PhiInst : public Instruction {
public:
    static PhiInst *create_phi(Type *ty, BasicBlock *bb);

    Value *get_lval() { return l_val_; }
    void set_lval(Value *l_val) { l_val_ = l_val; }

    void add_phi_pair_operand(Value *val, Value *pre_bb) {
        this->addOperand(val);
        this->addOperand(pre_bb);
    }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = create_phi(getType(), bb);
        for (auto op : getOperands()){
            new_inst->addOperand(op);
        }
        return new_inst;
    }
private:
     PhiInst(OpID op, std::vector<Value *> vals, std::vector<BasicBlock *> val_bbs, Type *ty, BasicBlock *bb);

private:
    Value *l_val_;
};

class CmpBrInst: public Instruction {

public:
    static CmpBrInst *create_cmpbr(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb, Module *m);

    CmpOp get_cmp_op() { return cmp_op_; }

    bool is_cmp_br() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = new CmpBrInst(cmp_op_,getOperand(0),getOperand(1),bb);
        new_inst->setOperand(2, getOperand(2));
        new_inst->setOperand(3, getOperand(3));
        return new_inst;
    }

private:
    CmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb);
    CmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);
private:
    CmpOp cmp_op_;

};


class FCmpBrInst : public Instruction {
public:
    static FCmpBrInst *create_fcmpbr(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb, Module *m);

    CmpOp get_cmp_op() { return cmp_op_; }

    bool is_fcmp_br() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = new FCmpBrInst(cmp_op_,getOperand(0),getOperand(1),bb);
        new_inst->setOperand(2, getOperand(2));
        new_inst->setOperand(3, getOperand(3));
        return new_inst;
    }


private:
    FCmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb);
    FCmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);
private:
    CmpOp cmp_op_;
};

class LoadOffsetInst: public Instruction {
public:
    static LoadOffsetInst *create_loadoffset(Type *ty, Value *ptr, Value *offset, BasicBlock *bb);

    Value *get_lval() { return this->getOperand(0); }
    Value *get_offset() { return this->getOperand(1); }

    Type *get_load_type() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = new LoadOffsetInst(getType(), getOperand(0), bb);
        new_inst->setOperand(1, getOperand(1));
        return new_inst;
    }

private:
    LoadOffsetInst(Type *ty, Value *ptr, Value *offset, BasicBlock *bb);
    LoadOffsetInst(Type *ty, Value *ptr, BasicBlock *bb);
};


class StoreOffsetInst: public Instruction {

public:
    static StoreOffsetInst *create_storeoffset(Value *val, Value *ptr, Value *offset, BasicBlock *bb);

    Type *get_store_type() const;

    Value *get_rval() { return this->getOperand(0); }
    Value *get_lval() { return this->getOperand(1); }
    Value *get_offset() { return this->getOperand(2); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto new_inst = new StoreOffsetInst(getOperand(0), getOperand(1), bb);
        new_inst->setOperand(2, getOperand(2));
        return new_inst;
    }

private:
    StoreOffsetInst(Value *val, Value *ptr, Value *offset, BasicBlock *bb);
    StoreOffsetInst(Value *val, Value *ptr, BasicBlock *bb);
};


#endif





