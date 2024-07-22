#ifndef CLND_HPP
#define CLND_HPP

#include "optimization/PassManager.hpp"
#include <vector>
#include <map>

using ::std::map, ::std::vector; 
//计算循环嵌套的深度
//Calculating loop nesting depth.
class CLND : public FunctionInfo{
    private:
        //访问控制
        enum state{
        unvisited,
        visiting,
        visited
    };
        //::std::map<BasicBlock*, int> depths;

        ::std::map<BasicBlock *, state> marker;

        //循环组：每个元素是一个循环，具体由一组bb组成
        ::std::vector<std::vector<BasicBlock*>*> Loops;

        //查找表：bb--->该bb所属的循环
        ::std::map<BasicBlock *, std::vector<BasicBlock *> *> bb_;

        //查找表：循环--->该循环的上一级循环（如果该循环属于嵌套循环的话）
        ::std::map<std::vector<BasicBlock *> *, std::vector<BasicBlock *> *> outer_;

        ::std::vector<std::vector<BasicBlock*>*> func_loops;
        ::std::vector<BasicBlock*> BBs;
        

        int dfn, low;
        ::std::map<BasicBlock*,int> dfn_;
        ::std::map<BasicBlock*,int> low_;



    
        Function* initialFunction(Function *func);
        void calLoopNestingDepth(BasicBlock* bb);
        //void depthInitialize(BasicBlock* bb){depths[bb] = 0;}
        //void depthUpdate(BasicBlock* bb, int increment){depths[bb]+=increment;}
        


    public:
        CLND(Module *m,InfoManager*im): FunctionInfo(m,im){}
        ~CLND(){};
        void analyseOnFunc(Function*func) override;
        //int getDepth(BasicBlock* bb){return depths[bb];}

        //循环优化需要用到的接口

        //返回函数的循环组，该组的每个元素都是一个循环
        ::std::vector<::std::vector<BasicBlock *> *>* getFuncLoops() { return &func_loops; }

        //返回该循环的入口BB
        BasicBlock* getLoopEntryBB(std::vector<BasicBlock *>* loop){return *(*loop).rbegin();}

        //返回该循环的外层循环（上一级循环）
        ::std::vector<BasicBlock *>* getOuterLoop(::std::vector<BasicBlock*>* loop){return outer_[loop];}

        //返回该BB所在的循环
        ::std::vector<BasicBlock *>* getBBLoop(BasicBlock* bb){return bb_[bb];}

};


#endif