#ifndef CREATECFGA_HPP
#define CREATECFGA_HPP

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
        CIDBB(Module *m): FunctionPass(m){}
        ~CIDBB(){};
        void runOnFunc(Function *func) override;
};


#endif