#ifndef INFO_MAN_HPP
#define INFO_MAN_HPP
#include "midend/Function.hpp"
#include "Dominators.hpp"
// #include "optimization/PassManager.hpp"
#include <map>
#include <memory>

class InfoManager{
    Module*module_;
    std::map<Function*,std::unique_ptr<Dominators>>func_doms_;
public:
    // template<typename T>
    // T* getFuncInfo(Function*f){
    //     return nullptr;
    // }
    Dominators* getFuncDom(Function*f);
    void run();

    explicit InfoManager(Module*m):module_(m){}
};
#endif