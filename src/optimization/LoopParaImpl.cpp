#include "analysis/Info.hpp"
#include "analysis/LoopInfo.hpp"
#include "midend/Function.hpp"
#include "optimization/LoopParallel.hpp"
#include "optimization/LoopParallerUtil.hpp"

bool LoopParallel::runImpl(Function*func){

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
    bodyFunc->is_Parallel=true;

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
    ArrayType*newglb_type=ArrayType::get(IntegerType::getInt32Type(),totalSize/4);
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

            if(giv && bodyInfo.recUsedByOuter) {
                const auto ptr = GetElementPtrInst::createGep(payloadStorage,{ ConstantInt::get(0),ConstantInt::get(givOffset->getValue()/4)}, exit);
                if(giv->getType()==i32) {
                    // if(module_.getTarget().isNativeSupported(InstructionID::AtomicAdd)) {
                    //      builder.makeOp<AtomicAddInst>(ptr, bodyExec);
                    AtomicAddInst::createAtomicAddInst(ptr, bodyExec, exit);
                    // } else {
                    //     Function* reduceAddI32 = lookupReduceAddI32(mod);
                    //     builder.makeOp<FunctionCallInst>(reduceAddI32, std::vector<Value*>{ ptr, bodyExec });
                    // }
                } else if(giv->getType()==f32) {
                    Function* reduceAddF32 = getReduceAddF32(module_); // ??
                    // builder.makeOp<FunctionCallInst>(reduceAddF32, std::vector<Value*>{ ptr, bodyExec });
                    CallInst::createCall(reduceAddF32, {ptr, bodyExec}, exit);
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
            for(auto [k, v] : payload) {
                // const auto ptr = GetElementPtrInst::createGep(payloadStorage, ConstantInt::get(i32, static_cast<intmax_t>(v)),
                //                                             PointerType::get(k->getType()));
                // builder.makeOp<StoreInst>(ptr, k);
                const auto ptr = GetElementPtrInst::createGep(payloadStorage, { ConstantInt::get(0),ConstantInt::get(static_cast<int>(v/4))},bodyInfo.recNext->getParent());
                popback_insertbefore(bodyInfo.recNext,bodyInfo.recNext->getParent());
                StoreInst::createStore(k,ptr,bodyInfo.recNext->getParent());
                popback_insertbefore(bodyInfo.recNext,bodyInfo.recNext->getParent());
                
            }

            // the order of inst is right??
            // builder.makeOp<FunctionCallInst>(
            //     parallelFor,
            //     std::vector<Value*>{ bodyInfo.indvar, bodyInfo.bound,
            //                          builder.makeOp<FunctionPtrInst>(bodyFunc, PointerType::get(IntegerType::get(8))) });
            Value *bodyFuncPtr = GetElementPtrInst::createGep(bodyFunc, {ConstantInt::get(0),ConstantInt::get(0)}, bodyInfo.recNext->getParent());
            popback_insertbefore(bodyInfo.recNext,bodyInfo.recNext->getParent());
            CallInst::createCall(parallelFor, { bodyInfo.indvar, bodyInfo.bound, bodyFuncPtr}, bodyInfo.recNext->getParent());
            popback_insertbefore(bodyInfo.recNext,bodyInfo.recNext->getParent());
            
            if(giv && bodyInfo.recUsedByOuter) {
                // const auto val = builder.makeOp<LoadInst>(givPtr);
                Value *val = LoadInst::createLoad(giv->getType(), givPtr, bodyInfo.recNext->getParent());
                popback_insertbefore(bodyInfo.recNext,bodyInfo.recNext->getParent());
                // bodyInfo.recNext->replaceWith(val);
                bodyInfo.recNext->replaceAllUseWith(val);
            }
            bodyInfo.loop->getInstructions().erase(bodyInfo.loop->findInstruction(bodyInfo.recNext), bodyInfo.loop->getInstructions().end());
            // builder.setCurrentBlock(bodyInfo.loop);
            // builder.makeOp<BranchInst>(bodyInfo.exit);
            BranchInst::createBr(bodyInfo.exit, bodyInfo.loop);
    return true;
};
Modify LoopParallel::runOnFunc(Function*func){
        if(func->isDeclaration())
            return {};
        // if(func.attr().hasAttr(FunctionAttribute::LoopBody))
        //     return false;
        auto info=info_man_->getInfo<LoopInfo>();
        if(info->getLoops(func).empty())
            return{};
        if(func->is_Parallel){
            return {};
        }
        Modify ret{};
        while(runImpl(func)) {
            ret.modify_bb = true;
            ret.modify_instr = true;
            ret.modify_call = true;

            // analysis.invalidateFunc(func);
        }
        return ret;
}