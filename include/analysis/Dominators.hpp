/*
reference:https://www.cs.tufts.edu/comp/150FP/archive/keith-cooper/dom14.pdf
*/
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
    ::std::vector<BasicBlock *> reverse_post_order_;
    ::std::map<BasicBlock*,int> post_order_id_;

    // ::std::map<BasicBlock*,::std::unordered_set<BasicBlock*>> dom_set_;
    //右边是被直接支配的集合
    // ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_tree_;
    //支配边界可以包含自己
    // ::std::map<BasicBlock*,::std::set<BasicBlock*>> dom_frontier_;


    //左：被支配者，右：直接支配者 entry的直接支配者为nullptr
    ::std::map<BasicBlock*,BasicBlock*> idom_;
    ::std::map<Function*, ::std::map<BasicBlock*,::std::unordered_set<BasicBlock*> > > func_dom_set_;
    ::std::map<Function*, ::std::map<BasicBlock*,::std::set<BasicBlock*> > > func_dom_tree_;
    ::std::map<Function*, ::std::map<BasicBlock*,::std::set<BasicBlock*> > > func_dom_frontier_;

    void setIDom(BasicBlock*b,BasicBlock *idom){
        idom_[b]=idom;
        // idomor_beidom[idom]=b;
        // b->setIdom(idom);
        // return idom_.find(b);
    }
    void addDomFrontier(BasicBlock* bb,BasicBlock* frontier){
        func_dom_frontier_[bb->getParent()].find(bb)->second.insert(frontier);
    }//bb->addDomFrontier(frontier);}
    void addDomSet(BasicBlock* bb,BasicBlock* dom){
        func_dom_set_[bb->getParent()].find(bb)->second.insert(dom);
    }
    // void addDomTree(BasicBlock* dominator,BasicBlock*bb ){dom_tree.find(dominator)->second.insert(bb);}

    void post(Function *func_);
    void sFastIDomAlg(Function *func_);
    void domAlg(Function *func_);
    void domTreeAlg(Function *func_);
    void domFrontierAlg(Function *func_);
public:
    Dominators(Module*module, InfoManager *im);
    virtual ~Dominators(){}
    void clear();
    virtual void analyse() override;
    virtual void reAnalyse() override;
    virtual void analyseOnFunc(Function *func) override;
    void printDomFront();
    void printDomSet();
    void printDomTree();

    ::std::set<BasicBlock*> &getDomFrontier(BasicBlock* bb){ return func_dom_frontier_[bb->getParent()].find(bb)->second; }
    ::std::unordered_set<BasicBlock*> &getDomSet(BasicBlock* bb){ return func_dom_set_[bb->getParent()].find(bb)->second; }
    ::std::set<BasicBlock*> &getDomTree(BasicBlock* dominator){ return func_dom_tree_[dominator->getParent()].find(dominator)->second; }
    ::std::map<BasicBlock*,::std::set<BasicBlock*> > &getDomTree(Function *func) { return func_dom_tree_[func]; }
    ::std::map<BasicBlock*,BasicBlock*> &getIDom() { return idom_; }

    BasicBlock* findLCA(BasicBlock* lbb,BasicBlock*rbb);
    //entry为0,依次递增
    int getDomDepth(BasicBlock* bb);
    BasicBlock* getIDom(BasicBlock*b){
        auto it=idom_.find(b);
        // if(it==idom_.end())
        //     assert(0);
        return it->second;
    }
    //左边支配右边?
    bool isLdomR(BasicBlock*l,BasicBlock*r){
        auto & l_set=func_dom_set_[l->getParent()].find(l)->second;
        return l_set.count(r);
    }
    bool isLBeforeR(Instruction*l,Instruction*r);
};

#endif
