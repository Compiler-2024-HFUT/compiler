#ifndef LOOP_PARALLER_UTIL_HPP
#define LOOP_PARALLER_UTIL_HPP

#include "midend/Function.hpp"
#include "midend/Type.hpp"
#include "midend/Module.hpp"

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
#endif