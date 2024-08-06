#include <array>

#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
static  std::array<std::string,38> const INSTR_ID2STRING{
    "ret",
    "br",
    "add",
    "sub",
    "mul",
    "mul64",
    "sdiv",
    "srem",
    "fadd",
    "fsub",
    "fmul",
    "fdiv",
    "alloca",
    "load",
    "store",
    "memset",
    "icmp",
    "fcmp",
    "phi",
    "call",
    "getelementptr",
    "and",
    "or",
    "xor",
    "ashr",
    "shl",
    "lshr",
    "asr64",
    "shl64",
    "lsr64",
    "zext",
    "fptosi",
    "sitofp",
    "cmpbr",
    "fcmpbr",
    "loadoffset",
    "storeoffset",
    "select",
    };
__attribute__((always_inline)) std::string OpID2String(Instruction::OpID op){
    return INSTR_ID2STRING[static_cast<int>(op)];
}
static std::array<std::string,6> const CMP2STR={
    "eq",
    "ne",
    "sgt",
    "sge",
    "slt",
    "sle",
};
__attribute__((always_inline)) std::string Cmp2String(CmpOp op){
    return CMP2STR[op];
}
static std::array<std::string,6> const FCMP2STR={
    "ueq",
    "une",
    "ugt",
    "uge",
    "ult",
    "ule",
};
__attribute__((always_inline)) std::string FCmp2String(CmpOp op){
    return FCMP2STR[op];
}

//& Instruction
Instruction::Instruction(Type *ty, OpID id, unsigned num_ops, BasicBlock *parent)
    : User(ty, "", num_ops), op_id_(id), parent_(parent) {
    parent_->addInstruction(this);
}

Instruction::Instruction(Type *ty, OpID id, unsigned num_ops)
    : User(ty, "", num_ops), op_id_(id), parent_(nullptr) {}


Function *Instruction::getFunction() {
    return parent_->getParent();
}

//& BinaryInst

BinaryInst::BinaryInst(Type *ty, OpID id, Value *v1, Value *v2)
    : Instruction(ty, id, 2) {
        setOperand(0, v1);
        setOperand(1, v2);
}

BinaryInst::BinaryInst(Type *ty, OpID id, Value *v1, Value *v2, BasicBlock *bb)
    : Instruction(ty, id, 2, bb) {
        setOperand(0, v1);
        setOperand(1, v2);
}
BinaryInst *BinaryInst::create( OpID id,Value *v1, Value *v2) {
    return new BinaryInst(v1->getType(), id, v1, v2);
}

BinaryInst *BinaryInst::createAdd(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(v1->getType()->isPointerType() ? v1->getType() : v2->getType(), Instruction::OpID::add, v1, v2, bb);
}

BinaryInst *BinaryInst::createSub(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::sub, v1, v2, bb);
}

BinaryInst *BinaryInst::createMul(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::mul, v1, v2, bb);
}

BinaryInst *BinaryInst::createMul64(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::mul64, v1, v2, bb);
}

BinaryInst *BinaryInst::createSDiv(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::sdiv, v1, v2, bb);
}

BinaryInst *BinaryInst::createSRem(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::srem, v1, v2, bb);
}

BinaryInst *BinaryInst::createFAdd(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getFloatType(), Instruction::OpID::fadd, v1, v2, bb);
}

BinaryInst *BinaryInst::createFSub(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getFloatType(), Instruction::OpID::fsub, v1, v2, bb);
}

BinaryInst *BinaryInst::createFMul(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getFloatType(), Instruction::OpID::fmul, v1, v2, bb);
}

BinaryInst *BinaryInst::createFDiv(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getFloatType(), Instruction::OpID::fdiv, v1, v2, bb);
}

BinaryInst *BinaryInst::createAnd(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::land, v1, v2, bb);
}

BinaryInst *BinaryInst::createOr(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::lor, v1, v2, bb);
}

BinaryInst *BinaryInst::createXor(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::lxor, v1, v2, bb);
}

BinaryInst *BinaryInst::createAsr(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::asr, v1, v2, bb);
}

BinaryInst *BinaryInst::createLsl(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::shl, v1, v2, bb);
}

BinaryInst *BinaryInst::createLsr(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::lsr, v1, v2, bb);
}

BinaryInst *BinaryInst::createAsr64(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::asr64, v1, v2, bb);
}

BinaryInst *BinaryInst::createLsl64(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::shl64, v1, v2, bb);
}

BinaryInst *BinaryInst::createLsr64(Value *v1, Value *v2, BasicBlock *bb) {
    return new BinaryInst(Type::getInt32Type(), Instruction::OpID::lsr64, v1, v2, bb);
}


//& CmpInst
CmpInst::CmpInst(Type *ty, CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb)
    : Instruction(ty, Instruction::OpID::cmp, 2, bb), cmp_op_(op) {
    setOperand(0, lhs);
    setOperand(1, rhs);
}

CmpInst *CmpInst::createCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb) {
    return new CmpInst(Type::getInt1Type(), op, lhs, rhs, bb);
}

void CmpInst::negation() {
    switch(getCmpOp()) {
        case CmpOp::EQ:
            cmp_op_ = CmpOp::NE;
            break;
        case CmpOp::NE:
            cmp_op_ = CmpOp::EQ;
            break;
        case CmpOp::GT:
            cmp_op_ = CmpOp::LE;
            break;
        case CmpOp::GE:
            cmp_op_ = CmpOp::LT;
            break;
        case CmpOp::LT:
            cmp_op_ = CmpOp::GE;
            break;
        case CmpOp::LE:
            cmp_op_ = CmpOp::GT;
            break;
        default:
            break;
    }
    return ;
}

//& FCmpInst

FCmpInst::FCmpInst(Type *ty, CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb)
    : Instruction(ty, Instruction::OpID::fcmp, 2, bb), cmp_op_(op) {
    setOperand(0, lhs);
    setOperand(1, rhs);
}

FCmpInst *FCmpInst::createFCmp(CmpOp op, Value *lhs, Value *rhs, BasicBlock *bb) {
    return new FCmpInst(Type::getInt1Type(), op, lhs, rhs, bb);
}

void FCmpInst::negation() {
    switch(getCmpOp()) {
        case CmpOp::EQ:
            cmp_op_ = CmpOp::NE;
            break;
        case CmpOp::NE:
            cmp_op_ = CmpOp::EQ;
            break;
        case CmpOp::GT:
            cmp_op_ = CmpOp::LE;
            break;
        case CmpOp::GE:
            cmp_op_ = CmpOp::LT;
            break;
        case CmpOp::LT:
            cmp_op_ = CmpOp::GE;
            break;
        case CmpOp::LE:
            cmp_op_ = CmpOp::GT;
            break;
        default:
            break;
    }
    return ;
}


//& CallInst
CallInst::CallInst(Function *func, std::vector<Value *> args, BasicBlock *bb)
    : Instruction(func->getReturnType(), Instruction::OpID::call, args.size()+1, bb) {
    assert(func->getNumOfArgs() == args.size());
    int num_ops = args.size() + 1;
    setOperand(0, func);
    for (int i = 1; i < num_ops; i++) {
        setOperand(i, args[i - 1]);
    }
}

CallInst::CallInst(Type *ret_ty, std::vector<Value *> args, BasicBlock *bb)
    : Instruction(ret_ty, Instruction::OpID::call, args.size() + 1, bb) {
    int num_ops = args.size() + 1;
    for (int i = 1; i < num_ops; i++) {
        setOperand(i, args[i-1]);
    }
}

CallInst *CallInst::createCall(Function *func, std::vector<Value *> args, BasicBlock *bb) {
    return new CallInst(func, args, bb);
}

//& BranchInst
BranchInst::BranchInst(Value *cond, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::br, 3, bb) {
    setOperand(0, cond);
    setOperand(1, if_true);
    setOperand(2, if_false);
}

BranchInst::BranchInst(BasicBlock *if_true, BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::br, 1, bb) {
    setOperand(0, if_true);
}

BranchInst::BranchInst(Value *cond, BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::br, 3, bb) {
    setOperand(0, cond);
}

BranchInst::BranchInst(BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::br, 1, bb) {
    //nothing to do
}

BranchInst *BranchInst::createCondBr(Value *cond, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb) {
    if_true->addPreBasicBlock(bb);
    if_false->addPreBasicBlock(bb);
    bb->addSuccBasicBlock(if_false);
    bb->addSuccBasicBlock(if_true);

    return new BranchInst(cond, if_true, if_false, bb);
}

BranchInst *BranchInst::createBr(BasicBlock *if_true, BasicBlock *bb) {
    if_true->addPreBasicBlock(bb);
    bb->addSuccBasicBlock(if_true);

    return new BranchInst(if_true, bb);
}


//& ReturnInst

ReturnInst::ReturnInst(Value *val, BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::ret, 1, bb) {
    setOperand(0, val);
}

ReturnInst::ReturnInst(BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::ret, 0, bb) {

}

ReturnInst *ReturnInst::createRet(Value *val, BasicBlock *bb) {
    return new ReturnInst(val, bb);
}

ReturnInst *ReturnInst::createVoidRet(BasicBlock *bb) {
    return new ReturnInst(bb);
}

//& GetElementPtrInst
GetElementPtrInst::GetElementPtrInst(Value *ptr, std::vector<Value *> idxs, BasicBlock *bb)
    : Instruction(PointerType::get(getElementType(ptr, idxs)), Instruction::OpID::getelementptr, 1+idxs.size(), bb) {
    setOperand(0, ptr);
    for (int i = 0; i < idxs.size(); i++) {
        setOperand(i + 1, idxs[i]);
    }
    element_ty_ = getElementType(ptr, idxs);
}

Type *GetElementPtrInst::getElementType(Value *ptr, std::vector<Value *> idxs) {
    Type *ty = ptr->getType()->getPointerElementType();
    assert("GetElementPtrInst ptr is wrong type" &&
           (ty->isArrayType() || ty->isIntegerType() || ty->isFloatType()));
    if (ty->isArrayType()) {
        ArrayType *arr_ty = static_cast<ArrayType *>(ty);
        for (int i = 1; i < idxs.size(); i++) {
            ty = arr_ty->getElementType();
            if (i < idxs.size() - 1) {
                assert(ty->isArrayType() && "Index error!");
            }
            if (ty->isArrayType()) {
                arr_ty = static_cast<ArrayType *>(ty);
            }
        }
    }
    return ty;
}

GetElementPtrInst *GetElementPtrInst::createGep(Value *ptr, std::vector<Value *> idxs, BasicBlock *bb) {
    return new GetElementPtrInst(ptr, idxs, bb);
}
//& StoreInst
StoreInst::StoreInst(Value *val, Value *ptr, BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::store, 2, bb) {
    setOperand(0, val);
    setOperand(1, ptr);
}

StoreInst *StoreInst::createStore(Value *val, Value *ptr, BasicBlock *bb) {
    return new StoreInst(val, ptr, bb);
}

//& MemsetInst
MemsetInst::MemsetInst(Value *ptr, BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::memset, 1, bb) {
    setOperand(0, ptr);
}

MemsetInst *MemsetInst::createMemset(Value *ptr, BasicBlock *bb) {
    return new MemsetInst(ptr, bb);
}


//& LoadInst
LoadInst::LoadInst(Type *ty, Value *ptr, BasicBlock *bb)
    : Instruction(ty, Instruction::OpID::load, 1, bb) {
    assert(ptr->getType()->isPointerType());
    assert(ty == static_cast<PointerType *>(ptr->getType())->getElementType());
    setOperand(0, ptr);
}

LoadInst *LoadInst::createLoad(Type *ty, Value *ptr, BasicBlock *bb) {
    return new LoadInst(ty, ptr, bb);
}


//& AllocInst
AllocaInst::AllocaInst(Type *ty, BasicBlock *bb)
    : Instruction(PointerType::get(ty), Instruction::OpID::alloca, 0, bb), alloca_ty_(ty) {

}

AllocaInst *AllocaInst::createAlloca(Type *ty, BasicBlock *bb) {
    return new AllocaInst(ty, bb);
}

//& ZextInst
ZextInst::ZextInst( Value *val, Type *ty, BasicBlock *bb)
    : Instruction(ty, Instruction::OpID::zext, 1, bb) {
    setOperand(0, val);
}

ZextInst *ZextInst::createZext(Value *val, Type *ty, BasicBlock *bb) {
    return new ZextInst(val, ty, bb);
}
//& FpToSiInst
FpToSiInst::FpToSiInst( Value *val, Type *ty, BasicBlock *bb)
    : Instruction(ty, Instruction::OpID::fptosi, 1, bb) {
    setOperand(0, val);
}

FpToSiInst *FpToSiInst::createFpToSi(Value *val, Type *ty, BasicBlock *bb) {
    return new FpToSiInst( val, ty, bb);
}

//& SiToFpInst
SiToFpInst::SiToFpInst( Value *val, Type *ty, BasicBlock *bb)
    : Instruction(ty, Instruction::OpID::sitofp, 1, bb) {
    setOperand(0, val);
}

SiToFpInst *SiToFpInst::createSiToFp(Value *val, Type *ty, BasicBlock *bb) {
    return new SiToFpInst( val, ty, bb);
}


//& PhiInst
PhiInst::PhiInst(OpID op, std::vector<Value *> vals, std::vector<BasicBlock *> val_bbs, Type *ty, BasicBlock *bb)
    : Instruction(ty, op, 2*vals.size()) {
    for (int i = 0; i < vals.size(); i++) {
        setOperand(2 * i, vals[i]);
        setOperand(2 * i + 1, val_bbs[i]);
    }
    this->setParent(bb);
}

PhiInst *PhiInst::createPhi(Type *ty, BasicBlock *bb) {
    return new PhiInst(Instruction::OpID::phi, {}, {}, ty, bb);
}


CmpBrInst::CmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb)
        :Instruction(Type::getVoidType(), Instruction::OpID::cmpbr, 4, bb), cmp_op_(op) {
    setOperand(2, if_true);
    setOperand(3, if_false);
    setOperand(0, lhs);
    setOperand(1, rhs);
}


CmpBrInst::CmpBrInst(CmpOp op, Value *lhs, Value *rhs,
            BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::cmpbr, 4, bb), cmp_op_(op) {
    setOperand(0, lhs);
    setOperand(1, rhs);
}

CmpBrInst *CmpBrInst::createCmpBr(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb) {
    if_true->addPreBasicBlock(bb);
    if_false->addPreBasicBlock(bb);
    bb->addSuccBasicBlock(if_false);
    bb->addSuccBasicBlock(if_true);

    return new CmpBrInst(op, lhs, rhs, if_true, if_false, bb);
}

FCmpBrInst::FCmpBrInst(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb)
        :Instruction(Type::getVoidType(), Instruction::OpID::fcmpbr, 4, bb), cmp_op_(op) {
    setOperand(2, if_true);
    setOperand(3, if_false);
    setOperand(0, lhs);
    setOperand(1, rhs);
}

FCmpBrInst::FCmpBrInst(CmpOp op, Value *lhs, Value *rhs,BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::fcmpbr, 4, bb), cmp_op_(op) {
    setOperand(1, rhs);
    setOperand(0, lhs);
}

FCmpBrInst *FCmpBrInst::createFCmpBr(CmpOp op, Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false, BasicBlock *bb) {
    if_true->addPreBasicBlock(bb);
    if_false->addPreBasicBlock(bb);
    bb->addSuccBasicBlock(if_false);
    bb->addSuccBasicBlock(if_true);

    return new FCmpBrInst(op, lhs, rhs, if_true, if_false, bb);
}

LoadOffsetInst::LoadOffsetInst(Type *ty, Value *ptr, Value *offset, BasicBlock *bb)
    : Instruction(ty, Instruction::OpID::loadoffset, 2, bb) {
    setOperand(0, ptr);
    setOperand(1, offset);
}

LoadOffsetInst::LoadOffsetInst(Type *ty, Value *ptr, BasicBlock *bb)
    : Instruction(ty, Instruction::OpID::loadoffset, 2, bb) {
    setOperand(0, ptr);
}

LoadOffsetInst *LoadOffsetInst::createLoadOffset(Type *ty, Value *ptr, Value *offset, BasicBlock *bb) {
    return new LoadOffsetInst(ty, ptr, offset, bb);
}

Type *LoadOffsetInst::getLoadType() const {
    return static_cast<PointerType *>(getOperand(0)->getType())->getElementType();
}

StoreOffsetInst::StoreOffsetInst(Value *val, Value *ptr, Value *offset, BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::storeoffset, 3, bb) {
    setOperand(0, val);
    setOperand(1, ptr);
    setOperand(2, offset);
}

StoreOffsetInst::StoreOffsetInst(Value *val, Value *ptr, BasicBlock *bb)
    : Instruction(Type::getVoidType(), Instruction::OpID::storeoffset, 3, bb) {
    setOperand(0, val);
    setOperand(1, ptr);
}

Type *StoreOffsetInst::getStoreType() const {
    return static_cast<PointerType *>(getOperand(1)->getType())->getElementType();
}

StoreOffsetInst *StoreOffsetInst::createStoreOffset(Value *val, Value *ptr, Value *offset, BasicBlock *bb) {
    return new StoreOffsetInst(val, ptr, offset, bb);
}

bool BinaryInst::isNeg(){
    bool ret=false;
    if(getInstrType()==Instruction::OpID::sub){
        if(auto co=dynamic_cast<ConstantInt*>(this->getOperand(0)))
            if(co->getValue()==0)ret=true;
    }else if(getInstrType()==Instruction::OpID::fsub){
        if(auto co=dynamic_cast<ConstantFP*>(this->getOperand(0)))
            if(co->getValue()==0.0f)ret=true;
    }
    return ret;
}
SelectInst::SelectInst(Type*type,Value *cond, Value *true_val, Value *false_val, BasicBlock *bb): Instruction(type, Instruction::OpID::select, 3, bb) {
    setOperand(0,cond);
    setOperand(1,true_val);
    setOperand(2,false_val);
}
SelectInst* SelectInst::createSelect(Type*type,Value *cond, Value *true_val, Value *false_val, BasicBlock *bb){
    return new SelectInst(type,cond, true_val, false_val,bb);
}