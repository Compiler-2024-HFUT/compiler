#include "analysis/funcAnalyse.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include <bitset>
#include <vector>

FuncSEInfo::FuncSEInfo():info(0){}
FuncSEInfo::FuncSEInfo(uint64_t i):info(i){}



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
        auto se=analyseSE(f);
        direct_se_info.insert({f,se});
        all_se_info.insert({f,se});
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
        {"putarray",LOAD_PARAM_ARRAY},
        {"putfarray",LOAD_PARAM_ARRAY},
        // {"putarray",WRITE_PARAM_ARRAY},
        // {"putfarray",WRITE_PARAM_ARRAY},
        {"memset_i",WRITE_PARAM_ARRAY},
        {"memset_f",WRITE_PARAM_ARRAY},
        // {"putf",LOAD_PARAM_ARRAY},
        {"_sysy_starttime",UINT64_MAX},
        {"_sysy_stoptime",UINT64_MAX},
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
    // printInfo();
}
FuncSEInfo FuncAnalyse::analyseSE(Function*func){
    // auto func=(Function*)call->getOperand(0);
    static std::map<std::string,FuncSEInfo> const cache_func_info{
        {"getint",GET_VAR},
        {"getch",GET_VAR},
        {"getfloat",GET_VAR},
        {"getarray",WRITE_PARAM_ARRAY|GET_VAR},
        {"getfarray",WRITE_PARAM_ARRAY|GET_VAR},
        {"putint",PUT_VAR},
        {"putch",PUT_VAR},
        {"putfloat",PUT_VAR},
        {"putarray",LOAD_PARAM_ARRAY|PUT_VAR},
        {"putfarray",LOAD_PARAM_ARRAY|PUT_VAR},
        // {"putarray",WRITE_PARAM_ARRAY},
        // {"putfarray",WRITE_PARAM_ARRAY},
        {"memset_i",WRITE_PARAM_ARRAY},
        {"memset_f",WRITE_PARAM_ARRAY},
        {"putf",LOAD_PARAM_ARRAY},
        {"_sysy_starttime",UINT64_MAX},
        {"_sysy_stoptime",UINT64_MAX},
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
                if(call_func->getBasicBlocks().empty())
                    ret=ret|(analyseSE(call_func));
            }else if(auto store=dynamic_cast<StoreInst*>(instr)){
                auto lval=store->getLVal();
                if(dynamic_cast<GlobalVariable*>(lval)!=nullptr){
                    ret.addWriteGlobal();
                }else if(auto gep=dynamic_cast<GetElementPtrInst*>(lval)){
                    std::vector<GetElementPtrInst*>vals{gep};
                    while(!vals.empty()){
                        gep=vals.back();
                        vals.pop_back();
                        if(dynamic_cast<Argument*>(gep->getOperand(0))){
                            ret.addWriteParamArray();
                        }else  if(dynamic_cast<GlobalVariable*>(gep->getOperand(0))){
                            ret.addWriteGlobal();
                        }else if(auto recur_gep=dynamic_cast<GetElementPtrInst*>(gep->getOperand(0))){
                            vals.push_back(recur_gep);
                        }
                    }
                }
            }else if(auto load =dynamic_cast<LoadInst*>(instr)){
                auto lval=load->getLVal();
                if(dynamic_cast<GlobalVariable*>(lval)!=nullptr){
                    ret.addLoadGlobal();
                }else if(auto gep=dynamic_cast<GetElementPtrInst*>(lval)){
                    std::vector<GetElementPtrInst*>vals{gep};
                    while(!vals.empty()){
                        gep=vals.back();
                        vals.pop_back();
                        if(dynamic_cast<Argument*>(gep->getOperand(0))){
                            ret.addLoadParamArray();
                        }else  if(dynamic_cast<GlobalVariable*>(gep->getOperand(0))){
                            ret.addLoadGlobal();
                        }else if(auto recur_gep=dynamic_cast<GetElementPtrInst*>(gep->getOperand(0))){
                            vals.push_back(recur_gep);
                        }
                    }
                }
            }
        }
    }
    return ret;
}
void FuncAnalyse::printInfo(){
    using std::cout,std::endl;
    for(auto f_call:this->call_info){
        if(f_call.first->isDeclaration())
            continue;
        cout<<f_call.first->getName()<<"   dicet call: "<<endl;
        for(auto dircall:f_call.second.direct_call){
        cout<<"\t"<<dircall->getName()<<endl;
        }
        cout<<f_call.first->getName()<<"   all call: "<<endl;
        for(auto all_call:f_call.second.all_call){
        cout<<"\t"<<all_call->getName()<<endl;
        }
    }
    for(auto f:direct_se_info){
        cout<<f.first->getName()<<"   dicet se: ";
        cout<<std::bitset<sizeof(f.second.info)*8>(f.second.info)<<endl;
    }
    for(auto f:all_se_info){
        cout<<f.first->getName()<<"   all se: ";
        cout<< std::bitset<sizeof(f.second.info)*8>(f.second.info)<<endl;
    }

}