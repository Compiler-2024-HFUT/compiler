#ifndef INFO_HPP
#define INFO_HPP

class InfoManager;

class Function;
class Module;

#include <string>
using std::string;

class Info {
protected:
    bool invalid = true;                    // 如果当前info无效，从其获取数据前需要reanalyze，分析完成设为true
    InfoManager *infoManager;
public:
    bool isInvalid() { return invalid; }
    void invalidate() { invalid = true; }
    virtual void analyse()=0;               // run on module
    virtual void reAnalyse()=0;
    virtual string print() { return ""; }   // debug
    virtual ~Info(){}
    Info(){}
};

class ModuleInfo: public Info {      
protected:
    Module*const module_;
public:
    virtual ~ModuleInfo(){}
    ModuleInfo(Module*module): Info(), module_(module) {}
};

class FunctionInfo: public Info {
protected:
    Function *const func_;
public:
    virtual ~FunctionInfo(){}
    FunctionInfo(Function*func):Info(), func_(func) {}
    // virtual void analyseOnFunc(Function *func)=0; 
};
#endif