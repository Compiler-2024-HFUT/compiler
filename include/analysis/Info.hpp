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
    Info(InfoManager *im): infoManager(im) {}
};

class ModuleInfo: public Info {      
protected:
    Module*const module_;
public:
    virtual ~ModuleInfo(){}
    ModuleInfo(Module*module, InfoManager *im): Info(im), module_(module) {}
};

class FunctionInfo: public Info {
protected:
    // set func_ = func, then analyse happened at func
    Function *func_;           
    Module *const module_;
public:
    virtual ~FunctionInfo(){}
    FunctionInfo(Module*module, InfoManager *im):Info(im), module_(module) { }
    
    // 暂时用于兼容Dominators
    FunctionInfo(Function* func):Info(nullptr), func_(func), module_(nullptr) { } 
    void setInfoManager(InfoManager *im) { infoManager = im; }
    
    virtual void analyseOnFunc()=0;
};
#endif