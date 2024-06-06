#include "backend/CIDBB.hpp"

void CIDBB::run(){
    auto Funcs = initialFunctions(moudle_);
    for(auto func:Funcs){
        marker.clear();
        marker[func->getEntryBlock()] = visiting;
        calInDreegeOfBB(func->getEntryBlock());

    }
    
}

::std::vector<Function*> CIDBB::initialFunctions(Module *m){
    ::std::vector<Function*> Funcs;
    for(auto function: m->getFunctions())
        if(function->getNumBasicBlocks()!=0){
            for(auto bb: function->getBasicBlocks())
                bb->incomingReset();
            Funcs.push_back(function);
        }
    return Funcs;


}

void CIDBB:: calInDreegeOfBB(BasicBlock* bb){
    for(auto bb_succ : bb->getSuccBasicBlocks())
        if(marker[bb_succ]==unvisited){
            bb_succ->incomingAdd(1);
            marker[bb_succ]=visiting;
            calInDreegeOfBB(bb_succ);
        }
        else if(marker[bb_succ]==visited)
            bb_succ->incomingAdd(1);
    marker[bb] = visited;
  
}