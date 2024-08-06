#ifndef IRPRINT_HPP
#define IRPRINT_HPP

#include "Value.hpp"
#include "Instruction.hpp"

std::string printAsOp(Value *v);
std::string printAsOpWithType(Value *v);
std::string printCmpType(CmpOp op);
std::string printFCmpType(CmpOp op);

#endif