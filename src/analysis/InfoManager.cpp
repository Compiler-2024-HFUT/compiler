#include "midend/Module.hpp"
#include "analysis/InfoManager.hpp"
#include "analysis/Dominators.hpp"
void InfoManager::run(){
    for(auto f:module_->getFunctions()){
        if(!f->getBasicBlocks().empty())
            func_doms_.insert({f,std::make_unique<Dominators>(f)});
    }
}
Dominators* InfoManager::getFuncDom(Function*f){
    auto ret=func_doms_.find(f);
    if(ret!=func_doms_.end())
        return ret->second.get();
    return nullptr;
}