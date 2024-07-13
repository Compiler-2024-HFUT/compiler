/*
    没有查找exits，loop的exits都为空

 */


#ifndef LOOPINFO_HPP
#define LOOPINFO_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "analysis/Info.hpp"
#include "analysis/InfoManager.hpp"
#include "utils/Logger.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
using std::cout;
using std::endl;
using std::string;
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
    vector<BB*> latchs;                 // 有一条边连接到header, latch->header形成回边backedge
    vector<BB*> exits;                  // 执行完循环或break后到达的基本块   

    uset<BB*> blocks;                 // 组成natural loop的集合
    Loop *outer;                        // 外层循环
    vector<Loop*> inners;               // 内循环
    int depth;                      // 循环深度
public:
    Loop(BB *tail, BB *head): latchs({tail}), header(head), depth(1) {}
    
    BB* const &getHeader() { return header; }
    int getDepth() { return depth; }
    Loop *getOuter() { return outer; }
    void setDepth(int d) { depth = d; }
    void setOuter(Loop *l) { outer = l; }

    // 自身及子循环深度+1
    void updateDepth() {
        depth += 1;
        for(Loop *l : inners) {
            l->updateDepth();
        }
    }

    vector<BB*> const &getExits()   { return exits; }
    vector<BB*> const &getLatchs()  { return latchs; }
    uset<BB*> const &getBlocks()    { return blocks; }
    vector<Loop*> const &getInners()  { return inners; }
 
    void addExits(BB* bb) { exits.push_back(bb); }
    void addLatchs(BB* bb) { latchs.push_back(bb); }
    void addBlock(BB* bb) { blocks.insert(bb); }
    void addInner(Loop *l) { inners.push_back(l); }

    string print() {
        string loop = "";
        loop += STRING_YELLOW("header: ") + header->getName() + '\n';
        loop += STRING_YELLOW("latchs") + ":\n";
        for(auto bb : latchs) {
            loop += '\t' + bb->getName() + '\n';
        }
        loop += STRING_YELLOW("blocks") + ":\n";
        for(auto bb : blocks) {
            loop += '\t' + bb->getName() + '\n';
        }
        /*
        loop += "exits: " + '\n';
        for(auto bb : exits) {
            loop += '\t' + bb->getName() + '\n';
        }
        */

        if(inners.size() != 0) {
            loop += STRING_YELLOW("inners") + ":\n";
            for(Loop *l : inners) {
                loop += STRING("outer's header: ") + STRING_RED(header->getName()) + STRING(" inner's depth: ") + STRING_NUM(l->getDepth());
                loop += "\n" + l->print() + "\n";
            }
        }
        
        return loop;
    }
};

class LoopInfo: public FunctionInfo {
    umap<Function*, vector<Loop*> > loops;
    umap<BB*, int> BBlevel;    
    vector< pair<BB*, BB*> > retreatEdges;       // 由低层次指向高层次的边
    vector< pair<BB*, BB*> > backEdges;

    void findRetreatEdges(Function *func_);   
    void findBackEdges();
    void findLoops(Function *func_);
    void combineLoops(Function *func_);         // 对于存在continue的循环，存在多条backedge回到header，该方法主要是将具有相同header的Loop合并
    void producedNestLoops(Function *func_);    // 处理嵌套循环，计算loop的inner
    void DFS_CFG( BB* rootBB, int level ); 
    void DFS_CFG( BB* tail, BB* head, Loop *loop, Function *func_);
public:
    LoopInfo(Module *m, InfoManager *im): FunctionInfo(m, im) { }
    virtual ~LoopInfo() { }

    virtual void analyseOnFunc(Function *func) override;

    virtual string print() override;

    vector<Loop*> getLoops(Function* func) { 
        if(isInvalid()) analyse();
        return loops[func]; 
    }

};

#endif