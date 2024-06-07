#include "midend/Function.hpp"
class Module;
struct ModuleInfo{
    Module*const module_;
    ModuleInfo(Module*_module):module_(_module){}
    virtual void analyse();
    virtual void reAnalyse();

};

struct FunctionInfo{
    Function *const func_;
    FunctionInfo(Function*func):func_(func){}
    virtual void analyse();
    virtual void reAnalyse();
};