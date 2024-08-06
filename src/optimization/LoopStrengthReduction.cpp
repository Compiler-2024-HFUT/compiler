#include "optimization/LoopStrengthReduction.hpp"

#include <list>
#include <algorithm>
using std::list;
using std::find;

// 对于一条线性表达式inv1 * ind + inv2，在中端它可由多条表达式构成
// endInst表示表达式的最终结果
static list<BinaryInst*> getLinearExpr(BinaryInst *endInst) {
    list<BinaryInst*> linearExpr = {};

    // 深度优先遍历线性表达式
    std::function<void(Value *) > dfs = [&](Value *val) {
        if(dynamic_cast<ConstantInt*>(val) || dynamic_cast<PhiInst*>(val))
            return;
        
        BinaryInst *bi = dynamic_cast<BinaryInst*>(val);
        LOG_ERROR("bi is not a BinaryInst", !bi)
        vector<Value*> &ops = bi->getOperands();
        dfs(ops[0]);
        dfs(ops[1]);
        linearExpr.push_back(bi);
    };

    dfs(endInst);
    return linearExpr;
} 

void LoopStrengthReduction::visitLoop(Loop *loop) {
    SCEV *scev = info_man_->getInfo<SCEV>();
    umap<Value*, SCEVExpr*> exprs = scev->getExprs(loop);
    BB *header = loop->getHeader();
    BB *latch = loop->getSingleLatch();
    BB *preheader = loop->getPreheader();
    BB *bbToInsert = nullptr;
    LOG_ERROR("preheader or latch is null, need LoopSimplified",!preheader || !latch)

    uset<Instruction*> instToDel  = {};
    for(auto [v, expr] : exprs) {
        if(!expr->isAddRec() || expr->operands.size()!= 2)
            continue;
        
        // v为中间临时变量，跳过
        list<Use> vUses = v->getUseList();
        Use vUse = vUses.front();
        if(vUses.size() == 1 && (!scev->getExpr(vUse.val_, loop) || !scev->getExpr(vUse.val_, loop)->isUnknown()))
            continue;

        BinaryInst *endInst = dynamic_cast<BinaryInst*>(v);
        if(!endInst)
            continue;
        
        // 获取endInst的相关指令集合
        list<BinaryInst*> linearExpr = getLinearExpr(endInst);
        bbToInsert = endInst->getParent();
        // 表达式只有1条，且为加减法、只乘1的乘法，跳过
        if(linearExpr.size() == 1 && 
            (linearExpr.front()->isAdd() || linearExpr.front()->isSub() || 
                (linearExpr.front()->isMul() && dynamic_cast<ConstantInt*>(linearExpr.front()->getOperand(1)) && dynamic_cast<ConstantInt*>(linearExpr.front()->getOperand(1))->getValue()==1)) )
            continue;

        LOG_WARNING("\nlinearExpr:" + endInst->getName() + ":");
        for(auto bi : linearExpr) {
            LOG_WARNING(bi->print());
        }
        
        // delete later
        for(BinaryInst *bi : linearExpr) {
            instToDel.insert(bi);
        }

        Value *phiIncomeVal1 = expr->operands[0]->val->transToValue(preheader);
        
        PhiInst *newPhi = PhiInst::createPhi(endInst->getType(), header);
        header->addInstrAfterPhiInst(newPhi);

        Value *phiIncomeVal2 = BinaryInst::createAdd(newPhi, expr->operands[1]->val->sval, bbToInsert);
        bbToInsert->getInstructions().pop_back();
        bbToInsert->addInstrAfterPhiInst(dynamic_cast<Instruction*>(phiIncomeVal2));

        newPhi->addPhiPairOperand(phiIncomeVal1, preheader);
        newPhi->addPhiPairOperand(phiIncomeVal2, latch);
        
        endInst->replaceAllUseWith(phiIncomeVal2);
    }

    for(auto inst : instToDel) {
        list<Use> &instUses =  inst->getUseList();
        bool del = true;
        for(Use u : instUses) {
            if(instToDel.count(dynamic_cast<Instruction*>(u.val_)) == 0) 
                del = false;
        }

        if(del)
            inst->getParent()->deleteInstr(inst);
    }
        
    for(Loop *inner : loop->getInners()) {
        visitLoop(inner);
    }
}

Modify LoopStrengthReduction::runOnFunc(Function* func) {
    vector<Loop*> loops = info_man_->getInfo<LoopInfo>()->getLoops(func);
    for (Loop *loop : loops) {
        visitLoop(loop);
    }    Modify mod{};

    mod.modify_instr = true;
    return mod;
}