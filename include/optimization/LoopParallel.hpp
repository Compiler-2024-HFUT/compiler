#ifndef LOOP_PARALLEL_HPP
#define LOOP_PARALLEL_HPP

#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/Dominators.hpp"
#include "optimization/PassManager.hpp"

#include "analysis/funcAnalyse.hpp"

struct LoopBodyInfo {
    BasicBlock* loop;
    PhiInst* indvar;
    Value* bound;
    PhiInst* rec;
    bool recUsedByOuter;
    bool recUsedByInner;
    Value* recInnerStep;
    CallInst* recNext;
    BasicBlock* exit;
};

class LoopParallel : public FunctionPass {
    bool isNoSideEffectExpr(Instruction *inst);
    bool extractLoopBody(Function *func, Loop *loop, Module *mod, bool independent, bool allowInnermost,
                     bool allowInnerLoop, bool onlyAddRec, bool estimateBlockSizeForUnroll, bool needSubLoop,
                     bool convertReduceToAtomic, bool duplicateCmp, LoopBodyInfo *ret);
public:
    LoopParallel(Module *m, InfoManager *im) : FunctionPass(m, im){}
    ~LoopParallel(){};
    bool runImpl(Function*func);
    Modify runOnFunc(Function* func) override;



bool isMovableExpr(Instruction* inst, bool relaxedCtx) {
    if(!isNoSideEffectExpr(inst))
        return false;
    switch(inst->getInstrType()) {
        case Instruction::OpID::alloca:
        case Instruction::OpID::phi:
        case Instruction::OpID::load:
            return false;
        // It is not safe to speculate division, since SIGFPE may be raised.
        case Instruction::OpID::sdiv:
        case Instruction::OpID::srem:
        //可以保证测试用例的op1不为0
            return !relaxedCtx /*|| isNonZero(inst->getOperand(1))*/;
        default:
            return true;
    }
}
bool isConstant(Value* val) {
    if(dynamic_cast<Constant*>(val) || dynamic_cast<GlobalVariable*>(val))
        return true;
    if(dynamic_cast<Instruction*>(val)) {
        auto inst = (Instruction*)(val);
        if(!isMovableExpr(inst, false))
            return false;
        for(auto opreand : inst->getOperands()) {
            if(!isConstant(opreand))
                return false;
        }
        return true;
    }
    return false;
}
Value* expandConstant(Value* val,BasicBlock*to_insertbb,std::list<Instruction*>::iterator insert_pos) {
    if(dynamic_cast<Constant*>(val) || dynamic_cast<GlobalVariable*>(val))
        return val;
    bool isend=false;
    Instruction*insertins=0;

    if(insert_pos==to_insertbb->getInstructions().end()){
        isend=true;
    }else{
        insertins=*insert_pos;
    }

    if(auto old_ins=dynamic_cast<Instruction*>(val)) {
        auto inst = old_ins->copyInst(to_insertbb);
        to_insertbb->getInstructions().pop_back();
        for(int i=0;i< inst->getNumOperands();i++){
            if(isend)
                inst->replaceOperand(i,expandConstant(inst->getOperand(i), to_insertbb,to_insertbb->getInstructions().end()));
            else
                inst->replaceOperand(i,expandConstant(inst->getOperand(i), to_insertbb,to_insertbb->findInstruction(insertins)));

        }
        if(isend)
            to_insertbb->insertInstr(to_insertbb->getInstructions().end(),inst);
        else
            to_insertbb->insertInstr(to_insertbb->findInstruction(insertins),inst);
        return inst;
    }

        // reportUnreachable(CMMC_LOCATION());
    assert(0);
}
Function* getParallelFor(Module*module) {
        for(auto func : module->getFunctions())
            if(func->getName() == "xcParallelFor")
                return func;
        const auto i32 = IntegerType::get(32);
        const auto parallelForType =FunctionType::get (Type::getVoidType(), { i32, i32, PointerType::get(IntegerType::get(8)) });
        const auto func = Function::create( parallelForType,"xcParallelFor",module);
        module->getFunctions().pop_back();
        module->getFunctions().push_front(func);
        return func;
}
Function* getReduceAddI32(Module*module) {
    for(auto func : module->getFunctions())
        if(func->getName() == "xcReduceAddI32")
            return func;
    const auto i32 = IntegerType::get(32);
    const auto i32ptr = PointerType::get(i32);
    const auto reduceType = FunctionType::get(Type::getVoidType(), { i32ptr, i32 });
    const auto func = Function::create( reduceType,"xcReduceAddI32",module);
    module->getFunctions().pop_back();
    module->getFunctions().push_front(func);
    // func->attr().addAttr(FunctionAttribute::NoRecurse);
    // func->setLinkage(Linkage::Internal);
    // mod.add(func);
    return func;
}
Function* getReduceAddF32(Module* module) {
    for(auto func : module->getFunctions())
        if(func->getName() == "xcReduceAddF32")
            return func;
    const auto i32 = IntegerType::get(32);

    const auto f32 = FloatType::get();
    const auto f32ptr = PointerType::get(f32);
    const auto reduceType = FunctionType::get(Type::getVoidType(), { f32ptr, f32 });
    const auto func = Function::create( reduceType,"xcReduceAddF32",module);
    module->getFunctions().pop_back();
    module->getFunctions().push_front(func);

    return func;
}

auto getUniqueID (Module*module)->string {
    auto base = "xc_parallel_body_";
    for(int32_t id = 0;; ++id) {
        // const auto name = base.withID(id);
        const auto name=base+std::to_string(id);
        bool used = false;
        //global还是func
        for(auto global : module->getFunctions()) {
            if(global->getName() == name) {
                used = true;
                break;
            }
        }
        if(!used)
            return name;
    }
};
auto getUniqueIDStorage (Module*module) ->string{
    auto base = "xc_parallel_body_payload_";
    for(int32_t id = 0;; ++id) {
        const auto name=base+std::to_string(id);
        // const auto name = base.withID(id);
        bool used = false;
        //global还是func
        for(auto global : module->getGlobalVariables()) {
            if(global->getName() == name) {
                used = true;
                break;
            }
        }
        if(!used)
            return name;
    }
}
void popback_insertbefore(Instruction*before_this,BasicBlock*to_popback){
    auto to_insert=to_popback->getInstructions().back();
    to_popback->getInstructions().pop_back();
    to_popback->insertInstr(before_this,to_insert);
}

};


#endif