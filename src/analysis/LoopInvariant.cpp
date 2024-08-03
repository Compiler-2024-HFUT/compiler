#include "analysis/LoopInvariant.hpp"

bool LoopInvariant::isInstInvariant(Loop *loop, Instruction *inst) {
    if(!inst)
        return false;
    
    list<Instruction*> &iiset = invariants[loop];
    if( find(iiset.begin(), iiset.end(), inst ) != iiset.end() )
        return true;

    // inst来自循环外
    if(!loop->contain(inst->getParent()))
        return true;

    if(inst->isAlloca()) {
    // is this right? 
        return true;
    } else if(inst->isVoid() || inst->isPhi()) {
        return false;
    } else if(inst->isLoad() || inst->isGep() ||  inst->isCmp() || inst->isFCmp()) {
    // can be better?
        return false;
    } else if (inst->isCall()) {
        vector<Value*> &ops = inst->getOperands();
        for(Value *op : ops) {
            if( dynamic_cast<Constant*>(op) )
                continue;
            if ( !isInstInvariant(loop, dynamic_cast<Instruction*>(op)) )
                return false;
        }
        return true;
    } else if (inst->isSitofp() || inst->isFptosi()) {
        Value *op = inst->getOperand(0);
        if( dynamic_cast<Constant*>(op) )
            return true;
        else 
            return isInstInvariant(loop, dynamic_cast<Instruction*>(op));
    } else {
        LOG_ERROR("inst isn't a BinaryInst!", !inst->isBinary())

        Instruction *inst1 = dynamic_cast<Instruction*>( inst->getOperand(0) );
        Instruction *inst2 = dynamic_cast<Instruction*>( inst->getOperand(1) );

        Constant *op1 = dynamic_cast<Constant*>( inst->getOperand(0) );
        Constant *op2 = dynamic_cast<Constant*>( inst->getOperand(1) );
        if( op1 && op2
        || op1 && isInstInvariant(loop, inst2) || op2 && isInstInvariant(loop, inst1)
        || isInstInvariant(loop, inst2) && isInstInvariant(loop, inst1) ) {
            return true;
        } else {
            return false;
        }
    }
}

void LoopInvariant::computeInvariants(Loop *loop) {
    // 每个BB对应的最里层loop
    umap<BB*, Loop*> bbLoop = {};
    std::function<void(Loop*)> findBBLoop = [&](Loop *loop) {
        for(Loop *inner : loop->getInners()) {
            findBBLoop(inner);
        }

        for(BB *bb : loop->getBlocks()) {
            if(bbLoop.count(bb))
                continue;
            bbLoop[bb] = loop;
        }
    };
    findBBLoop(loop);

    std::function<void(Loop*)> findInvariants = [&](Loop *loop) {
        // 先计算子循环不变量, 之后再检查其是否在外面也是不变量
        for(Loop *inner : loop->getInners()) {
            findInvariants(inner);
            for(auto iter = invariants[inner].begin(); iter!= invariants[inner].end();) {
                if(isInstInvariant(loop, *iter)) {
                    invariants[loop].push_back(*iter);
                    iter = invariants[inner].erase(iter);
                } else {
                    ++iter;
                }
            }
        }

        for(BB *bb : loop->getBlocks()) {
            if(bbLoop[bb]!= loop)
                continue;
            for(Instruction *inst : bb->getInstructions()) {
                if(isInstInvariant(loop, inst)) {
                    invariants[loop].push_back(inst);
                }
            }
        }
    };
    findInvariants(loop);
}

void LoopInvariant::analyseOnFunc(Function *func) {
    vector<Loop*> loops = infoManager->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops) {
        computeInvariants(loop);
    } 
}

string LoopInvariant::print() {
    module_->print();

    std::function<string(Loop *)> printInner = [&](Loop *l) -> string {
        string innerStr = "";
        for(Loop *inner : l->getInners()) {
            innerStr += STRING_RED("Loop<" + inner->getHeader()->getName() + ">:\n");
            for(Instruction *inst : invariants[inner]) {
                innerStr += STRING_YELLOW(inst->getName()) + ": \n";
                innerStr += STRING(inst->print()) + "\n";
            }
            
            innerStr += printInner(inner);
        }
        return innerStr;
    };

    string str = STRING_RED("Invariants:\n");
    LoopInfo *loopInfo = infoManager->getInfo<LoopInfo>();
    for(Function *f : module_->getFunctions()) {
        if(f->getBasicBlocks().size() == 0)
            continue;
        
        str += STRING_YELLOW("Function " + f->getName()) + ":\n";
        for(Loop *loop : loopInfo->getLoops(f)) {
            str += STRING_RED("Loop<" + loop->getHeader()->getName() + ">:\n");
            for(Instruction *inst : invariants[loop]) {
                str += STRING_YELLOW(inst->getName()) + ": \n";
                str += STRING(inst->print()) + "\n";
            }
            str += printInner(loop);
        }
    }
    return str;
}