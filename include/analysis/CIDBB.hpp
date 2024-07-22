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
        ::std::map<BasicBlock *, state> marker;
        ::std::map<BasicBlock*, int> in_degrees;

        Function* initialFunction(Function *func);
        void calInDreegeOfBB(BasicBlock* bb);
        void inDegreeInitialize(BasicBlock* bb){in_degrees[bb]=0;}



    public:
        CIDBB(Module *m,InfoManager*im): FunctionInfo(m,im){}
        ~CIDBB(){};
        void analyseOnFunc(Function *func) override;
        void inDegreeUpdate(BasicBlock* bb, int increment){in_degrees[bb]+=increment;}
        int getInDegree(BasicBlock* bb){return in_degrees[bb];}

};


#endif