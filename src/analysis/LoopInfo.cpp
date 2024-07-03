#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/Dominators.hpp"

#include <unordered_map>
template<typename key, typename value>
using umap = std::unordered_map<key, value>;

void LoopInfo::analyseOnFunc() {
    DFS_CFG(func_->getEntryBlock(), 0);
    findRetreatEdges();
    findBackEdges();
    findLoops();
}

void LoopInfo::analyse() {
    for (Function* f : module_->getFunctions()) {
        func_ = f;
        analyseOnFunc();
    }
    invalid = false;    // validate
}

void LoopInfo::reAnalyse() {
    analyse();
}

// 深度优先遍历CFG，并标注层次，EntryBlock的level为0
void LoopInfo::DFS_CFG( BB* rootBB, int level ) {
    static umap<BB*, bool> isVisited;
    if(level == 0) {
        isVisited.clear();
    }

    BBlevel.insert( {rootBB, level} );
    BBlevel.insert( {rootBB, true} );

    for(BB* bb : rootBB->getSuccBasicBlocks()) {
        if(!isVisited[bb]) {
            DFS_CFG(bb, level+1);
        }
    }
}

// **反向**深度优先遍历CFG，从tail开始，到head或entry结束
// 遍历过程中遇到的BB既是Loop的BB
// 遍历过程中，使用的参数head、Loop不变，但参数tail逐渐靠近head
void LoopInfo::DFS_CFG( BB* tail, BB* head, Loop *loop ) {
    static umap<BB*, bool> isVisited;
    if(loop->getLatch() == tail) {
        isVisited.clear();
    }

    BBlevel.insert( {tail, true} );
    loop->addBlock(tail);

    for(BB* pre : tail->getPreBasicBlocks()) {
        // pre未被访问，并且pre不是head或entry
        if(!isVisited[pre] && pre != head && pre != func_->getEntryBlock() ) {
            DFS_CFG(pre, head, loop);
        }
    }
}

// 如果某个BBtail的level小于其后继节点BBhead，那么（BBtail，BBhead）构成一条RetreatEdge
void LoopInfo::findRetreatEdges() {
    retreatEdges.clear();
    for(BB* tail : func_->getBasicBlocks()) {
        for(BB* head : tail->getSuccBasicBlocks()) {
            if(BBlevel[tail] > BBlevel[head]) {
                retreatEdges.push_back( {tail, head} );
            }
        }
    }
}

// Back edge: t->h (RetreatEdge) and h dominates t
void LoopInfo::findBackEdges() {
    backEdges.clear();
    Dominators *dom = infoManager->getInfo<Dominators>();
    if(dom == nullptr) {
        // here
        exit(-1);
    }
    

    for(pair<BB*, BB*> edge : retreatEdges) {
        auto &domSet = dom->getDomSet(edge.second);
        if( domSet.find(edge.first) != domSet.end() ) {
            backEdges.push_back(edge);
        }
    }
}

void LoopInfo::findLoops() {
    for(auto be : backEdges) {
        Loop *tmp = new Loop(be.first, be.second);
        DFS_CFG( be.first, be.second, tmp );
        tmp->addBlock(be.second);
        loops[func_].push_back(tmp);
    }
}