#include "../../include/midend/IRprint.hpp"

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
    switch (op) {
    case GE: return "sge"; break;
    case GT: return "sgt"; break;
    case LE: return "sle"; break;
    case LT: return "slt"; break;
    case EQ: return "eq"; break;
    case NE: return "ne"; break;
    default: break;
    }
    return "wrong cmp op";
}

std::string printFCmpType(CmpOp op) {
    switch (op) {
    case GE: return "uge"; break;
    case GT: return "ugt"; break;
    case LE: return "ule"; break;
    case LT: return "ult"; break;
    case EQ: return "ueq"; break;
    case NE: return "une"; break;
    default: break;
    }
    return "wrong fcmp op";
}