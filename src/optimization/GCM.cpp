#include "optimization/GCM.hpp"
#include "utils/Logger.hpp"

#include <list>
using std::list;

bool GCM::isPinned(Instruction *inst) {
    return (inst->isPhi() || inst->isTerminator() || inst->isStore() || inst->isLoad() || inst->isCall());
}

void GCM::computeDepths(Function *func) {
    domDepth.clear();
    loopDepth.clear();

    Dominators *dom = info_man_->getInfo<Dominators>();
    map<BasicBlock*, set<BasicBlock*> > &domTree = dom->getDomTree(func);
    LoopInfo *loopinfo = info_man_->getInfo<LoopInfo>();
    vector<Loop*> loops = loopinfo->getLoops(func);
    BasicBlock *root = func->getEntryBlock();

    domDepth[root] = 0;
    std::function<void(BasicBlock*)> DFSDomTree = [&](BasicBlock *bb) {
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
    if(bb1 == nullptr)  return bb2;
    
    Dominators *dom = info_man_->getInfo<Dominators>();
    map<BasicBlock*, BasicBlock*> idom = dom->getIDom();

    while(domDepth[bb1] > domDepth[bb2])
        bb1 = idom[bb1];
    while(domDepth[bb1] < domDepth[bb2])
        bb2 = idom[bb2];
    while(bb1 != bb2) {
        bb1 = idom[bb1];
        bb2 = idom[bb2];
    }
    LOG_ERROR("lca can't be null", !bb1)
    return bb1;
}

Instruction *GCM::scheduleEarly(Instruction *inst) {
    if(visited.count(inst))
        return inst;
    visited.insert(inst);

    BasicBlock *instBB = inst->getFunction()->getEntryBlock();
    for(Value *op : inst->getOperands()) {
        if( Instruction *input = dynamic_cast<Instruction*>(op) ) {
            input = scheduleEarly(input);
            // from shallow to deep
            if(domDepth[ instBB ] < domDepth[ input->getParent() ]) {
                instBB = input->getParent();
            }
        }
    }

    LOG_WARNING("earlyBB: " + inst->getName() + ", " + instBB->getName())

    if(!isPinned(inst) && inst->getParent() != instBB) {
        inst->getParent()->getInstructions().remove(inst);
        instBB->addInstrBeforeTerminator(inst);
    }
    
    return inst;
}

Instruction *GCM::scheduleLate(Instruction *inst) {
    if(visited.count(inst))
        return inst;
    visited.insert(inst);

    Dominators *dom = info_man_->getInfo<Dominators>();
    map<BasicBlock*, BasicBlock*> idom = dom->getIDom();

    BasicBlock *lca = nullptr;
    list<Use> uselist = list<Use>(inst->getUseList().begin(), inst->getUseList().end());
    for(Use user : uselist) {
        if(Instruction *userInst = dynamic_cast<Instruction*>(user.val_)) {
            userInst = scheduleLate(userInst);
            BasicBlock *userBB = userInst->getParent();
            if(userInst->isPhi()) {
                vector<Value*> &ops = userInst->getOperands();
                for(int i = 0; i < ops.size(); i += 2) {
                    if(ops[i] == inst)
                        userBB = dynamic_cast<BasicBlock*>(ops[i+1]);
                        break;
                }
            }
            lca = findLCA(lca, userBB);
        }
    }
    
    LOG_WARNING("last: " + inst->getName() + ", " + lca->getName())

    BasicBlock *bestBB = lca;
    BasicBlock *instBB = inst->getParent();
    while(lca != nullptr && lca != instBB) {
        if(loopDepth[lca] < loopDepth[bestBB])
            bestBB = lca;
        lca = idom[lca];
    }
    if(!lca) return inst;

    uselist = list<Use>(inst->getUseList().begin(), inst->getUseList().end());
    for(Use user : uselist) {
        if(Instruction *userInst = dynamic_cast<Instruction*>(user.val_)) {
            if(!isPinned(inst) && userInst->getParent() == bestBB && !userInst->isPhi()) {
                list<Instruction*> &userBBInstList = userInst->getParent()->getInstructions();
                auto pos = find(userBBInstList.begin(), userBBInstList.end(), userInst);
                
                inst->getParent()->getInstructions().remove(inst);
                inst->setParent(userInst->getParent());
                userInst->getParent()->addInstruction(pos, inst);
                return inst;
            }
        }
    }

    if(!isPinned(inst) && bestBB != instBB) {
        inst->getParent()->getInstructions().remove(inst);
        bestBB->addInstrBeforeTerminator(inst);
    }
    return inst;
}

bool GCM::visitFunction(Function *func) {
    bool changed = false;
    computeDepths(func);

    // ScheduleEarly
    visited.clear();
    for(BasicBlock *bb : func->getBasicBlocks()) {
        for(Instruction *inst : bb->getInstructions()) {
            if(isPinned(inst)) {
                visited.insert(inst);
                for(Value *op : inst->getOperands()) {
                    if(Instruction *input = dynamic_cast<Instruction*>(op))
                        scheduleEarly(input);
                }
            }
        }
    }

    // ScheduleLate
    visited.clear();
    for(BasicBlock *bb : func->getBasicBlocks()) {
        list<Instruction*> backupInsts = list<Instruction*>(bb->getInstructions().begin(), bb->getInstructions().end());
        for(Instruction *inst : backupInsts) {
            if(isPinned(inst)) {
                visited.insert(inst);
                list<Use> uselist = list<Use>(inst->getUseList().begin(), inst->getUseList().end());
                for(Use user : uselist) {
                    if(Instruction *userInst = dynamic_cast<Instruction*>(user.val_))
                        scheduleLate(userInst);
                }
            }
        }
    }

    return changed;
}

Modify GCM::runOnModule(Module *m) {
    Modify ret{};
    for(Function *func : m->getFunctions()) {
        if(func->getBasicBlocks().size() == 0)
            continue;

        if( visitFunction(func) ) 
            ret.modify_instr = true;
    }
    return ret;
}