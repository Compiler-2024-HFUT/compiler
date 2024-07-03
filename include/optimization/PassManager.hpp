/*
 * 
 * 
 * 
 */


#ifndef PASSMANAGER_HPP
#define PASSMANAGER_HPP

#include "midend/Function.hpp"
#include "midend/Module.hpp"

#include "analysis/InfoManager.hpp"
#include "analysis/Info.hpp"

#include <vector>
#include <iostream>
#include <type_traits>

class Pass{
public:
    Pass(Module* m, InfoManager *im) : module_(m), info_man_(im) { }
    Pass(Module *m): module_(m), info_man_(nullptr) { }     // delete after

    virtual void run()=0;
protected:
    Module* const module_;
    InfoManager* const info_man_;
};

class ModulePass: public Pass {
public:
    ModulePass(Module* m, InfoManager *im) : Pass(m, im){ }

    virtual void run() final {
        runOnModule(module_);
    }
    virtual void runOnModule(Module* m)=0;
};

class FunctionPass: public Pass {
public:
    FunctionPass(Module* m, InfoManager *im) : Pass(m, im){ }
    // FunctionPass(Module* m): Pass(m) { }    // delete after

    virtual void run()final{
        for(auto f:module_->getFunctions())
            runOnFunc(f);
    }
    virtual void runOnFunc(Function*func)=0;
};

class PassManager{
    public:
        PassManager(Module* m) : module_(m){
            infoManager = new InfoManager(m);
        }
        
        template<typename PassType> 
        void addPass(bool print_ir=false){
            // assert(std::is_base_of<Pass, PassType>::value && "must be passtype")
            passes_.push_back(std::pair<Pass*,bool>(new PassType(module_, infoManager), print_ir));
        }

        template<typename InfoType>
        void addInfo() {
            // assert(std::is_base_of<Info, InfoType>::value && "must be infotype")
            infoManager->addInfo<InfoType>();
        }

        template<typename InfoType>
        InfoType *getInfo() {
            // assert(std::is_base_of<Info, InfoType>::value, "must be infotype")
            return infoManager->getInfo<InfoType>();
        }

        void run(){
            // module_->getInfoMan()->run();    info pass 应该在需要它时再进行分析，而非从一开始将其全部执行
            for(auto pass : passes_){
                pass.first->run();
                if(pass.second){
                    std::cout<<module_->print();
                }
            }
        }

    private:
        Module* module_;
        std::vector<std::pair<Pass*,bool>> passes_;
        InfoManager *infoManager;
};

#endif