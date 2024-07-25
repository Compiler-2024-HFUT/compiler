#include "analysis/funcAnalyse.hpp"
#include "midend/Instruction.hpp"
constexpr uint64_t WRITE_GLOBAL=(1<<0);
constexpr uint64_t LOAD_GLOBAL=(1<<1);
constexpr uint64_t WRITE_PARAM_ARRAY=(1<<2);
constexpr uint64_t LOAD_PARAM_ARRAY=(1<<3);
constexpr uint64_t WRITE_GLOBAL_ARRAY=(1<<2);
constexpr uint64_t LOAD_GLOBAL_ARRAY=(1<<3);
constexpr uint64_t GET_VAR=(1<<4);
constexpr uint64_t PUT_VAR=(1<<5);

FuncSEInfo::FuncSEInfo():info(0){}
FuncSEInfo::FuncSEInfo(uint64_t i):info(i){}

bool FuncSEInfo::isWriteGlobal(){return info&WRITE_GLOBAL;}
bool FuncSEInfo::isLoadGlobal(){return info&LOAD_GLOBAL;}
bool FuncSEInfo::isWriteParamArray(){return info&WRITE_PARAM_ARRAY;}
bool FuncSEInfo::isLoadParamArray(){return info&LOAD_PARAM_ARRAY;}
bool FuncSEInfo::isGetVar(){return info&GET_VAR;}
bool FuncSEInfo::isPutVar(){return info&PUT_VAR;}

void FuncSEInfo::addWriteGlobal(){  info=(info|WRITE_GLOBAL);}
void FuncSEInfo::addLoadGlobal(){  info=(info|LOAD_GLOBAL);}
void FuncSEInfo::addWriteParamArray(){  info=(info|WRITE_PARAM_ARRAY);}
void FuncSEInfo::addLoadParamArray(){  info=(info|LOAD_PARAM_ARRAY);}
void FuncSEInfo::addGetVar(){  info=(info|GET_VAR);}
void FuncSEInfo::addPutVar(){  info=(info|PUT_VAR);}

void FuncAnalyse::analyseOnModule(Module*module_){
    clear();
    for(auto f:module_->getFunctions()){
        call_info.insert({f,{}});
    }
    for(auto f:module_->getFunctions()){
        for(auto [u,i]:f->getUseList()){
            if(auto call=dynamic_cast<CallInst*>(u)){
                auto callfunc=call->getFunction();
                auto &iter=this->call_info.find(callfunc)->second;
                iter.direct_call.insert(f);
                iter.all_call.insert(f);
            }
        }
    }
    bool changed=true;
    do{
        changed=false;
        for(auto &func_call_info:call_info){
            auto &curallcall=func_call_info.second.all_call;
            auto size=curallcall.size();
            auto curcall=func_call_info.second.all_call;
            for(auto called:curcall){
                if(called->isDeclaration())
                    continue;
                auto iter=call_info.find(called)->second;
                if(iter.all_call.empty())
                    continue;
                // func_call_info.second.all_call
                curallcall.insert(iter.all_call.begin(),iter.all_call.end());
            }
            if(curallcall.size()>size)
            changed=true;
        }
    }while(changed);
    for(auto f:module_->getFunctions()){
        if(f->isDeclaration())
            continue;
        direct_se_info.insert({f,analyseSE(f)});
        all_se_info.insert({f,analyseSE(f)});
    }
    changed=true;
    do{
        changed=false;
    static std::map<std::string,FuncSEInfo> const cache_func_info{
        {"getint",GET_VAR},
        {"getch",GET_VAR},
        {"getfloat",GET_VAR},
        {"getarray",WRITE_PARAM_ARRAY},
        {"getfarray",WRITE_PARAM_ARRAY},
        {"putint",PUT_VAR},
        {"putch",PUT_VAR},
        {"putfloat",PUT_VAR},
        {"putarray",WRITE_PARAM_ARRAY},
        {"putfarray",WRITE_PARAM_ARRAY},
        {"putarray",WRITE_PARAM_ARRAY},
        {"putfarray",WRITE_PARAM_ARRAY},
        {"memset_i",WRITE_PARAM_ARRAY},
        {"memset_f",WRITE_PARAM_ARRAY},
        // {"putf",LOAD_PARAM_ARRAY},
        {"_sysy_starttime",UINT64_MAX},
        {"_sysy_starttime",UINT64_MAX},
    };
        for(auto &func_call_info:call_info){
            auto &curallcall=func_call_info.second.all_call;
            FuncSEInfo &this_info=all_se_info.find(func_call_info.first)->second;
            FuncSEInfo old(this_info.info);
            for(auto f:curallcall){
                if(f->isDeclaration()){
                    this_info=this_info|cache_func_info.find(f->getName())->second;
                }else{
                    this_info=this_info|all_se_info.find(f)->second;
                }
            }
            if(old.info!=this_info.info){
                changed=true;
            }
        }
    }while(changed);
}
FuncSEInfo FuncAnalyse::analyseSE(Function*func){
    // auto func=(Function*)call->getOperand(0);
    static std::map<std::string,FuncSEInfo> const cache_func_info{
        {"getint",GET_VAR},
        {"getch",GET_VAR},
        {"getfloat",GET_VAR},
        {"getarray",WRITE_PARAM_ARRAY},
        {"getfarray",WRITE_PARAM_ARRAY},
        {"putint",PUT_VAR},
        {"putch",PUT_VAR},
        {"putfloat",PUT_VAR},
        {"putarray",WRITE_PARAM_ARRAY},
        {"putfarray",WRITE_PARAM_ARRAY},
        {"putarray",WRITE_PARAM_ARRAY},
        {"putfarray",WRITE_PARAM_ARRAY},
        {"memset_i",WRITE_PARAM_ARRAY},
        {"memset_f",WRITE_PARAM_ARRAY},
        {"putf",LOAD_PARAM_ARRAY},
        {"starttime",UINT64_MAX},
        {"stoptime",UINT64_MAX},
    };
    FuncSEInfo ret;
    auto iter=cache_func_info.find(func->getName());
    if(iter!=cache_func_info.end()){
        ret=iter->second;
        // if(ret.getInfo()==WRITE_PARAM_ARRAY){

        // }
        return ret;
    }

    if(func->getBasicBlocks().empty())
        assert(0&&"unexpect func");

    for(auto bb:func->getBasicBlocks()){
        for(auto instr:bb->getInstructions()){
            if(auto this_call=dynamic_cast<CallInst*>(instr)){
                auto call_func=static_cast<Function*>(this_call->getOperand(0));
                // if(call_func->getBasicBlocks().empty()){
                    // ret=ret.addCallInfo(FuncAnalyse(call_func));
            }else if(auto store=dynamic_cast<StoreInst*>(instr)){
                auto lval=store->getLVal();
                if(dynamic_cast<GlobalVariable*>(lval)!=nullptr){
                    ret.addWriteGlobal();
                }else if(dynamic_cast<AllocaInst*>(lval)!=nullptr){

                }
            }else if(auto load =dynamic_cast<LoadInst*>(instr)){
                auto lval=load->getLVal();
                if(dynamic_cast<GlobalVariable*>(lval)!=nullptr){
                    ret.addLoadGlobal();
                }else if(dynamic_cast<AllocaInst*>(lval)!=nullptr){

                }
            }
        }
    }
    return ret;
}
