#ifndef FUNCANALYSE_HPP
#define FUNCANALYSE_HPP

#include "../midend/Function.hpp"
#include "../midend/BasicBlock.hpp"
#include "../midend/Instruction.hpp"
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "midend/Module.hpp"
#include <cstdint>
#include <map>
#include <set>
#include <sys/cdefs.h>
//side effect
struct FuncSEInfo{
    uint64_t info;
    __always_inline bool isWriteGlobal();
    __always_inline bool isLoadGlobal();
    __always_inline bool isWriteParamArray();
    __always_inline bool isLoadParamArray();
    __always_inline bool isWriteGlobalArray();
    __always_inline bool isLoadGlobalArray();
    __always_inline bool isGetVar();
    __always_inline bool isPutVar();
    __always_inline bool isPureFunc();

    __always_inline void addWriteGlobal();
    __always_inline void addLoadGlobal();
    __always_inline void addWriteParamArray();
    __always_inline void addLoadParamArray();
    __always_inline void addWriteGlobalArray();
    __always_inline void addLoadGlobalArray();
    __always_inline void addGetVar();
    __always_inline void addPutVar();

    FuncSEInfo();
    FuncSEInfo(uint64_t i);
    FuncSEInfo operator|(FuncSEInfo const& other){
        return FuncSEInfo{this->info|other.info};
    }
};
struct OneFuncInfo{
    //函数直接调用的函数
    std::set<Function*>direct_call;
    //函数所有可能调用的函数
    std::set<Function*>all_call;

};
class FuncAnalyse:public ModuleInfo{
    void clear(){call_info.clear();}
    FuncSEInfo analyseSE(Function*func);
public:
    std::map<Function*,OneFuncInfo>call_info;
    std::map<Function*,FuncSEInfo>direct_se_info;
    std::map<Function*,FuncSEInfo>all_se_info;
    void analyse()override{analyseOnModule(this->module_);}
    void analyseOnModule(Module*const module_)override;
    virtual void reAnalyse() override{analyse();}
    FuncAnalyse(Module*m,InfoManager*im):ModuleInfo(m, im){
        mod.modify_call=true;
    }
    void printInfo();
    ~FuncAnalyse(){}
};
// FuncInfo FuncAnalyse(Function *func);

#endif
