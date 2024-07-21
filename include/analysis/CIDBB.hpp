#ifndef CREATECFGA_HPP
#define CREATECFGA_HPP

#include "analysis/InfoManager.hpp"
#include "optimization/PassManager.hpp"
#include <vector>
#include <map>

//计算CFG中各个BB的入度
//Calculate the in-degree of basic blocks
class CIDBB : public FunctionInfo{
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
        CIDBB(Module *m,InfoManager*im): FunctionInfo(m,im){}
        ~CIDBB(){};
        void analyseOnFunc(Function *func) override;

};


#endif