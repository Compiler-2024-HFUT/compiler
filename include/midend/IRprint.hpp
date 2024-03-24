#ifndef IPPRINT_HPP
#define IRPRINT_HPP

#include "Value.hpp"
#include "Module.hpp"
#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "Constant.hpp"
#include "BasicBlock.hpp"
#include "Instruction.hpp"
#include "User.hpp"
#include "Type.hpp"

std::string printAsOp(Value *v, bool print_ty);
std::string printCmpType(CmpOp op);
std::string printFCmpType(CmpOp op);

#endif