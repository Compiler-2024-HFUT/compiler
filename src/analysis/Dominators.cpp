#include "analysis/Dominators.hpp"
#include "midend/Module.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include <algorithm>
#include <functional>
#include <ostream>
#include <set>
//寻找公共祖先
BasicBlock* Dominators::findLCA(BasicBlock* lbb,BasicBlock*rbb){
    auto findPathToRoot=[this](BasicBlock* block) {
        std::vector<BasicBlock*> path;
        while (block) {
            path.push_back(block);
            block = this->getIDom(block);
        }
        return path;
    };
    auto pathl = findPathToRoot(lbb);
    auto pathr = findPathToRoot(rbb);

     std::set<BasicBlock*> ancestors_l(pathl.begin(), pathl.end());
     for (auto block : pathr) {
         if (ancestors_l.count(block)) {
             return block;
         }
     }
     return nullptr;  // 如果没有找到公共祖先，返回 nullptr
}
void Dominators::post(Function *func_){
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

void Dominators::sFastIDomAlg(Function *func_){
    idom_.clear();
    post_order_id_.clear();
    reverse_post_order_.clear();
    post(func_);
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
}

void Dominators::domAlg(Function *func_){
    std::function<::std::set<BasicBlock*>(BasicBlock*)> getdoms=[&,this](BasicBlock*bb){
        auto &dom_set=func_dom_set_.find(func_)->second.find(bb)->second;
        auto b_set=getDomTree(bb);
        if(b_set.empty())return std::set<BasicBlock*>{};
        ::std::vector<BasicBlock*> work_list(b_set.begin(),b_set.end());
        for(auto tree_node:work_list){
            auto b=getdoms(tree_node);
            if(b.empty())continue;
            std::copy(b.begin(),b.end(),inserter(b_set, b_set.end()));
        }
        std::copy(b_set.begin(),b_set.end(),inserter(dom_set, dom_set.end()));
        return b_set;
    };

    getdoms(func_->getEntryBlock());
}
void Dominators::domTreeAlg(Function *func_){
    for (auto bb : func_->getBasicBlocks()) {
        auto idom = getIDom(bb);
        if (idom != bb&&idom!=nullptr) {
            auto tree_iter=func_dom_tree_[func_].find(idom);
            if(tree_iter!=func_dom_tree_[func_].end())
                tree_iter->second.insert(bb);
            else
                func_dom_tree_[func_].insert({idom,{bb}});
        }
    }
}
void Dominators::domFrontierAlg(Function *func_){

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

    func_dom_set_.clear();
    func_dom_tree_.clear();
    func_dom_frontier_.clear();

    for(Function *f : module_->getFunctions()) {
        if(f->getBasicBlocks().size() == 0)
            continue;
        for(BasicBlock *bb : f->getBasicBlocks()) {
            func_dom_tree_[f].insert({bb,{}});
            func_dom_set_[f].insert({bb,{bb}});
            func_dom_frontier_[f].insert({bb,{}});
        }
    }

}
void Dominators::analyse(){
    this->clear();
    for(Function *f : module_->getFunctions()) {
        if(f->getBasicBlocks().empty())
            continue;
        analyseOnFunc(f);
    }
    printDomSet();
    invalid = false;
}
void Dominators::reAnalyse(){
    // clear();
    analyse();
}

void Dominators::analyseOnFunc(Function *func) {
    sFastIDomAlg(func);
    domFrontierAlg(func);
    domTreeAlg(func);
    domAlg(func);       // slow??
}

void Dominators::printDomFront(){
#ifdef DEBUG
    ::std::cout<<"dominate frontier:\n";
    for(auto f : module_->getFunctions()) {
        for(auto [b ,bbset]:func_dom_frontier_[f]){
            if(bbset.empty()) continue;
            ::std::cout<<b->getName()<<" domf : ";
            for(auto df:bbset){
                ::std::cout<<df->getName()<<" ";
            }
            ::std::cout<<::std::endl;
        }
    }

#endif
}
void Dominators::printDomSet(){
#ifdef DEBUG
    ::std::cout<<"dominate set:\n";
    for(auto f : module_->getFunctions()) {
        ::std::cout<<f->getName()<<"\n";
        for(auto &[b ,bbset]:func_dom_set_[f]){
            if(bbset.empty()) continue;
            ::std::cout<<"\t"<<b->getName()<<" dom : "<<bbset.size()<<"\n";
            for(auto dom:bbset){
                ::std::cout<<"\t\t"<<dom->getName()<<"\n";
            }
            ::std::cout<<::std::endl;
        }
    }
#endif
}
void Dominators::printDomTree(){
#ifdef DEBUG
    ::std::cout<<"dominate tree:\n";
    for(auto f : module_->getFunctions()) {
        ::std::cout<<f->getName()<<"\n";
        for(auto &[b ,bbset]:func_dom_tree_[f]){
            if(bbset.empty()) continue;
            ::std::cout<<"\t"<<b->getName()<<" dom : "<<bbset.size()<<"\n";
            for(auto dom:bbset){
                ::std::cout<<"\t\t"<<dom->getName()<<"\n";
            }
            ::std::cout<<::std::endl;
        }
    }

#endif
}
Dominators::Dominators(Module*module, InfoManager *im): FunctionInfo(module, im) {
    // this->clear();

    /*
    // 没有用到，而且当前算法非常影响性能，之后可能要改
    // domAlg();
    // printDomFront();
    // printDomSet();
    // printDomTree();
     */
}
