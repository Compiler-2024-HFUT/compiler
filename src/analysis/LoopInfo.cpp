#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/Dominators.hpp"

#include <unordered_map>
#include <algorithm>
using std::sort;
using std::includes;
template<typename key, typename value>
using umap = std::unordered_map<key, value>;

void LoopInfo::analyseOnFunc(Function *func_) {
    DFS_CFG(func_->getEntryBlock(), 0);
    findRetreatEdges(func_);
    findBackEdges();
    findLoops(func_);
    combineLoops(func_);
    producedNestLoops(func_);
}

// 深度优先遍历CFG，并标注层次，EntryBlock的level为0
void LoopInfo::DFS_CFG( BB* rootBB, int level ) {
    static umap<BB*, bool> isVisited;
    if(level == 0) {
        isVisited.clear();
        for(BB *bb : rootBB->getParent()->getBasicBlocks()) {
            isVisited[bb] = false;
        }
    }

    BBlevel[rootBB] = level;
    isVisited[rootBB] = true;

    for(BB* bb : rootBB->getSuccBasicBlocks()) {
        if(!isVisited[bb]) {
            DFS_CFG(bb, level+1);
        }
    }
}

// **反向**深度优先遍历CFG，从tail开始，到head或entry结束
// 遍历过程中遇到的BB既是Loop的BB
// 遍历过程中，使用的参数head、Loop不变，但参数tail逐渐靠近head
void LoopInfo::DFS_CFG( BB* tail, BB* head, Loop *loop, Function *func_ ) {
    static umap<BB*, bool> isVisited;
    // 第一个exit必然是
    if(loop->getLatchs()[0] == tail) {
        isVisited.clear();
        for(BB *bb : head->getParent()->getBasicBlocks()) {
            isVisited[bb] = false;
        }
    }

    isVisited[tail] = true;
    loop->addBlock(tail);

    for(BB* pre : tail->getPreBasicBlocks()) {
        // pre未被访问，并且pre不是head或entry
        if(!isVisited[pre] && pre != head && pre != func_->getEntryBlock() ) {
            DFS_CFG(pre, head, loop, func_);
        }
    }
}

// 如果某个BBtail的level小于其后继节点BBhead，那么（BBtail，BBhead）构成一条RetreatEdge
// 特殊地，如果tail == head，它们也组成一条RetreatEdge
void LoopInfo::findRetreatEdges(Function *func_) {
    retreatEdges.clear();
    for(BB* tail : func_->getBasicBlocks()) {
        for(BB* head : tail->getSuccBasicBlocks()) {
            if(BBlevel[tail] > BBlevel[head] || tail == head) {
                retreatEdges.push_back( {tail, head} );
            }
        }
    }
}

// Back edge: t->h (RetreatEdge) and h dominates t
void LoopInfo::findBackEdges() {
    backEdges.clear();
    Dominators *dom = infoManager->getInfo<Dominators>();

    for(pair<BB*, BB*> edge : retreatEdges) {
        auto &domSet = dom->getDomSet(edge.second);
        if( domSet.find(edge.first) != domSet.end() ) {
            backEdges.push_back(edge);
        }
    }
}

void LoopInfo::findLoops(Function *func_) {
    for(auto be : backEdges) {
        Loop *tmp = new Loop(be.first, be.second);
        DFS_CFG( be.first, be.second, tmp, func_ );
        tmp->addBlock(be.second);
        loops[func_].push_back(tmp);
    }
}

// 同header的loop合并
void LoopInfo::combineLoops(Function *func_) {
    // loops按照header排序，使得同样header的loop相邻
    sort(loops[func_].begin(), loops[func_].begin(), [](Loop *a, Loop* b){ return a->getHeader() < b->getHeader(); });

    auto iter1 = loops[func_].begin(), iter2 = loops[func_].begin();
    while(iter1 != loops[func_].end()) { 
        iter2++;
        // 如果iter1是最后一个元素，iter2为end
        while(iter1 != loops[func_].end() && iter2 != loops[func_].end() && (*iter1)->getHeader() == (*iter2)->getHeader()) {
            // 合并同header的2个loop
            (*iter1)->addLatchs( (*iter2)->getLatchs()[0] );
            for(auto bb : (*iter2)->getBlocks()) {
                (*iter1)->addBlock(bb);
            }
            // 下个元素
            iter2 = loops[func_].erase(iter2);
        }
        // 下一个header或end结束
        iter1 = iter2;
    }
}

void LoopInfo::producedNestLoops(Function *func_) {
    bool erased = false;

    // loops按照包含的基本块数量排序，少的在前
    sort(loops[func_].begin(), loops[func_].end(), [](Loop *a, Loop *b) { return (a->getBlocks().size() < b->getBlocks().size()); });

    // 判断a为b的真子集
    auto isSubset = [](const uset<BB*> &a, const uset<BB*> &b) {
        for (const auto& element : a) {
            if (b.find(element) == b.end()) {
                return false;
            }
        }
        return a.size() < b.size();
    };

    auto iter1 = loops[func_].begin(), iter2 = loops[func_].begin();
    while(iter1 != loops[func_].end()) {
        iter2 = iter1 + 1;
        while(iter2 != loops[func_].end()) {
            // loop1 header 属于 loop2 且 loop1的blocks属于loop2的blocks
            if( (*iter2)->getBlocks().count( (*iter1)->getHeader() ) && isSubset( (*iter1)->getBlocks(), (*iter2)->getBlocks() ) ) {

                (*iter2)->addInner( (*iter1) );
                (*iter1)->setOuter( (*iter2) );
                (*iter1)->setDepth( (*iter2)->getDepth() + 1 );
                
                // 标记移除已经被识别为子循环的iter1
                erased = true;
                break;
            }
            ++iter2;
        }

        // iter1是子循环，移除，iter1是下一个；不是，iter1移动到下一个
        if(erased && iter2 != loops[func_].end()) {
            iter1 = loops[func_].erase(iter1);
        } else {
            ++iter1;
        }
    }
}

string LoopInfo::print() {
    string loopinfo = "";
    module_->print();
    for(Function *f : module_->getFunctions()) {
        if(f->getBasicBlocks().size() == 0 || loops[f].size() == 0)
            continue;
        
        if(f->getBasicBlocks().size() != 0) {
            loopinfo += STRING_RED("Loops of " + f->getName() + " with LoopSize " + STRING_NUM(loops[f].size()) +":\n");
            for(auto loop : loops[f]) {
                loopinfo += STRING(loop->print() + "\n");
            }
        }
    }
    return loopinfo;
}