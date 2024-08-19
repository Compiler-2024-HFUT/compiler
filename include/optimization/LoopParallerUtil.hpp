#ifndef LOOP_PARALLER_UTIL_HPP
#define LOOP_PARALLER_UTIL_HPP

#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "midend/Type.hpp"
#include "midend/Module.hpp"
#include "midend/Value.hpp"
#include <cassert>
#include <string>
#include <unordered_set>
// static std::list<Instruction*>::iterator glb_insert_pos;
// static BasicBlock* glb_insert_bb;
static Function* getParallelFor(Module*module) {
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
static Function* getReduceAddI32(Module*module) {
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
static Function* getReduceAddF32(Module* module) {
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

static auto getUniqueID (Module*module)->string {
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
static auto getUniqueIDStorage (Module*module) ->string{
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
};
static bool isNoSideEffectExpr(Instruction*inst) {
    if(!inst->canbeOperand())
        return false;
    if(inst->isTerminator())
        return false;
    switch(inst->getInstrType()) {
        case Instruction::OpID::store:
        // case Instruction::OpID::AtomicAdd: {
        //     return false;
        // }
        case Instruction::OpID::call: {
            const auto callee = inst->getOperand(0);
            if(auto func = dynamic_cast<Function*>(callee)) {
                // auto& attr = func->attr();
                // return attr.hasAttr(FunctionAttribute::NoSideEffect) && attr.hasAttr(FunctionAttribute::Stateless) &&
                //     !attr.hasAttr(FunctionAttribute::NoReturn);
            }
            return false;
        }
        default:
            break;
    }

    return true;
}
static bool isMovableExpr(Instruction* inst, bool relaxedCtx) {
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
static bool isConstant(Value* val) {
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
static Value* expandConstant(Value* val,BasicBlock*to_insertbb,std::list<Instruction*>::iterator insert_pos) {
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
static void popback_insertbefore(Instruction*before_this,BasicBlock*to_popback){
    auto to_insert=to_popback->getInstructions().back();
    to_popback->getInstructions().pop_back();
    to_popback->insertInstr(before_this,to_insert);
}
struct LoopBodyInfo final {
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

void runimpl(Module*module_,InfoManager*info_man_){

    // auto initialRange = rangeInfo.query(loop.initial, dom, nullptr, 5);
    // const auto isAligned = (initialRange.knownZeros() & 3) == 3;
    // auto boundRange = rangeInfo.query(loop.bound, dom, nullptr, 5);
    // const auto needSubLoop = (boundRange - initialRange).maxSignedValue() < 400;




    ///******************extractLoopBody()******************************///
    Function*const parallelFor=getParallelFor(module_);

    LoopBodyInfo bodyInfo;
    bodyInfo.indvar->removePhiPairOperand(bodyInfo.loop);
    if(bodyInfo.rec)
        bodyInfo.rec->removePhiPairOperand(bodyInfo.loop);
    FunctionType* funcType = FunctionType::get(Type::getVoidType(), {});
    Function* const bodyFunc = Function::create(funcType,getUniqueID(module_),module_ );
    //将新函数放前面,
    module_->getFunctions().pop_back();
    module_->getFunctions().push_front(bodyFunc);
    // BasicBlock* curbb;

    /*
    设置 attribute
            bodyFunc->setLinkage(Linkage::Internal);
            bodyFunc->attr().addAttr(FunctionAttribute::NoRecurse).addAttr(FunctionAttribute::ParallelBody);
            if(isAligned)
                bodyFunc->attr().addAttr(FunctionAttribute::AlignedParallelBody
    */
    std::vector<std::pair<Value*, size_t>> payload;
    size_t totalSize = 0;
    size_t maxAlignment = 0;
    ConstantInt* givOffset = nullptr;
    IntegerType*const i32 = Type::getInt32Type();
    FloatType*const f32=Type::getFloatType();
    // const auto f32 = FloatingPointType::get(true);
    std::unordered_set<Value*> insertedParam;
    auto addArgument = [&](Value* param) {
        if(param == bodyInfo.indvar)
            return;
        if(isConstant(param))
            return;
        if(param == bodyInfo.rec && !bodyInfo.recUsedByOuter && !bodyInfo.recUsedByInner)
            return;

        if(!insertedParam.insert(param).second)
            return;
        //align
        //align:ptr:8
        //align:int :   mBitWidth / 8 + (mBitWidth % 8 ? 1 : 0);  例如 32 / 8 + (32 % 8 ? 1 : 0);
        // const auto align = param->getType()->getAlignment(dataLayout);
        const auto align = (param->getType()->isPointerType()?8:4);
        const auto size = param->getType()->getSize();
        // const auto size = param->getType()->getSize(dataLayout);
        totalSize = (totalSize + align - 1) / align * align;
        maxAlignment = (maxAlignment> align?maxAlignment:align);
        if(param == bodyInfo.rec)
            givOffset = ConstantInt::get( static_cast</*intmax_t*/int>(totalSize));
        else
            payload.emplace_back(param, totalSize);
        totalSize += size;
    };
    for(auto param : bodyInfo.recNext->getArgs())
        addArgument(param);
    if(bodyInfo.recInnerStep)
        addArgument(bodyInfo.recInnerStep);
    /*
    
    不知如何处理
    ArrayType*newglb_type;
    
    
    */
    //(8>maxAlignment?8:maxAlignment)
    ArrayType*newglb_type;
    const auto payloadStorage =GlobalVariable::create(getUniqueIDStorage(module_),module_,newglb_type,false,ConstantZero::get(newglb_type));
    Function*cur_func=bodyFunc;
    auto &args=bodyFunc->getArgs();
    auto beg=args.front();
    auto __iter=args.begin();
    auto end=*(++__iter);
    // const auto beg = bodyFunc->addArg(i32);
    // const auto end = bodyFunc->addArg(i32);
    const auto entry = BasicBlock::create("entry",bodyFunc);
    const auto subLoop = BasicBlock::create("subloop",bodyFunc);
    const auto exit =  BasicBlock::create("exit",bodyFunc);
    // builder.setCurrentBlock(entry);
    // curbb=entry;
    const auto bodyExec = bodyInfo.recNext->copyInst(subLoop);
    subLoop->getInstructions().pop_back();
    ///*********************不知道如何处理***************//
    // BasicBlock*bodyExec;
    const auto indVar = PhiInst::createPhi(i32,entry);
    entry->addInstruction(indVar);
    indVar->addPhiPairOperand( beg,entry);
        PhiInst*giv;
        if(bodyInfo.rec){
            auto p=PhiInst::createPhi(bodyInfo.rec->getType(),entry);
            entry->addInstruction(p);
            giv=p;
        }else{
            giv=nullptr;
        }
       auto remapArgument = [&](Value* operand,Instruction*user,int op_offset,BasicBlock*insertbb,std::list<Instruction*>::iterator insert_pos){
                if(operand== bodyInfo.indvar) {
                    // operand->resetValue(indVar);
                    user->replaceOperand(op_offset,indVar);
                } else if(operand == bodyInfo.rec) {
                    user->replaceOperand(op_offset,giv);
                    // operand->resetValue(giv);
                } else {
                    if(isConstant(operand)) {
                        user->replaceOperand(op_offset,expandConstant(operand, insertbb,insert_pos));
                        // builder.setCurrentBlock(entry);
                    } else {
                        bool replaced = false;
                        for(auto& [param, offset] : payload) {
                            if(operand == param) {
                                const auto ptr = GetElementPtrInst::createGep(payloadStorage, {ConstantInt::get(0),ConstantInt::get(givOffset->getValue()/4)},insertbb);
                                insertbb->getInstructions().pop_back();
                                insertbb->insertInstr(insert_pos,ptr);
                                const auto val = LoadInst::createLoad(ptr->getType()->getPointerElementType(),ptr,insertbb);
                                insertbb->getInstructions().pop_back();
                                insertbb->insertInstr(insert_pos,val);
                                user->replaceOperand(op_offset,val);
                                replaced = true;
                                break;
                            }
                        }
                        if(!replaced)
                            assert(0);
                            // reportUnreachable(CMMC_LOCATION());
                    }
                }
            };
        for(int i=0;i<bodyExec->getNumOperands();++i)
            remapArgument(bodyExec->getOperand(i),bodyExec,i,entry,entry->getInstructions().end());
    if(giv) {
        // giv->insertBefore(subLoop, subLoop->instructions().end());
        if(giv->getType()==i32) {
            if(bodyInfo.recUsedByInner) {
                //ptradd==>add i32* i32
                //ptradd==>add f32* i32
                //最好用gep代替
                const auto ptr = GetElementPtrInst::createGep(payloadStorage, {ConstantInt::get(0),ConstantInt::get(givOffset->getValue()/4)},entry);
                // Value*ptr;
                const auto initial = LoadInst::createLoad(i32,ptr,entry);
                const auto step = bodyInfo.recInnerStep;
                const auto offset = BinaryInst::createMul( beg, step,entry);
                //remaparg插入的指令会被放到offset前
                remapArgument(offset->getOperands().back(),offset,offset->getNumOperands()-1,entry,entry->findInstruction(offset));
                // builder.setCurrentBlock(entry);
                const auto realInitial = BinaryInst::createAdd(  initial, offset,entry);
                giv->addPhiPairOperand( realInitial,entry);
            } else
                giv->addPhiPairOperand( ConstantInt::get( 0),entry);
        } else if(giv->getType()==f32) {
            if(bodyInfo.recUsedByInner) {
                const auto ptr = GetElementPtrInst::createGep(payloadStorage,{ConstantInt::get(0),ConstantInt::get(givOffset->getValue()/4)}, entry);
                const auto initial = LoadInst::createLoad(f32,ptr,entry);
                const auto step = bodyInfo.recInnerStep;
                const auto offset = BinaryInst::createFMul(CastInst::createCastInst(f32,beg), step,entry);
                // builder.setInsertPoint(entry, offset->asIterator());
                auto offset_iter=entry->findInstruction(offset);
                //remaparg插入的指令会被放到offset前
                remapArgument(offset->getOperands().back(),offset,offset->getNumOperands()-1,entry,entry->findInstruction(offset));
                // builder.setCurrentBlock(entry);
                const auto realInitial = BinaryInst::createFAdd( initial, offset,entry);
                giv->addPhiPairOperand(realInitial,entry );
            } else
                giv->addPhiPairOperand(ConstantFP::get(0.0),entry);
        } else
            assert(0);
        giv->addPhiPairOperand( bodyExec,subLoop);
    }
    BranchInst::createBr(subLoop,entry);

    subLoop->addInstruction(bodyExec);
    // builder.setCurrentBlock(subLoop);
    const auto next = BinaryInst::createAdd( indVar, ConstantInt::get( 1),subLoop);
    indVar->addPhiPairOperand( next,subLoop);
    const auto cond = CmpInst::createCmp( CmpOp::LT, next, end,subLoop);
    BranchInst::createCondBr(cond, subLoop, exit,subLoop);

            // builder.setCurrentBlock(exit);
            if(giv && bodyInfo.recUsedByOuter) {
                const auto ptr = GetElementPtrInst::createGep(payloadStorage,{ ConstantInt::get(0),ConstantInt::get(givOffset->getValue()/4)}, exit);
                if(giv->getType()==i32) {
                    // if(module_.getTarget().isNativeSupported(InstructionID::AtomicAdd)) {
                    //     builder.makeOp<AtomicAddInst>(ptr, bodyExec);
                    // } else {
                    //     Function* reduceAddI32 = lookupReduceAddI32(mod);
                    //     builder.makeOp<FunctionCallInst>(reduceAddI32, std::vector<Value*>{ ptr, bodyExec });
                    // }
                } else if(giv->getType()==f32) {
                    // Function* reduceAddF32 = lookupReduceAddF32(mod);
                    // builder.makeOp<FunctionCallInst>(reduceAddF32, std::vector<Value*>{ ptr, bodyExec });
                } else
                    assert(0);
            }
            ReturnInst::createVoidRet(exit);

            // builder.setInsertPoint(bodyInfo.loop, bodyInfo.recNext);
            Value* givPtr = nullptr;
            if(giv && (bodyInfo.recUsedByOuter || bodyInfo.recUsedByInner)) {
                givPtr = GetElementPtrInst::createGep(payloadStorage, { ConstantInt::get(0),ConstantInt::get(givOffset->getValue()/4)},bodyInfo.recNext->getParent() );
                popback_insertbefore(bodyInfo.recNext,bodyInfo.recNext->getParent());
                StoreInst::createStore( bodyInfo.rec,givPtr,bodyInfo.rec->getParent());
                popback_insertbefore(bodyInfo.rec,bodyInfo.rec->getParent());
            }
            // for(auto [k, v] : payload) {
            //     const auto ptr = builder.makeOp<PtrAddInst>(payloadStorage, ConstantInteger::get(i32, static_cast<intmax_t>(v)),
            //                                                 PointerType::get(k->getType()));
            //     builder.makeOp<StoreInst>(ptr, k);
            // }

            // builder.makeOp<FunctionCallInst>(
            //     parallelFor,
            //     std::vector<Value*>{ bodyInfo.indvar, bodyInfo.bound,
            //                          builder.makeOp<FunctionPtrInst>(bodyFunc, PointerType::get(IntegerType::get(8))) });
            // if(giv && bodyInfo.recUsedByOuter) {
            //     const auto val = builder.makeOp<LoadInst>(givPtr);
            //     bodyInfo.recNext->replaceWith(val);
            // }
            // bodyInfo.loop->instructions().erase(bodyInfo.recNext->asIterator(), bodyInfo.loop->instructions().end());
            // builder.setCurrentBlock(bodyInfo.loop);
            // builder.makeOp<BranchInst>(bodyInfo.exit);

    BranchInst::createBr(bodyInfo.exit,bodyInfo.loop);
};
#endif