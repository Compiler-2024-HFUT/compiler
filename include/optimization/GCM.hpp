/*
    Global Code Motion
    https://roife.github.io/posts/click1995/
*/

#ifndef GCM_HPP
#define GCM_HPP

#include "analysis/Dominators.hpp"
#include "analysis/LoopInfo.hpp"
#include "optimization/PassManager.hpp"
#include <unordered_map>
#include <map>
#include <set>
using std::set;

class GCM : public ModulePass {
    umap<BasicBlock*, int> domDepth;
    umap<BasicBlock*, int> loopDepth;
    umap<Instruction*, BasicBlock*> earlyBB;
    set<Instruction*> visited;
    
    bool isPinned(Instruction *inst);
    void computeDepths(Function *func);
    // find the nearest common ancestor(LCA) of two basic blocks
    BasicBlock *findLCA(BasicBlock *bb1, BasicBlock *bb2);  
    void scheduleEarly(Instruction *inst);
    bool scheduleLate(Instruction *inst);
    bool visitFunction(Function *func);
public:
    GCM(Module *m,InfoManager *im): ModulePass(m, im){}
    ~GCM(){};
    Modify runOnModule(Module *m) override;
};

#endif