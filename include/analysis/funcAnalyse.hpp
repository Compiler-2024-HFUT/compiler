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
constexpr uint64_t WRITE_GLOBAL=(1<<0);
constexpr uint64_t LOAD_GLOBAL=(1<<1);
constexpr uint64_t WRITE_PARAM_ARRAY=(1<<2);
constexpr uint64_t LOAD_PARAM_ARRAY=(1<<3);
constexpr uint64_t GET_VAR=(1<<4);
constexpr uint64_t PUT_VAR=(1<<5);
// constexpr uint64_t WRITE_GLOBAL_ARRAY=(1<<6);
// constexpr uint64_t LOAD_GLOBAL_ARRAY=(1<<7);
struct FuncSEInfo{
    uint64_t info;

    bool isWriteGlobal(){return (info&WRITE_GLOBAL)>0;}
    bool isLoadGlobal(){return (info&LOAD_GLOBAL)>0;}
    bool isWriteParamArray(){return (info&WRITE_PARAM_ARRAY)>0;}
    bool isLoadParamArray(){return (info&LOAD_PARAM_ARRAY)>0;}
    bool isGetVar(){return (info&GET_VAR)>0;}
    bool isPutVar(){return (info&PUT_VAR)>0;}

    void addWriteGlobal(){  info=(info|WRITE_GLOBAL);}
    void addLoadGlobal(){  info=(info|LOAD_GLOBAL);}
    void addWriteParamArray(){  info=(info|WRITE_PARAM_ARRAY);}
    void addLoadParamArray(){  info=(info|LOAD_PARAM_ARRAY);}
    void addGetVar(){  info=(info|GET_VAR);}
    void addPutVar(){  info=(info|PUT_VAR);}
    __always_inline bool isNoSeFunc(){
    if(isWriteGlobal()||isWriteParamArray()||isGetVar()||isPutVar())
        return false;
    return true;
    }

    // __always_inline bool isWriteGlobalArray();
    // __always_inline bool isLoadGlobalArray();

    // __always_inline void addWriteGlobalArray();
    // __always_inline void addLoadGlobalArray();

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
