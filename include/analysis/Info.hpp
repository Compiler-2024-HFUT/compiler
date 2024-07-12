#ifndef INFO_HPP
#define INFO_HPP

class InfoManager;

class Function;
class Module;

#include <string>
using std::string;

class Info {
protected:
    bool invalid = true;                    // 如果当前info无效，从其获取数据前需要reanalyze，分析完成设为false
    InfoManager *infoManager;
public:
    bool isInvalid() { return invalid; }
    void invalidate() { invalid = true; }
    
    // analyse(std::Function f) { f(); invalid = false; }   // 在后面添加一些操作，使得写分析的时候可以暂时忽略一些细节
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
    virtual void analyse() override;
    virtual void reAnalyse() override;

    virtual void analyseOnModule(Module *m)=0;
};

class FunctionInfo: public Info {
protected:               
    Module *const module_;
public:
    virtual ~FunctionInfo(){}
    FunctionInfo(Module*module, InfoManager *im):Info(im), module_(module) { }
    virtual void analyse() override;
    virtual void reAnalyse() override;

    virtual void analyseOnFunc(Function *func)=0;
};

#endif