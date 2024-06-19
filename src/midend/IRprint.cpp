#include "midend/IRprint.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
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
    static std::map<CmpOp,std::string> cmp2str={
        {CmpOp::GE, "sge"},
        {CmpOp::GT, "sgt"},
        {CmpOp::LE, "sle"},
        {CmpOp::LT, "slt"},
        {CmpOp::EQ, "eq"},
        {CmpOp::NE, "ne"},
    };
    const auto iter=cmp2str.find(op);
    assert(iter!=cmp2str.end()&&"wrong cmp op");
    return iter->second;
}

std::string printFCmpType(CmpOp op) {
    static std::map<CmpOp,std::string> fcmp2str={
        {CmpOp::GE,"uge"},
        {CmpOp::GT,"ugt"},
        {CmpOp::LE,"ule"},
        {CmpOp::LT,"ult"},
        {CmpOp::EQ,"ueq"},
        {CmpOp::NE,"une"},
    };
    const auto iter=fcmp2str.find(op);
    assert(iter!=fcmp2str.end()&&"wrong fcmp op");
    return iter->second;
}