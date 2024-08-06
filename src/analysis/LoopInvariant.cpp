#include "analysis/LoopInvariant.hpp"

bool LoopInvariant::isValueInvariant(Loop *loop, Value *val) {
    if(dynamic_cast<Instruction*>(val))
        return isInstInvariant(loop, dynamic_cast<Instruction*>(val));
    else if(dynamic_cast<Constant*>(val) || dynamic_cast<Argument*>(val)||dynamic_cast<GlobalVariable*>(val))
        return true;
    else    
        return false;
}

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
    } else if (inst->isCmp() || inst->isFCmp()) {
        return false;
    } else if (inst->isLoad()) {    
        Type *opType = inst->getOperand(0)->getType();
        // 指向指针的指针其值总不变
        if(opType->getPointerElementType()->isPointerType()) 
            return true;
        // 普通指针load的值可能因store而变化
        return false;
    } else if (inst->isCall() || inst->isGep()) {
        vector<Value*> &ops = inst->getOperands();
        for(Value *op : ops) {
            if ( !isValueInvariant(loop, op) )
                return false;
        }
        return true;
    } else if (inst->isSitofp() || inst->isFptosi()) {
        Value *op = inst->getOperand(0);
        return isValueInvariant(loop, op);
    } else {
        LOG_ERROR("inst isn't a BinaryInst!", !inst->isBinary())

        Value *op1 = inst->getOperand(0);
        Value *op2 = inst->getOperand(1);

        if(isValueInvariant(loop, op1) && isValueInvariant(loop, op2) ) {
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