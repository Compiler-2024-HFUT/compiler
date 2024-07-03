#ifndef LOOPINFO_HPP
#define LOOPINFO_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"

#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
using std::vector;
using std::map;
using std::pair;
template<typename key, typename value>
using umap = std::unordered_map<key, value>;
template<typename value>
using uset = std::unordered_set<value>;

using BB = BasicBlock;

class Loop {
    BB *header;  
    BB *latch;                          // 有一条边连接到header, latch->header形成回边backedge
    
    vector<BB*> blocks;                 // 组成natural loop的集合
    Loop *outer;                        // 外层循环
    vector<Loop*> inner;                // 内循环
public:
    Loop(BB *tail, BB *head): latch(tail), header(head) {}
    BB* getHeader() { return header; }
    BB* getLatch()  { return latch;  }
    void addBlock(BB* bb) { blocks.push_back(bb); }
};

class LoopInfo: public FunctionInfo {
    umap<Function*, vector<Loop*> > loops;
    umap<BB*, int> BBlevel;    
    vector< pair<BB*, BB*> > retreatEdges;       // 由低层次指向高层次的边
    vector< pair<BB*, BB*> > backEdges;

    void findRetreatEdges();   
    void findBackEdges();
    void findLoops();
    void DFS_CFG( BB* rootBB, int level ); 
    void DFS_CFG( BB* tail, BB* head, Loop *loop);
public:
    LoopInfo(Module *m, InfoManager *im): FunctionInfo(m, im) { }
    virtual ~LoopInfo() { }

    virtual void analyse() override;
    virtual void reAnalyse() override;
    virtual void analyseOnFunc() override;

    vector<Loop*> getLoops(Function* func) { return loops[func]; }

};

#endif