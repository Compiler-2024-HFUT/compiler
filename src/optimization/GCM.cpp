#include "optimization/GCM.hpp"

bool GCM::isPinned(Instruction *inst) {
    return (inst->isBinary() || inst->isCmp() || inst->isFCmp() || 
            inst->isGep() || inst->isZext() || 
            inst->isFptosi() || inst->isSitofp());
}

void GCM::computeDepths(Function *func) {
    domDepth.clear();
    loopDepth.clear();

    Dominators *dom = info_man_->getInfo<Dominators>();
    map<BasicBlock*, set<BasicBlock*> > &domTree = dom->getDomTree(func);
    LoopInfo *loopinfo = info_man_->getInfo<LoopInfo>();
    vector<Loop*> loops = loopinfo->getLoops(func);
    BasicBlock *root = func->getEntryBlock();

    std::function<void(BasicBlock*)> DFSDomTree = [&](BasicBlock *bb) {
        // start from 0
        if(domDepth.count(bb) == 0)
            domDepth[bb] = 0;
        
        for(BasicBlock *child : domTree[bb]) {
            domDepth[child] = domDepth[bb] + 1;
            DFSDomTree(child);
        }
    };

    std::function<void(Loop*)> DFSLoop = [&](Loop *loop) {
        for(Loop *l : loop->getInners()) {
            DFSLoop(l);
        }

        for(BasicBlock *bb : loop->getBlocks()) {
            if(loopDepth.count(bb) == 0)
                loopDepth[bb] = loop->getDepth();
        }
    };

    DFSDomTree(root);

    for(Loop *loop : loops) {
        DFSLoop(loop);
    }
    for(BasicBlock *bb : func->getBasicBlocks()) {
        if(loopDepth.count(bb) == 0)
            loopDepth[bb] = 0;
    }
}

BasicBlock *GCM::findLCA(BasicBlock *bb1, BasicBlock *bb2) {
    Dominators *dom = info_man_->getInfo<Dominators>();
    map<BasicBlock*, BasicBlock*> idom = dom->getIDom();

    if(bb1 == nullptr)  return bb2;
    if(bb2 == nullptr)  return bb1;

    while(domDepth[bb1] < domDepth[bb2])
        bb1 = idom[bb1];
    while(domDepth[bb1] > domDepth[bb2])
        bb2 = idom[bb2];
    while(bb1!= bb2) {
        bb1 = idom[bb1];
        bb2 = idom[bb2];
    }
    return bb1;
}

void GCM::scheduleEarly(Instruction *inst) {
    if(visited.count(inst))
        return;
    visited.insert(inst);

    earlyBB[inst] = inst->getParent();
    for(Value *op : inst->getOperands()) {
        Instruction *opInst = dynamic_cast<Instruction*>(op);
        if(!opInst)
            continue;

        scheduleEarly(opInst);
        BasicBlock *opBB = opInst->getParent();
        if(domDepth[opBB] < domDepth[ earlyBB[inst] ]) {
            earlyBB[inst] = opBB;
        }
    }
}


bool GCM::scheduleLate(Instruction *inst) {
    if(visited.count(inst))
        return false;
    visited.insert(inst);

    bool changed = false;
    BasicBlock *lca = nullptr;
    for(Use user : inst->getUseList()) {
        Instruction *userInst = dynamic_cast<Instruction*>(user.val_);
        if(!userInst)
            continue;

        changed |= scheduleLate(userInst);
        BasicBlock *userBB = userInst->getParent();
        if(userInst->isPhi()) {
            vector<Value*> &ops = userInst->getOperands();
            for(int i = 1; i < ops.size(); i += 2) {
                userBB = dynamic_cast<BasicBlock*>(ops[i]);
                lca = (lca) ? findLCA(lca, userBB) : userBB;
            }
        }
        lca = (lca) ? findLCA(lca, userBB) : userBB;
    }

    Dominators *dom = info_man_->getInfo<Dominators>();
    map<BasicBlock*, BasicBlock*> idom = dom->getIDom();
    BasicBlock *bestBB = lca;
    BasicBlock *curBB = lca;
    while(domDepth[curBB] >= domDepth[ earlyBB[inst] ]) {
        if(loopDepth[curBB] < loopDepth[bestBB])
            bestBB = curBB;
        curBB = idom[curBB];
    }

    if(bestBB != inst->getParent()) {
        inst->getParent()->deleteInstr(inst);
        bestBB->addInstrBeforeTerminator(inst);
        changed = true;
    }
    return changed;
}

bool GCM::visitFunction(Function *func) {
    bool changed = false;
    computeDepths(func);
    
    visited.clear();

    // ScheduleEarly
    for(BasicBlock *bb : func->getBasicBlocks()) {
        for(Instruction *inst : bb->getInstructions()) {
            if(isPinned(inst)) {
                visited.insert(inst);
                for(Value *op : inst->getOperands()) {
                    Instruction *opInst = dynamic_cast<Instruction*>(op);
                    if(!opInst)
                        continue;
                    scheduleEarly(opInst);
                }
            }
        }
    }

    // ScheduleLate
    for(BasicBlock *bb : func->getBasicBlocks()) {
        for(Instruction *inst : bb->getInstructions()) {
            if(isPinned(inst)) {
                visited.insert(inst);
                for(Use user : inst->getUseList()) {
                    Instruction *userInst = dynamic_cast<Instruction*>(user.val_);
                    if(!userInst)
                        continue;
                    changed |= scheduleLate(userInst);
                }
            }
        }
    }
    return changed;
}

Modify GCM::runOnModule(Module *m) {
    Modify ret{};
    for(Function *func : m->getFunctions()) {
        if( visitFunction(func) ) 
            ret.modify_instr = true;
    }
    return ret;
}