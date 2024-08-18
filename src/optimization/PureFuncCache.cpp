// This section of code is inspired by an outstanding project from last year's competition.
// Project name: CMMC
// Project link: https://gitlab.eduxiji.net/educg-group-17291-1894922/202314325201374-1031/-/blob/riscv/
// Copyright 2023 CMMC Authors
//
// We have modified and extended the original implementation to suit our project's requirements.
// For the original license details, please refer to http://www.apache.org/licenses/LICENSE-2.0.
// All modifications are made in compliance with the terms of the Apache License, Version 2.0.

#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "midend/Type.hpp"
#include <algorithm>
#include <list>
#include <vector>
#include "optimization/PureFuncCache.hpp"

/*
gcc 13.3

        slli    a1,a1,32
        or      a2,a1,a2
        li      a5,1021
        remu    a5,a2,a5
        slli    a5,a5,4
        add     a0,a0,a5
        lw      a5,12(a0)
        beq     a5,zero,.L4
        ld      a5,0(a0)
        beq     a5,a2,.L1
        sw      zero,12(a0)
.L4:
        sd      a2,0(a0)
.L1:
        ret

*/
bool NEED_CACHE_LOOKUP=false;
Function*__getLookUpCache(Module*const module_){
    Function* lookup_cache=0;
    for(auto cache_f:module_->getFunctions()){
        if(cache_f->getName()=="xcCacheLookup"){
            lookup_cache=cache_f;
            break;
        }
    }
    if(lookup_cache==0){
        std::vector<Type*>input_params{Type::getInt32PtrType(),Type::getInt32Type(),Type::getInt32Type()};
        // std::vector<Type*>input_params{Type::getInt32Type(),Type::getInt32Type(),Type::getInt32Type()};
        auto functype=FunctionType::get(Type::getInt32PtrType(), input_params);
        lookup_cache =Function::create(functype,"xcCacheLookup",module_);
        module_->getFunctions().pop_back();
        module_->getFunctions().push_front(lookup_cache);
        // auto entry=BasicBlock::create("",lookup_cache);
        // auto arg=lookup_cache->getArgs();
        // auto iter=arg.begin();
        // auto arg0=*iter;
        // ++iter;
        // auto arg1=*iter;
        // ++iter;
        // auto arg2=*iter;
        // BinaryInst::createLsl(arg1,ConstantInt::get(32),entry);
        // BinaryInst::createOr(arg1,arg2,entry);
        // BinaryInst::createSRem(arg2,ConstantInt::get(1021),entry);
    }
    return lookup_cache;
}
BasicBlock * __get_rec_call(Function*func){
    std::vector<CallInst*>calls;
    for(auto block : func->getBasicBlocks())
        for(auto& inst : block->getInstructions()) {
            if(inst->getInstrType() == Instruction::OpID::call) {
                const auto callee = inst->getOperand(0);
                if(callee == func)
                    calls.push_back((CallInst*)inst);
            }
        }
    BasicBlock*bb=0;
    for(auto call:calls){
        if(bb==0){
            bb=call->getParent();
        }else{
            if(bb!=call->getParent())
                return 0;
        }
    }
    return bb;
}

Modify PureFuncCache::runOnFunc(Function*func)
{
    if(func->isDeclaration())
        return {};
    //递归
    auto hasRecursive = [&] {
        uint32_t count = 0;
        for(auto block : func->getBasicBlocks())
            for(auto& inst : block->getInstructions()) {
                if(inst->getInstrType() == Instruction::OpID::call) {
                    const auto callee = inst->getOperand(0);
                    if(callee == func)
                        ++count;
                    else
                        return false;
                }
            }
        return count >= 2;
    };
    if(!hasRecursive())
        return {};
    //纯函数
    if(!fa->isPureFunc(func))
        return {};
    auto const i32_type = Type::getInt32Type();
    auto const f32_type = Type::getFloatType();
    auto const ret_type = func->getReturnType();
    if(!(ret_type!=i32_type || ret_type!=f32_type))
        return {};

    std::vector<Argument*> args;
    if( func->getArgs().size()>2)
        return {};
    for(auto val : func->getArgs())
        if(val->getType()==i32_type || val->getType()==f32_type) {
            args.push_back(val);
        } else
            return {};
    if(args.empty())
        return {};

    // Function* lookup_cache=__getLookUpCache(module_);

    // auto cur_bb=func->getEntryBlock();
    uint32_t constexpr tableSize = 1021;
    const auto arrayType = ArrayType::get(i32_type, tableSize * 4);
    const auto lut = GlobalVariable::create("lut_" + std::string(func->getName()), module_,arrayType,false,ConstantZero::get(arrayType));
    // lut->setLinkage(Linkage::Internal);

    Function* lookup=__getLookUpCache(module_);


    auto rec_bb=__get_rec_call(func);
    if(rec_bb==0||rec_bb==func->getEntryBlock()||rec_bb->getSuccBasicBlocks().size()!=0||(!rec_bb->getInstructions().back()->isRet())){



    auto old_entry=func->getBasicBlocks().front();
    BasicBlock* new_entryBlock = BasicBlock::create("",func);
    func->getBasicBlocks().pop_back();
    func->getBasicBlocks().push_front(new_entryBlock);
    BasicBlock*cur_bb=old_entry;

    std::vector<Value*> argVal;
    argVal.reserve(3);
    auto lut_ptr=GetElementPtrInst::createGep(lut, {ConstantInt::get(0), ConstantInt::get( 0) },new_entryBlock);
    argVal.push_back(lut_ptr);
    for(auto arg : args) {
        if(arg->getType()==i32_type)
            argVal.push_back(arg);
        else
            argVal.push_back(CastInst::createCastInst(i32_type, arg,new_entryBlock));
    }
    while(argVal.size() < 3)
        argVal.push_back(ConstantInt::get( 0));

    const auto call_lookup_ret_ptr = CallInst::createCall(lookup, std::move(argVal),new_entryBlock);

    Value* valPtr = GetElementPtrInst::createGep(call_lookup_ret_ptr, { ConstantInt::get( 2) },new_entryBlock);

    // if(!valPtr->getType()->as<PointerType>()->isSame(ret))
    //     valPtr = builder.makeOp<PtrCastInst>(valPtr, PointerType::get(ret));

    const auto hasValPtr = GetElementPtrInst::createGep(call_lookup_ret_ptr, std::vector<Value*>{ ConstantInt::get( 3) },new_entryBlock);
    const auto hasVal = CmpInst::createCmp( CmpOp::NE,
                                                    LoadInst::createLoad(i32_type,hasValPtr,new_entryBlock), ConstantInt::get( 0),new_entryBlock);
    const auto earlyExit = BasicBlock::create("earlyexit"+func->getName(),func);
    BranchInst::createCondBr(hasVal, earlyExit, old_entry,new_entryBlock);
    Instruction* new_retval=LoadInst::createLoad(valPtr->getType()->getPointerElementType(),valPtr,earlyExit);
    if(new_retval->getType()!=func->getReturnType()){
        new_retval=CastInst::createCastInst(func->getReturnType(),new_retval,earlyExit);
    }
    ReturnInst::createRet(new_retval,earlyExit);
    for(auto block : func->getBasicBlocks()) {
        if(block == earlyExit)
            continue;
        const auto terminator = block->getTerminator();
        if(terminator->isRet()) {
            block->getInstructions().pop_back();
            const auto retVal = terminator->getOperand(0);
            StoreInst::createStore( ConstantInt::get(1),hasValPtr,block);
            StoreInst::createStore( retVal,valPtr,block);
            block->addInstruction(terminator);
        }
    }
    NEED_CACHE_LOOKUP=true;
    Modify ret{};
    ret.modify_bb=true;
    ret.modify_instr=true;
    ret.modify_call=true;
    return ret;
    }else{
        std::vector<Instruction*>new_ins_stack;
        auto new_bb=BasicBlock::create("",func);
        func->getBasicBlocks().pop_back();
        func->getBasicBlocks().insert(std::find(func->getBasicBlocks().begin(),func->getBasicBlocks().end(),rec_bb),new_bb);
        auto prebbs=rec_bb->getPreBasicBlocks();
        for(auto pre:prebbs){
            auto br=pre->getInstructions().back();
            for(int i=0;i<br->getNumOperands();++i){
                if(br->getOperand(i)==rec_bb){
                    br->replaceOperand(i,new_bb);
                    rec_bb->removePreBasicBlock(pre);
                    new_bb->addPreBasicBlock(pre);
                    pre->removeSuccBasicBlock(rec_bb);
                    pre->addSuccBasicBlock(new_bb);
                    break;
                }
            }
        }
        std::vector<Value*> argVal;
        argVal.reserve(3);
        auto lut_ptr=GetElementPtrInst::createGep(lut, {ConstantInt::get(0), ConstantInt::get( 0) },new_bb);
        argVal.push_back(lut_ptr);
        for(auto arg : args) {
            if(arg->getType()==i32_type)
                argVal.push_back(arg);
            else
                argVal.push_back(CastInst::createCastInst(i32_type, arg,new_bb));
        }
        while(argVal.size() < 3)
            argVal.push_back(ConstantInt::get( 0));

        const auto call_lookup_ret_ptr = CallInst::createCall(lookup, std::move(argVal),new_bb);

        Value* valPtr = GetElementPtrInst::createGep(call_lookup_ret_ptr, { ConstantInt::get( 2) },new_bb);
        const auto hasValPtr = GetElementPtrInst::createGep(call_lookup_ret_ptr, std::vector<Value*>{ ConstantInt::get( 3) },new_bb);
        const auto hasVal = CmpInst::createCmp( CmpOp::NE,
                                                        LoadInst::createLoad(i32_type,hasValPtr,new_bb), ConstantInt::get( 0),new_bb);
        const auto earlyExit = BasicBlock::create("earlyexit"+func->getName(),func);
        BranchInst::createCondBr(hasVal, earlyExit, rec_bb,new_bb);
        Instruction* new_retval=LoadInst::createLoad(valPtr->getType()->getPointerElementType(),valPtr,earlyExit);
        if(new_retval->getType()!=func->getReturnType()){
            new_retval=CastInst::createCastInst(func->getReturnType(),new_retval,earlyExit);
        }
        ReturnInst::createRet(new_retval,earlyExit);
        auto old_ret=rec_bb->getInstructions().back();
        rec_bb->getInstructions().pop_back();
        auto ret_val=old_ret->getOperand(0);
        StoreInst::createStore( ConstantInt::get(1),hasValPtr,rec_bb);
        StoreInst::createStore( ret_val,valPtr,rec_bb);
        rec_bb->addInstruction(old_ret);
        NEED_CACHE_LOOKUP=true;
        Modify ret{};
        ret.modify_bb=true;
        ret.modify_instr=true;
        ret.modify_call=true;
        return ret;
    }
}
