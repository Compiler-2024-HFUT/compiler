#ifndef INFO_HPP
#define INFO_HPP

#include <string>
using std::string;

class Function;
class Module;
class ModuleInfo{
protected:
    Module*const module_;
    bool invalid = true;         // 如果当前info无效，从其获取数据前需要reanalyze，并设为true
public:
    bool isInvalid() { return invalid; }
    void invalidate() { invalid = true; }
    virtual void analyse()=0;
    virtual void reAnalyse()=0;
    virtual string print() { return ""; }   // debug
    virtual ~ModuleInfo(){}
    ModuleInfo(Module*_module):module_(_module){}
};

class FunctionInfo{
protected:
    Function *const func_;
    bool invalid = true;
public:
    bool isInvalid() { return invalid; }
    void invalidate() { invalid = true; }
    virtual void analyse()=0;
    virtual void reAnalyse()=0;
    virtual string print() { return ""; }
    virtual ~FunctionInfo(){}
    FunctionInfo(Function*func):func_(func){}
};
#endif