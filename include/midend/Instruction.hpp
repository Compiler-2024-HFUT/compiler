#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <vector>

#include "User.hpp"
#include "Type.hpp"
#include "Constant.hpp"
#include "BasicBlock.hpp"

class BasicBlock;
class Function;

class Instruction : public User {
public:
    enum OPID {
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
    OPID getInstrType() { return opid; }

    static std::string getInstrOpName(OPID id) {
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
    //& make instruction, auto insert to bb, ty here is result type
    Instruction(Type *ty, OPID id, unsigned numops, BasicBlock *parent);
    Instruction(Type *ty, OPID id, unsigned numops);

    inline const BasicBlock *getParent() const { return parent; }
    inline BasicBlock *getParent() { return parent; }
    void setParent(BasicBlock *parent) { this->parent = parent; }

    //// Return the function this instruction belongs to.
    Function *getFunction();
    
    Module *getModule();

    bool isVoid() {
        return ((opid == ret) || (opid == br) || (opid == store) || (opid == cmpbr) || (opid == fcmpbr) || (opid == storeoffset) || (opid == memset) ||
                (opid == call && this->getType()->isVoidType()));
    }

    bool isRet() { return opid == ret; } 
    bool isBr() { return opid ==  br; } 

    bool isAdd() { return opid ==  add; } 
    bool isSub() { return opid ==  sub; }
    bool isMul() { return opid ==  mul; } 
    bool isMul64() { return opid == mul64; }
    bool isDiv() { return opid ==  sdiv; }
    bool isRem() { return opid ==  srem; } 

    bool isFadd() { return opid ==  fadd; } 
    bool isFsub() { return opid ==  fsub; } 
    bool isFmul() { return opid ==  fmul; } 
    bool isFdiv() { return opid ==  fdiv; } 

    bool isAlloca() { return opid ==  alloca; } 
    bool isLoad() { return opid ==  load; } 
    bool isStore() { return opid ==  store; } 
    bool isMemset() { return opid == memset; }

    bool isCmp() { return opid ==  cmp; }
    bool isFcmp() { return opid ==  fcmp; } 
    bool isPhi() { return opid ==  phi; } 
    bool isCall() { return opid ==  call; }
    bool isGep(){ return opid ==  getelementptr; } 

    bool isAnd() { return opid ==  land; } 
    bool isOr() { return opid ==  lor; }
    bool isXor() { return opid == lxor; } 

    bool isAsr() { return opid ==  asr; }
    bool isLsl() { return opid ==  shl; } 
    bool isLsr() { return opid ==  lsr; } 
    bool isAsr64() { return opid ==  asr64; }
    bool isLsl64() { return opid ==  shl64; } 
    bool isLsr64() { return opid ==  lsr64; }

    bool isZext() { return opid ==  zext; } 
    
    bool isFptosi(){ return opid ==  fptosi; } 
    bool isSitofp(){ return opid ==  sitofp; } 

    bool isCmpBr() { return opid == cmpbr; }
    bool isFCmpBr() { return opid == fcmpbr; }
    bool isLoadOffset() { return opid == loadoffset; }
    bool isStoreOffset() { return opid == storeoffset; }

    bool isExtendBr() { return (opid ==  br || opid == cmpbr || opid == fcmpbr); }
    bool isExtendCondBr() const { return getNumOpe() == 3 || getNumOpe() == 4; }


    bool isIntBinary() {
        return (isAdd() || isSub() || isMul() || isDiv() || isRem() || isMul64() ||
                isAnd() || isOr() || isXor() || 
                isAsr() || isLsl() || isLsr() || isAsr64() || isLsl64() || isLsr64()) &&
               (getNumOpe() == 2);
    }

    bool isFloatBinary() {
        return (isFadd() || isFsub() || isFmul() || isFdiv()) && (getNumOpe() == 2);
    }

    bool isBinary() {
        return isIntBinary() || isFloatBinary();

    }

    bool isTerminator() { return isBr() || isRet() || isCmpBr() || isFCmpBr(); }

    void setId(int id) { id = id; }
    int getId() { return id; }

    virtual Instruction *copyInst(BasicBlock *bb) = 0;

private:
    OPID opid;
    unsigned numops;
    BasicBlock* parent;
    int id;
};

class BinaryInst : public Instruction {
public:
    static BinaryInst *makeAdd(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeSub(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeMul(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeMul64(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeSDiv(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeSRem(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeFAdd(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeFSub(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeFMul(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeFDiv(Value *v1, Value *v2, BasicBlock *bb, Module *m);

    static BinaryInst *makeAnd(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeOr(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeXor(Value *v1, Value *v2, BasicBlock *bb, Module *m);

    static BinaryInst *makeAsr(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeLsl(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeLsr(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeAsr64(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeLsl64(Value *v1, Value *v2, BasicBlock *bb, Module *m);
    static BinaryInst *makeLsr64(Value *v1, Value *v2, BasicBlock *bb, Module *m);

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final {
        return new BinaryInst(getType(), getInstrType(), getOpe(0), getOpe(1), bb);
    }

private:
    BinaryInst(Type *ty, OPID id, Value *v1, Value *v2, BasicBlock *bb); 

    //~ void assertvalid();
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
    static CmpInst *makeCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb, Module *m);

    CmpOp getCmpOp() { return cmpop; }

    void negation();

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final {
        return new CmpInst(getType(), cmpop, getOpe(0), getOpe(1), bb);
    }

private:
    CmpInst(Type *ty, CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb); 
    //~ void assertvalid();
    
private:
    CmpOp cmpop;
    
};

class FCmpInst : public Instruction {
  public:
    static FCmpInst *makeFCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb, Module *m);

    CmpOp getCmpOp() { return cmpop; }

    void negation();

    virtual std::string print() override;
    Instruction *copyInst(BasicBlock *bb) override final {
        return new FCmpInst(getType(), cmpop, getOpe(0), getOpe(1), bb);
    }

  private:
    FCmpInst(Type *ty, CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);

    //~ void assertvalid();

  private:
    CmpOp cmpop;
};

class CallInst : public Instruction {
public:
    static CallInst *makeCall(Function *func, std::vector<Value *>args, BasicBlock *bb);
    
    FuncType *getFuncType() const { return static_cast<FuncType *>(getOpe(0)->getType()); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        std::vector<Value *> args;
        for (auto i = 1; i < getNumOpe(); i++){
            args.push_back(getOpe(i));
        }
        auto newInst = new CallInst(getFuncType()->getResultType(),args,bb);
        newInst->setOpe(0, getOpe(0));
        return newInst;
    }

protected:
    CallInst(Function *func, std::vector<Value *>args, BasicBlock *bb);
    CallInst(Type *retty, std::vector<Value *> args, BasicBlock *bb);
};

class BranchInst : public Instruction {
public:
    static BranchInst *makeCondBr(Value *cond, BasicBlock *iftrue, BasicBlock *iffalse, BasicBlock *bb);
    static BranchInst *makeBr(BasicBlock *iftrue, BasicBlock *bb);

   
    bool isCondBr() const { return getNumOpe() == 3; }
    // bool isextendcondbr() const { return getNumOpe() == 3 || getNumOpe() == 4; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        if (getNumOpe() == 1){
            auto newInst = new BranchInst(bb);
            newInst->setOpe(0, getOpe(0));
            return newInst;
        } else {
            auto newInst = new BranchInst(getOpe(0),bb);
            newInst->setOpe(1, getOpe(1));
            newInst->setOpe(2, getOpe(2));
            return newInst;
        }
    }

private:
    BranchInst(Value *cond, BasicBlock *iftrue, BasicBlock *iffalse,
               BasicBlock *bb);
    BranchInst(BasicBlock *iftrue, BasicBlock *bb);
    BranchInst(BasicBlock *bb);
    BranchInst(Value *cond, BasicBlock *bb);
};

class ReturnInst : public Instruction {
public:
    static ReturnInst *makeRet(Value *val, BasicBlock *bb);
    static ReturnInst *makeVoidRet(BasicBlock *bb);

    bool isVoidRet() const { return getNumOpe() == 0; }

    Type * getRetType() const { return getOpe(0)->getType(); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        if (isVoidRet()){
            return new ReturnInst(bb);
        } else {
            return new ReturnInst(getOpe(0),bb);
        }
    }

private:
    ReturnInst(Value *val, BasicBlock *bb);
    ReturnInst(BasicBlock *bb);
};

class GetElementPtrInst : public Instruction {
public:
    static Type *getElementType(Value *ptr, std::vector<Value *> idxs);
    static GetElementPtrInst *makeGep(Value *ptr, std::vector<Value *> idxs, BasicBlock *bb);
    Type *getElementType() const { return elementty; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        std::vector<Value *> idxs;
        for (auto i = 1; i < getNumOpe(); i++) {
            idxs.push_back(getOpe(i));
        }
        return new GetElementPtrInst(getOpe(0),idxs,bb);
    }

private:
    GetElementPtrInst(Value *ptr, std::vector<Value *> idxs, BasicBlock *bb);

private:
    Type *elementty;
};


class StoreInst : public Instruction {
public:
    static StoreInst *makeStore(Value *val, Value *ptr, BasicBlock *bb);

    Value *getRVal() { return this->getOpe(0); }
    Value *getLVal() { return this->getOpe(1); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new StoreInst(getOpe(0),getOpe(1),bb);
    }

private:
    StoreInst(Value *val, Value *ptr, BasicBlock *bb);
};

//& 加速使用全0初始化数组的代码优化分析和代码生成
class MemsetInst : public Instruction {
public:
    static MemsetInst *makeMemset(Value *ptr, BasicBlock *bb);

    Value *getLVal() { return this->getOpe(0); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new MemsetInst(getOpe(0),bb);
    }

private:
    MemsetInst(Value *ptr, BasicBlock *bb);
};

class LoadInst : public Instruction {
public:
    static LoadInst *makeLoad(Type *ty, Value *ptr, BasicBlock *bb);
    
    Value * getLVal() { return this->getOpe(0); }

    Type *getLoadType() const { return static_cast<PtrType *>(getOpe(0)->getType())->getPtrElementType(); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new LoadInst(getType(),getOpe(0),bb);
    }

private:
    LoadInst(Type *ty, Value *ptr, BasicBlock *bb);
};


class AllocaInst : public Instruction {
public:
    static AllocaInst *makeAlloca(Type *ty, BasicBlock *bb);

    Type *getAllocaType() const { return allocaty; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new AllocaInst(allocaty,bb);
    }

private:
    AllocaInst(Type *ty, BasicBlock *bb);

private:
    Type *allocaty;
};

class ZextInst : public Instruction {
public:
    static ZextInst *makeZext(Value *val, Type *ty, BasicBlock *bb);

    Type *getDestType() const { return destty; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new ZextInst(getInstrType(),getOpe(0),destty,bb);
    }

private:
    ZextInst(OPID op, Value *val, Type *ty, BasicBlock *bb);

private:
  Type *destty;
};

class SiToFpInst : public Instruction {
public:
    static SiToFpInst *makeSitofp(Value *val, Type *ty, BasicBlock *bb);

    Type *getDestType() const { return destty; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new SiToFpInst(getInstrType(), getOpe(0), getDestType(), bb);
    }

private:
    SiToFpInst(OPID op, Value *val, Type *ty, BasicBlock *bb);

private:
    Type *destty;
};

class FpToSiInst : public Instruction {
public:
    static FpToSiInst *makeFptosi(Value *val, Type *ty, BasicBlock *bb);

    Type *getDestType() const { return destty; }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        return new FpToSiInst(getInstrType(), getOpe(0), getDestType(), bb);
    }

private:
    FpToSiInst(OPID op, Value *val, Type *ty, BasicBlock *bb);

private:
    Type *destty;
};

class PhiInst : public Instruction {
public:
    static PhiInst *makePhi(Type *ty, BasicBlock *bb);

    Value *getLVal() { return lval; }
    void setLVal(Value *lval) { lval = lval; }

    void addPhiPairPperand(Value *val, Value *prebb) {
        this->addOpe(val);
        this->addOpe(prebb);
    }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto newInst = makePhi(getType(), bb);
        for (auto op : getDefUseList()){
            newInst->addOpe(op);
        }
        return newInst;
    }
private:
     PhiInst(OPID op, std::vector<Value *> vals, std::vector<BasicBlock *> valbbs, Type *ty, BasicBlock *bb);

private:
    Value *lval;
};

class CmpBrInst: public Instruction {

public:
    static CmpBrInst *makeCmpBr(CmpOp op, Value *lhs, Value *rhs, BasicBlock *iftrue, BasicBlock *iffalse, BasicBlock *bb, Module *m);

    CmpOp getCmpOp() { return cmpop; }

    bool isCmpBr() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto newInst = new CmpBrInst(cmpop,getOpe(0),getOpe(1),bb);
        newInst->setOpe(2, getOpe(2));
        newInst->setOpe(3, getOpe(3));
        return newInst;
    }

private:
    CmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *iftrue, BasicBlock *iffalse, BasicBlock *bb);
    CmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);
private:
    CmpOp cmpop;

};


class FCmpBrInst : public Instruction {
public:
    static FCmpBrInst *makeFCmpBr(CmpOp op, Value *lhs, Value *rhs, BasicBlock *iftrue, BasicBlock *iffalse, BasicBlock *bb, Module *m);

    CmpOp getCmpOp() { return cmpop; }

    bool isFCmpBr() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto newInst = new FCmpBrInst(cmpop,getOpe(0),getOpe(1),bb);
        newInst->setOpe(2, getOpe(2));
        newInst->setOpe(3, getOpe(3));
        return newInst;
    }


private:
    FCmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *iftrue, BasicBlock *iffalse, BasicBlock *bb);
    FCmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb);
private:
    CmpOp cmpop;
};

class LoadOffsetInst: public Instruction {
public:
    static LoadOffsetInst *makeLoadOffset(Type *ty, Value *ptr, Value *offset, BasicBlock *bb);

    Value *getLVal() { return this->getOpe(0); }
    Value *getOffset() { return this->getOpe(1); }

    Type *getLoadType() const;

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto newInst = new LoadOffsetInst(getType(), getOpe(0), bb);
        newInst->setOpe(1, getOpe(1));
        return newInst;
    }

private:
    LoadOffsetInst(Type *ty, Value *ptr, Value *offset, BasicBlock *bb);
    LoadOffsetInst(Type *ty, Value *ptr, BasicBlock *bb);
};


class StoreOffsetInst: public Instruction {

public:
    static StoreOffsetInst *makeStoreOffset(Value *val, Value *ptr, Value *offset, BasicBlock *bb);

    Type *getStoreType() const;

    Value *getRVal() { return this->getOpe(0); }
    Value *getLVal() { return this->getOpe(1); }
    Value *getOffset() { return this->getOpe(2); }

    virtual std::string print() override;

    Instruction *copyInst(BasicBlock *bb) override final{
        auto newInst = new StoreOffsetInst(getOpe(0), getOpe(1), bb);
        newInst->setOpe(2, getOpe(2));
        return newInst;
    }

private:
    StoreOffsetInst(Value *val, Value *ptr, Value *offset, BasicBlock *bb);
    StoreOffsetInst(Value *val, Value *ptr, BasicBlock *bb);
};

#endif






