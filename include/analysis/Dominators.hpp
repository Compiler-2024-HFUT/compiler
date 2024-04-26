#ifndef DOMINATORS_HPP
#define DOMINATORS_HPP
#include "midend/BasicBlock.hpp"
#include <map>
#include <set>

class Dominators{
private:
    Function *const fun_;
    ::std::list<BasicBlock *> reverse_post_order_;
    ::std::map<BasicBlock*,int> post_order_id_;

    ::std::map<BasicBlock*,BasicBlock*> idom_;

    ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_set_;
    ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_tree;
    ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_frontier_;

    
    void setIDom(BasicBlock*b,BasicBlock *idom){
        idom_[b]=idom;
        // idomor_beidom[idom]=b;
        b->setIdom(idom);
        // return idom_.find(b);
    }
    void addDomFrontier(BasicBlock* bb,BasicBlock* frontier){dom_frontier_.find(bb)->second.insert(frontier);bb->addDomFrontier(frontier);}
    void addDomSet(BasicBlock* bb,BasicBlock* dom){dom_set_.find(bb)->second.insert(dom);}
    void addDomTree(BasicBlock* dominator,BasicBlock*bb ){dom_tree.find(dominator)->second.insert(bb);}
    
    void post();
    void sFastIDomAlg();
    void domAlg();
    void domTreeAlg();
    void domFrontierAlg();
public:
    Dominators(Function* fun);
    void run();
    void printDomFront();
    void printDomSet();

    inline ::std::set<BasicBlock*> &getDomFrontier(BasicBlock* bb){return dom_frontier_.find(bb)->second;}
    inline ::std::set<BasicBlock*> &getDomSet(BasicBlock* bb){return dom_set_.find(bb)->second;}
    inline ::std::set<BasicBlock*> &getDomTree(BasicBlock* dominator){return dom_set_.find(dominator)->second;}
    inline BasicBlock* getIDom(BasicBlock*b){ return idom_[b];}

};

#endif