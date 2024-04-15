#ifndef DOMINATORS_HPP
#define DOMINATORS_HPP
#include "dom.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include <map>
#include <set>

class Dominators{
private:
    Function *const fun_;
    ::std::list<BasicBlock *> reverse_post_order_;
    ::std::map<BasicBlock*,int> post_order_id_;

    ::std::map<BasicBlock*,BasicBlock*> idom_;

    ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_set_;
    ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_frontier_;

    // DominatorSet dom_set_;
    // DominanceFrontier dom_frontier_;
    
    decltype(idom_.begin()) setIDom(BasicBlock*b,BasicBlock *idom){
        idom_[b]=idom;
        b->setIdom(idom);
        return idom_.find(b);
    }
    void addDomFrontier(BasicBlock* bb,BasicBlock* frontier){dom_frontier_.find(bb)->second.insert(frontier);}
    void addDomSet(BasicBlock* bb,BasicBlock* dom){dom_set_.find(bb)->second.insert(dom);}
    BasicBlock* intersect(BasicBlock*,BasicBlock*);
    void post();
    void sFastIDomAlg();
    void domAlg();
    void domFrontierAlg();
public:
    Dominators(Function* fun);
    void run();

    inline ::std::set<BasicBlock*> &getDomFrontier(BasicBlock* bb){return dom_frontier_.find(bb)->second;}
    inline ::std::set<BasicBlock*> &getDomSet(BasicBlock* bb){return dom_set_.find(bb)->second;}
    inline BasicBlock* getIDom(BasicBlock*b){ return idom_[b];}

};

#endif