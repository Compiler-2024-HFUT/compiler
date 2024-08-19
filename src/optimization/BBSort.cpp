#include "optimization/BBSort.hpp"
#include "midend/BasicBlock.hpp"
#include <queue>
#include <limits>

bool BBSort::sortBB(Function *func) {
    if(func->getBasicBlocks().size() <= 1)
        return false;

    std::unordered_map<BasicBlock*, uint32_t> weight;
    constexpr uint32_t branchTrueCost = 100;
    constexpr uint32_t branchFalseCost = 101;
    constexpr uint32_t ubrCost = 2;
    constexpr uint32_t brCost = 0;
    constexpr uint32_t switchCost = 3;
    constexpr uint32_t retCost = 10;
    constexpr uint32_t unreachableCost = 1'000'000;

    std::queue<BasicBlock*> q{ { func->getEntryBlock() } };
    const auto addTarget = [&](BasicBlock* block, uint32_t w) {
        if(weight.emplace(block, w).second)
            q.push(block);
    };

    while(!q.empty()) {
        BasicBlock *u = q.front();
        q.pop();
        auto& val = weight[u];
        Instruction *term = u->getTerminator();
        
        if(term->isBr()) {
            // cond br
            if(term->getOperands().size() == 3) {
                addTarget(dynamic_cast<BasicBlock*>(term->getOperand(1)), val + branchTrueCost);
                addTarget(dynamic_cast<BasicBlock*>(term->getOperand(2)), val + branchFalseCost);
                val += ubrCost;
            } else {
                addTarget(dynamic_cast<BasicBlock*>(term->getOperand(0)), val + branchTrueCost);
                val += brCost;
            }
        } else if(term->isCmpBr() || term->isFCmpBr()) {
            addTarget(dynamic_cast<BasicBlock*>(term->getOperand(2)), val + branchTrueCost);
            addTarget(dynamic_cast<BasicBlock*>(term->getOperand(3)), val + branchFalseCost);
            val += ubrCost;
        } else if(term->isRet()) {
            val += retCost;
        } else {
            assert(0);
        }
    }   

    for(auto& block : func->getBasicBlocks())
        if(!weight.count(block))
            weight[block] = std::numeric_limits<uint32_t>::max();

    const auto comp = [&](BasicBlock* lhs, BasicBlock* rhs) { return weight[lhs] < weight[rhs]; };
    bool modified = !std::is_sorted(func->getBasicBlocks().begin(), func->getBasicBlocks().end(), comp);
    if(modified)
        func->getBasicBlocks().sort(comp);
    return modified;    
}

Modify BBSort::runOnFunc(Function*func) {
    Modify mod;
    mod.modify_bb = sortBB(func);
    mod.modify_instr = false;
    mod.modify_call = false;
    return mod;
}