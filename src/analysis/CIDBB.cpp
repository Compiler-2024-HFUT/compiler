#include "analysis/CIDBB.hpp"
#include "midend/BasicBlock.hpp"

void CIDBB::analyseOnFunc(Function *func){
    auto func_initial = initialFunction(func);
    if(func_initial!=nullptr){
        marker.clear();
        marker[func->getEntryBlock()] = visiting;
        calInDreegeOfBB(func->getEntryBlock());
    }
    return;
}

Function* CIDBB::initialFunction(Function *func){
    if(func->getNumBasicBlocks()!=0){
        for(auto bb: func->getBasicBlocks())
            inDegreeInitialize(bb);
        return func; 
    }
    return nullptr;


}

void CIDBB:: calInDreegeOfBB(BasicBlock* bb){
    for(auto bb_succ : bb->getSuccBasicBlocks()){
        if(marker[bb_succ]==unvisited){
            inDegreeUpdate(bb_succ, 1);
            marker[bb_succ]=visiting;
            calInDreegeOfBB(bb_succ);
        }
        else if(marker[bb_succ]==visited){
            inDegreeUpdate(bb_succ, 1);
        }

    }
    marker[bb] = visited;
    
}