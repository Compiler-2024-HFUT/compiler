#ifndef INFO_HPP
#define INFO_HPP
class Function;
class Module;
struct ModuleInfo{
    Module*const module_;
    ModuleInfo(Module*_module):module_(_module){}
    virtual void analyse()=0;
    virtual void reAnalyse()=0;
    virtual ~ModuleInfo(){}

};

struct FunctionInfo{
    Function *const func_;
    FunctionInfo(Function*func):func_(func){}
public:
    virtual void analyse()=0;
    virtual void reAnalyse()=0;
    virtual ~FunctionInfo(){}
};
#endif