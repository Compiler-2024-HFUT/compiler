#include "optimization/MoveAlloca.hpp"
#include "midend/BasicBlock.hpp"

Modify MoveAlloca::runOnFunc(Function *function){
    if(function->isDeclaration())
        return{};
    moveAlloc(function);
    return {};
}

void MoveAlloca::moveAlloc(Function*functon){
    std::vector<Instruction*>to_move_allocs{};
    auto entry=functon->getEntryBlock();
    for(auto b:functon->getBasicBlocks()){
        auto &insts_list=b->getInstructions();
        for(auto __iter=insts_list.begin();__iter!=insts_list.end();){
            auto curiter=__iter++;
            auto cur_ins=*curiter;
            if((cur_ins)->isAlloca()){
                to_move_allocs.push_back((cur_ins));
                cur_ins->setParent(entry);
                insts_list.erase(curiter);
            }
        }
    }
    while(!to_move_allocs.empty()){
        auto ins=to_move_allocs.back();
        to_move_allocs.pop_back();
        entry->addInstrBegin(ins);
    }
}