#ifndef CREATECFGA_HPP
#define CREATECFGA_HPP

#include "analysis/InfoManager.hpp"
#include "optimization/PassManager.hpp"
#include <vector>
#include <map>

//计算CFG中各个BB的入度
//Calculate the in-degree of basic blocks
class CIDBB : public FunctionPass{
    private:
        enum state{
        unvisited,
        visiting,
        visited
    };
        std::map<BasicBlock *, state> marker;
        Function* initialFunction(Function *func);
        void calInDreegeOfBB(BasicBlock* bb);


    public:
        CIDBB(Module *m,InfoManager*im): FunctionPass(m,im){}
        ~CIDBB(){};
        void runOnFunc(Function *func) override;
};


#endif