#include "analysis/Dominators.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include <functional>
#include <iostream>
#include <ostream>
#include <set>
void Dominators::post(){
    auto bbs=func_->getBasicBlocks();
    // int id=0;
    
    ::std::function<void(/*int & id,*/BasicBlock*bb)> post_order_func=[&post_order_func,this](/*int & id,*/BasicBlock*bb){
        auto it=post_order_id_.insert({bb,-1}).first;
        // ++id;
        for(auto succ_bb:bb->getSuccBasicBlocks()){
            if(post_order_id_.find(succ_bb)==post_order_id_.end())
                post_order_func(succ_bb);
        }
        // --id;
        it->second=reverse_post_order_.size();
        reverse_post_order_.push_back(bb);
    };
    post_order_func(func_->getEntryBlock());
}
void Dominators::sFastIDomAlg(){
    post_order_id_.clear();
    reverse_post_order_.clear();
    post();
    auto entry=func_->getEntryBlock();
    bool changed=true;
    for (auto bb : this->reverse_post_order_) {
        if (bb == entry)
            continue;
        auto new_idom=*bb->getPreBasicBlocks().begin();
        setIDom(bb,new_idom);
    }

    // setIDom(entry,entry);
    setIDom(entry,nullptr);

    auto intersect=[this](BasicBlock *b1, BasicBlock *b2){
        while (b1 != b2) {
            if (post_order_id_[b1] < post_order_id_[b2]) {
                b1 = getIDom(b1);
            }else{
                b2=getIDom(b2);
            }
        }
        return b1; 
    };
    
    while (changed) {
        changed = false;
        for (auto bb : this->reverse_post_order_) {
            if (bb == entry) continue;
            auto &pre_l=bb->getPreBasicBlocks(); 
            auto new_idom=*pre_l.begin();
            if(pre_l.size()>1){
                auto b_it=pre_l.begin();
                b_it++;
                for(;b_it!=pre_l.end();b_it++)
                    new_idom=intersect(new_idom,*b_it);
            }
            if(getIDom(bb)!=new_idom){
                setIDom(bb,new_idom);
                changed=true;
            }
        }
    }

    // for(auto [bb,idom]:idom_){
    //     if(bb!=entry)
    //         ::std::cout<<bb->getName()<<" idom: "<<idom->getName()<<::std::endl;
    // }
}

void Dominators::domAlg(){
    for (auto bb : func_->getBasicBlocks()) {
        auto idom = getIDom(bb);        
        if (idom != bb&&idom!=nullptr) {
            addDomSet(idom, bb);
        }
    }
    // for(auto [bb,children]:dom_tree_){
    //     auto &sets=dom_set_.find(bb)->second;
    //     std::list<BasicBlock*>offspring{children.begin(),children.end()};
    //     std::set<BasicBlock*>visited{bb};
    //     while(!offspring.empty()){
    //         auto b=offspring.back();
    //         offspring.pop_back();
    //         if(visited.count(b))
    //             continue;
    //         visited.insert(b);
    //         sets.insert(b);
    //         auto set=getDomTree(b);
    //         std::copy(set.begin(),set.end(),inserter(offspring, offspring.end())); 
    //     }
    // }
    bool changed=true;
    while (changed) {
        changed=false;
        for(auto [domed,idom]:idom_){
            for(auto &[dom, dset]:dom_set_){
                if(idom==dom||dset.find(idom)!=dset.end()){
                    if(dset.find(domed)==dset.end()){
                        changed=true;
                        dset.insert(domed);
                    }
                }
            }
        }
        // for(auto &[dom, dsets]:dom_set_){
        //     for(auto d_set:dsets){
        //         auto size=dsets.size();
        //         auto set=getDomSet(d_set);
        //         std::copy(set.begin(),set.end(),inserter(dsets, dsets.end())); 
        //         if(dsets.size()!=size)
        //             changed=true;
        //     }
        // }
    }
}
void Dominators::domTreeAlg(){
    for (auto bb : func_->getBasicBlocks()) {
        auto idom = getIDom(bb);        
        if (idom != bb&&idom!=nullptr) {
            if(auto tree_iter=dom_tree_.find(idom);tree_iter!=dom_tree_.end())
                tree_iter->second.insert(bb);
            else
                dom_tree_.insert({idom,{bb}});
        }
    }
}
void Dominators::domFrontierAlg(){

    for (auto bb : func_->getBasicBlocks()) {
        if (bb->getPreBasicBlocks().size() <2) continue;

        for (auto pre : bb->getPreBasicBlocks()) {
            auto runner = pre;
            while (runner != getIDom(bb)) {
                // if(runner==bb) break;
                addDomFrontier(runner,bb);
                runner = getIDom(runner);
            }
        }

    }

//    for(auto &[bb,bb_set]:dom_frontier_){
//         if(auto find=bb_set.find(bb);find!=bb_set.end()){
//             bb_set.erase(bb);
//         }
//    }

}
void Dominators::clear(){
    reverse_post_order_.clear();
    post_order_id_.clear();
    idom_.clear();
    dom_set_.clear();
    dom_tree_.clear();
    dom_frontier_.clear();
}
void Dominators::analyse(){
    this->clear();
    for(auto bb:func_->getBasicBlocks()){
        dom_set_.insert({bb,{bb}});
        dom_frontier_.insert({bb,{}});
    }
    sFastIDomAlg();
    domFrontierAlg();
    domTreeAlg();
    // domAlg();
}
void Dominators::reAnalyse(){
    this->clear();
    for(auto bb:func_->getBasicBlocks()){
        dom_set_.insert({bb,{bb}});
        dom_frontier_.insert({bb,{}});
    }
    sFastIDomAlg();
    domFrontierAlg();
    domTreeAlg();
    // domAlg();
}
void Dominators::printDomFront(){
#ifdef DEBUG
    ::std::cout<<"dominate frontier:\n";
    for(auto [b ,bbset]:dom_frontier_){
        if(bbset.empty()) continue;
        ::std::cout<<b->getName()<<" domf : ";
        for(auto df:bbset){
            ::std::cout<<df->getName()<<" ";            
        }
        ::std::cout<<::std::endl;
    }
#endif
}
void Dominators::printDomSet(){
#ifdef DEBUG
    ::std::cout<<"dominate set:\n";
    for(auto &[b ,bbset]:dom_set_){
        if(bbset.empty()) continue;
        ::std::cout<<b->getName()<<" dom : "<<bbset.size()<<" ";
        for(auto dom:bbset){
            ::std::cout<<dom->getName()<<" ";            
        }
        ::std::cout<<::std::endl;
    }
#endif
}
void Dominators::printDomTree(){
#ifdef DEBUG
    ::std::cout<<"dominate tree:\n";
    for(auto &[b ,bbset]:dom_tree_){
        if(bbset.empty()) continue;
        ::std::cout<<b->getName()<<" dom : "<<bbset.size()<<" ";
        for(auto dom:bbset){
            ::std::cout<<dom->getName()<<" ";            
        }
        ::std::cout<<::std::endl;
    }
#endif
}
Dominators::Dominators(Function* fun):FunctionInfo(fun){
    for(auto bb:fun->getBasicBlocks()){
        dom_set_.insert({bb,{bb}});
        dom_frontier_.insert({bb,{}});
    }
    sFastIDomAlg();
    domFrontierAlg();
    domTreeAlg();
    // 没有用到，而且当前算法非常影响性能，之后可能要改
    // domAlg();
    // printDomFront();
    // printDomSet();
    // printDomTree();
}