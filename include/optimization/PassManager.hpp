#ifndef PASSMANAGER_HPP
#define PASSMANAGER_HPP

#include "midend/Function.hpp"
#include "midend/Module.hpp"
#include <vector>
#include "iostream"

class Pass{
public:
    Pass(Module* m) : module_(m){
    }

    virtual void run()=0;

protected:

    Module* module_;
};
class FunctionPass:public Pass{
public:
    FunctionPass(Module* m) : Pass(m){
    }
    virtual void run()final{
        for(auto f:module_->getFunctions())
            runOnFunc(f);
    }
    virtual void runOnFunc(Function*func)=0;

};
class PassManager{
    public:
        PassManager(Module* m) : module_(m){}
        template<typename PassType> void add_pass(bool print_ir=false){
            passes_.push_back(std::pair<Pass*,bool>(new PassType(module_),print_ir));
        }
        void run(){
            for(auto pass : passes_){
                pass.first->run();
                if(pass.second){
                    std::cout<<module_->print();
                }
            }
        }


    private:
        std::vector<std::pair<Pass*,bool>> passes_;
        // std::unique_ptr<Module> m_;
        Module* module_;

};

#endif