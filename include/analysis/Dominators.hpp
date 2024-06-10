#ifndef DOMINATORS_HPP
#define DOMINATORS_HPP
#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include <map>
#include <set>
#include <unordered_set>
// struct DomTree{
//     struct Node{
//         BasicBlock* this_bb;
//         std::set<Node*> children;
//     };
//     Node * root_;
// };
class Dominators:public FunctionInfo{
private:
    ::std::list<BasicBlock *> reverse_post_order_;
    ::std::map<BasicBlock*,int> post_order_id_;

    //左：被支配者，右：直接支配者 entry的直接支配者为nullptr
    ::std::map<BasicBlock*,BasicBlock*> idom_;

    ::std::map<BasicBlock*,::std::unordered_set<BasicBlock*>> dom_set_;
    //右边是被直接支配的集合
    ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_tree_;
    //支配边界可以包含自己
    ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_frontier_;

    
    void setIDom(BasicBlock*b,BasicBlock *idom){
        idom_[b]=idom;
        // idomor_beidom[idom]=b;
        // b->setIdom(idom);
        // return idom_.find(b);
    }
    void addDomFrontier(BasicBlock* bb,BasicBlock* frontier){dom_frontier_.find(bb)->second.insert(frontier);}//bb->addDomFrontier(frontier);}
    void addDomSet(BasicBlock* bb,BasicBlock* dom){dom_set_.find(bb)->second.insert(dom);}
    // void addDomTree(BasicBlock* dominator,BasicBlock*bb ){dom_tree.find(dominator)->second.insert(bb);}
    
    void post();
    void sFastIDomAlg();
    void domAlg();
    void domTreeAlg();
    void domFrontierAlg();
public:
    Dominators(Function* fun);
    virtual ~Dominators(){}
    void clear();
    virtual void analyse() override;
    virtual void reAnalyse() override;
    void printDomFront();
    void printDomSet();
    void printDomTree();

    ::std::set<BasicBlock*> &getDomFrontier(BasicBlock* bb){return dom_frontier_.find(bb)->second;}
    ::std::unordered_set<BasicBlock*> &getDomSet(BasicBlock* bb){return dom_set_.find(bb)->second;}
    ::std::set<BasicBlock*> &getDomTree(BasicBlock* dominator){return dom_tree_.find(dominator)->second;}
    BasicBlock* getIDom(BasicBlock*b){ return idom_[b];}

};

#endif