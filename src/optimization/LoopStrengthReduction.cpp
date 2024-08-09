#include "optimization/LoopStrengthReduction.hpp"
#include "analysis/Dominators.hpp"

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
        BinaryInst *bi = dynamic_cast<BinaryInst*>(val);
        if(!bi)
            return;
        
        dfs(bi->getOperand(0));
        dfs(bi->getOperand(1));
        linearExpr.push_back(bi);
    };

    dfs(endInst);
    return linearExpr;
} 

void LoopStrengthReduction::visitLoop(Loop *loop) {
    SCEV *scev = info_man_->getInfo<SCEV>();
    Dominators *dom = info_man_->getInfo<Dominators>();
    umap<Value*, SCEVExpr*> exprs = scev->getExprs(loop);
    BB *header = loop->getHeader();
    BB *latch = loop->getSingleLatch();
    BB *preheader = loop->getPreheader();
    BB *bbToInsert = nullptr;
    LOG_ERROR("preheader or latch is null, need LoopSimplified",!preheader || !latch)

    uset<BB*> domSet = {};
    uset<Instruction*> instToDel  = {};
    for(auto [v, expr] : exprs) {
        BinaryInst *inst = dynamic_cast<BinaryInst*>(v);
        if(!inst || !expr || expr->isUnknown()) 
            continue;
        bbToInsert = inst->getParent();
        
        domSet = dom->getDomSet(inst->getParent());
        if(!domSet.count(latch))
            continue;

        // 获取inst的相关指令集合
        list<BinaryInst*> linearExpr = getLinearExpr(inst);

        // 如果表达式的结果为单值，直接替换
        if(expr->isValue() && ( expr->getValue()->isConst() || expr->getValue()->isInv() ) ) {
            if(inst != expr->getValue()->sval) {
                inst->replaceAllUseWith(expr->getValue()->sval);
                // delete later
                for(BinaryInst *bi : linearExpr) {
                    instToDel.insert(bi);
                }
            }
        } else if(expr->isValue()) {
            // todo later
            continue;
        }

        // 如果不是迭代的表达式
        if(!expr->isAddRec() || expr->operands.size()!= 2)
            continue;
        
        // v为中间临时变量，跳过
        list<Use> vUses = v->getUseList();
        Use vUse = vUses.front();
        if(vUses.size() == 1 && (!scev->getExpr(vUse.val_, loop) || !scev->getExpr(vUse.val_, loop)->isUnknown()))
            continue;
        
        // 表达式只有1条，且为加减法、只乘1的乘法，跳过，影响消除和外移
        if(linearExpr.size() == 1 && 
            (linearExpr.front()->isAdd() || linearExpr.front()->isSub() || 
                (linearExpr.front()->isMul() && dynamic_cast<ConstantInt*>(linearExpr.front()->getOperand(1)) && dynamic_cast<ConstantInt*>(linearExpr.front()->getOperand(1))->getValue()==1)) )
            continue;

        // log linearExpr
        LOG_WARNING("linearExpr:" + inst->getName() + ":");
        for(auto bi : linearExpr) {
            LOG_WARNING(bi->print());
        }
        
        // 可识别的迭代关系式，之后删除
        for(BinaryInst *bi : linearExpr) {
            instToDel.insert(bi);
        }

        Value *phiIncomeVal1 = expr->operands[0]->val->transToValue(preheader);
        
        PhiInst *newPhi = PhiInst::createPhi(inst->getType(), header);
        header->addInstrAfterPhiInst(newPhi);

        Value *phiIncomeVal2 = BinaryInst::createAdd(newPhi, expr->operands[1]->val->sval, latch);
        latch->getInstructions().pop_back();
        latch->addInstrBeforeTerminator(dynamic_cast<Instruction*>(phiIncomeVal2));

        newPhi->addPhiPairOperand(phiIncomeVal1, preheader);
        newPhi->addPhiPairOperand(phiIncomeVal2, latch);
        
        inst->replaceAllUseWith(newPhi);
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

void LoopStrengthReduction::runOnFunc(Function* func) {
    vector<Loop*> loops = info_man_->getInfo<LoopInfo>()->getLoops(func);
    for (Loop *loop : loops) {
        visitLoop(loop);
    }
}