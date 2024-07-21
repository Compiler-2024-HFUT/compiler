#include "midend/IRprint.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include <array>
#include <cassert>
#include <string>
std::string printAsOp(Value *v, bool print_ty) {
    std::string op_ir;
    if (print_ty) {
        op_ir += v->getType()->print();
        op_ir += " ";
    }

    if (dynamic_cast<GlobalVariable *>(v)) {
        op_ir += "@" + v->getName();
    } else if (dynamic_cast<Function *>(v)) {
        op_ir += "@" + v->getName();
    } else if (dynamic_cast<Constant *>(v)) {
        op_ir += v->print();
    } else {
        op_ir += "%" + v->getName();
    }

    return op_ir;
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
    // const auto iter=cmp2str.find(op);
    // assert(iter!=cmp2str.end()&&"wrong cmp op");
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
    // const auto iter=fcmp2str.find(op);
    // assert(iter!=fcmp2str.end()&&"wrong fcmp op");
    return fcmp2str[op];
}