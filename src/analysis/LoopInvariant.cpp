#include "analysis/LoopInvariant.hpp"

bool LoopInvariant::isInstInvariant(Loop *loop, Instruction *inst) {
    if(!inst)
        return false;
    
    vector<Instruction*> &iiset = invariants[loop];
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
    } else if(inst->isLoad() || inst->isGep() || inst->isCall() || inst->isCmp() || inst->isFCmp()) {
    // can be better?
        return false;
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
        || op1 && isInstInvariant(loop, inst2) || op2&& isInstInvariant(loop, inst1)
        || isInstInvariant(loop, inst2) && isInstInvariant(loop, inst1) ) {
            return true;
        } else {
            return false;
        }
    }
}

void LoopInvariant::computeInvariants(Loop *loop) {
    for(BB *bb : loop->getBlocks()) {
        for(Instruction *inst : bb->getInstructions()) {
            if(isInstInvariant(loop, inst)) {
                invariants[loop].push_back(inst);
            }
        }
    }
}

void LoopInvariant::analyseOnFunc(Function *func) {
    vector<Loop*> loops = infoManager->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops) {
        computeInvariants(loop);
    } 
}

string LoopInvariant::print() {
    module_->print();

    string str = STRING_RED("Invariants:\n");
    LoopInfo *loopInfo = infoManager->getInfo<LoopInfo>();
    for(Function *f : module_->getFunctions()) {
        if(f->getBasicBlocks().size() == 0)
            continue;
        
        str += STRING_YELLOW("Function " + f->getName()) + ":\n";
        for(Loop *loop : loopInfo->getLoops(f)) {
            str += STRING_YELLOW("Loop " + loop->getHeader()->getName()) + ":\n";
            for(Value *val : invariants[loop]) {
                str += STRING(val->print()) + "\n";
            }
        }
    }
    return str;
}