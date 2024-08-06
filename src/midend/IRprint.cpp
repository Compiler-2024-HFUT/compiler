#include "midend/IRprint.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include <array>
#include <cassert>
#include <string>
std::string printAsOp(Value *v) {
    std::string op_ir;

    if (dynamic_cast<GlobalVariable *>(v)||dynamic_cast<Function *>(v)) {
        op_ir += "@" + v->getName();
    } else if (dynamic_cast<Constant *>(v)) {
        op_ir += v->print();
    } else {
        op_ir += "%" + v->getName();
    }

    return op_ir;
}

std::string printAsOpWithType(Value *v) {
    std::string ret=v->getType()->print();
    ret += " ";
    if (dynamic_cast<GlobalVariable *>(v)||dynamic_cast<Function *>(v)) {
        ret += "@" + v->getName();
    } else if (dynamic_cast<Constant *>(v)) {
        ret += v->print();
    } else {
        ret += "%" + v->getName();
    }

    return ret;
}

std::string printCmpType(CmpOp op) {
    static std::array<std::string,6> const cmp2str={
        "eq",
        "ne",
        "sgt",
        "sge",
        "slt",
        "sle",
    };
    assert(op>=0&&op<=CmpOp::LE);
    return cmp2str[op];
}

std::string printFCmpType(CmpOp op) {
    static std::array<std::string,6> const fcmp2str={
        "ueq",
        "une",
        "ugt",
        "uge",
        "ult",
        "ule",
    };
    assert(op>=0&&op<=CmpOp::LE);
    return fcmp2str[op];
}