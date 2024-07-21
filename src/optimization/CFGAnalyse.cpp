#include "optimization/CFGAnalyse.hpp"
#include"midend/BasicBlock.hpp"

void CFGAnalyse::runOnFunc(Function *func){
    
        
        if(func->getNumBasicBlocks()!=0){
            
        /*
        // std::cout<<"**************************************************"<<std::endl;
        // std::cout<<func->print()<<std::endl;
        */
        for(auto BB : func->getBasicBlocks()){
            BB->incomingReset();
            BB->loopDepthReset();
        }
        incoming_find(func);
        loop_find(func);
        /*
        // for(auto BB : func->get_basic_blocks()){
        //     std::cout<<"============================================="<<std::endl;
        //     std::cout<<BB->print()<<std::endl;
        //     std::cout << "incoming_branch:" << std::endl;
        //     std::cout<<BB->get_incoming_branch()<<std::endl;
        //     std::cout << "loop_depth:" << std::endl;
        //     std::cout<<BB->get_loop_depth()<<std::endl;
        //     std::cout<<"============================================="<<std::endl;
        // }
        // std::cout<<"**************************************************"<<std::endl;
        */}
    
}

void CFGAnalyse::incoming_find(Function* func){
    color.clear();
    auto bb = func->getEntryBlock();
    color[bb] = 1;
    incoming_DFS(bb);
}

//& get In-Degree during Depth-First Traversal -> BasicBlock::incoming_branch
void CFGAnalyse::incoming_DFS(BasicBlock* bb){
    for (auto succ_bb : bb->getSuccBasicBlocks()){
        if (color[succ_bb] == 0){
            succ_bb->incomingAdd(1);
            color[succ_bb] = 1;
            incoming_DFS(succ_bb);
        }
        else if (color[succ_bb] == 2){
            succ_bb->incomingAdd(1);
        }
    }
    color[bb] = 2;
}

int DFN,LOW;
std::map<BasicBlock*,int> BB_DFN;
std::map<BasicBlock*,int> BB_LOW;
std::stack<BasicBlock*> BB_Stack;
std::vector<BasicBlock*>* BBs;
std::stack<std::vector<BasicBlock*>*> loop_stack;


void CFGAnalyse::loop_find(Function* func){
    DFN = 0;
    BB_DFN.clear();
    BB_LOW.clear();
    color.clear();
    tarjan_DFS(func->getEntryBlock());
    while(!loop_stack.empty()){
        auto loop = loop_stack.top();
        loop_stack.pop();
        if (bb_loop[find_loop_entry(loop)]!=nullptr){
            outer_loop[loop] = bb_loop[find_loop_entry(loop)];
        }
        for (auto BB : *loop){
            BB->loopDepthAdd(1);
            bb_loop[BB] = loop;
        }
        color[find_loop_entry(loop)] = 3;
        DFN = 0;
        BB_DFN.clear();
        BB_LOW.clear();
        for (auto succ : find_loop_entry(loop)->getSuccBasicBlocks()){
            if (bb_loop[succ] == loop && color[succ]!=3){
                tarjan_DFS(succ);
            }
        }
    }
}

//& calculate Loop Nesting Depth using Tarjan's Algorithm
void CFGAnalyse::tarjan_DFS(BasicBlock* BB){
    DFN++;
    BB_DFN[BB] = DFN;
    BB_LOW[BB] = DFN;
    BB_Stack.push(BB);
    color[BB] = 1;
    for(auto succ_bb : BB->getSuccBasicBlocks()){
        if (color[succ_bb] != 3){
            if (BB_DFN[succ_bb] == 0){
                tarjan_DFS(succ_bb);
                if (BB_LOW[succ_bb] < BB_LOW[BB]){
                    BB_LOW[BB] = BB_LOW[succ_bb];
                }
            }
            else if (BB_LOW[succ_bb] < BB_LOW[BB] && color[succ_bb] == 1){
                BB_LOW[BB] = BB_LOW[succ_bb];
            }
        }
    }
    if(BB_DFN[BB] == BB_LOW[BB]){
        BBs = new std::vector<BasicBlock *>;
        auto bb_top = BB_Stack.top();
        while(bb_top != BB){
            BB_Stack.pop();
            color[bb_top] = 2;
            BBs->push_back(bb_top);
            bb_top = BB_Stack.top();
        }
        BB_Stack.pop();
        color[bb_top] = 2;
        BBs->push_back(bb_top);
        if(BBs->size() > 1){
            loops.push_back(BBs);
            loop_stack.push(BBs);
        }
    }
}

