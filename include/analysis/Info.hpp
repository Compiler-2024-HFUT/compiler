#include "midend/Function.hpp"
struct ModuleInfo{

};

struct FunctionInfo{
    Function *const func_;
    FunctionInfo(Function*func):func_(func){}
};