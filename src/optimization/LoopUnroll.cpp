#include "optimization/LoopUnroll.hpp"
#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/SCEV.hpp"

#include <list>
using std::list;

void LoopUnroll::unrollCommonLoop(Loop *loop, LoopTrip trip, int time) {
    LOG_WARNING("Unrolling Common Loop")
    vector<BB*> blockToAdd = {};

    BB *header = loop->getHeader();
    vector<BB*> exits = loop->getExits();
    BB *latch = loop->getSingleLatch();
    LOG_ERROR("You should LoopSimplified Before Unrolling" , latch == nullptr)

    // 迭代的phi指令
    vector<Instruction*> phiSet = {};
    auto iter = header->getInstructions().begin();
    while((*iter)->isPhi()) {
        if((*iter)->getNumOperands() == 4 && (*iter)->getOperand(3) == latch)
            phiSet.push_back(*iter);
        iter++;
    }

    umap<Instruction*, vector<Instruction*> > phiValIter = {};
    for(Instruction *phi : phiSet) {
        vector<Value*> &ops = phi->getOperands();
        phiValIter[phi] = {};
        for(int i = 1; i < ops.size(); i += 2) {
            if(ops[i] == latch) {
                phiValIter[phi].push_back(dynamic_cast<Instruction*>(ops[i-1]));
            }
        }        
    }

    vector<BB*> entrys = { nullptr };
    vector<BB*> latchs = { latch };
    vector< vector<BB*> > exitings = { exits };
    vector<map<BB*, BB*> > BBMaps;
    vector<map<Instruction*, Instruction*> >  instMaps;

    BB *newEntry = nullptr;
    BB *newLatch = nullptr;
    vector<BB*> newExiting = {};
    map<BB*, BB*> newBBMap;
    map<Instruction*, Instruction*> newInstMap;

    for(int i = 1; i < time; i++) {
        loop->copyBody(newEntry, newLatch, newExiting, newBBMap, newInstMap);
        entrys.push_back(newEntry);
        latchs.push_back(newLatch);
        exitings.push_back(newExiting);
        BBMaps.push_back(newBBMap);
        instMaps.push_back(newInstMap);

        for(Instruction *phi : phiSet){
            phiValIter[phi].push_back( newInstMap[phiValIter[phi][0]] );
            
            // 替换复制的Body中出现的PhiVal
            for(auto [oldBB, newBB] : newBBMap) {
                if(!newBB)
                    continue;
                for(Instruction *inst : newBB->getInstructions()) {
                    vector<Value*> &opers = inst->getOperands();
                    for(int j = 0; j < opers.size(); j++) {
                        if(opers[j] == phi) {
                            inst->replaceOperand(j, phiValIter[phi][i-1]);
                        }
                    }
                }
            }
        }

        // 将复制的BB添加到loop的blocks里
        for(auto [oldBB, newBB] : newBBMap) {
            // BBMap可能因为查找BBMap[header]而导致插入{header，nullptr}
            if(newBB != nullptr)
                blockToAdd.push_back(newBB);
        }

        // 暂不考虑多exit（存在break）的情况
        // 用新复制的BB 替换掉 exit中phi指令原来的BB，包括相应的参数
        // 存在问题，exit及其succ中对firstIndval和firstIter没有替换！！
        // for(BB *exit : exits) {
        //     for(Instruction *inst : exit->getInstructions()) {
        //         if(!inst->isPhi())
        //             break;
        //         
        //         vector<Value*> &ops = inst->getOperands();
        //         for(int j = 1; j < ops.size(); j += 2) {
        //             Instruction *valIn = dynamic_cast<Instruction*>(ops[i-1]);
        //             BB *preBB = dynamic_cast<BB*>(ops[j]);
        //             if(newBBMap.find(preBB) != newBBMap.end()) {
        //                 ops[i-1]->removeUse(inst);
        //                 ops[i]->removeUse(inst);
        //                 
        //                 ops[i] = newBBMap[preBB];
        //                 if(valIn && newInstMap[valIn])
        //                     ops[i-1] = newInstMap[valIn];
        //             }
        //         }
        //     }
        //     for(BB *exitingBB : newExiting) {
        //         exit->addPreBasicBlock(exitingBB);
        //     }
        // }
    }
    loop->addBlocks(blockToAdd);
    loop->setSingleLatch(newBBMap[latch]);

    for(int i = 0; i < time-1; i++) {
        latchs[i]->removeSuccBasicBlock( header );
        latchs[i]->addSuccBasicBlock( entrys[i+1] );
        entrys[i+1]->removePreBasicBlock( header );
        entrys[i+1]->addPreBasicBlock( latchs[i] );
        
        latchs[i]->getTerminator()->replaceOperand(0, entrys[i+1]);
        
        // entry[i+1]中一定没有phi吗？
        LOG_ERROR("entry has phiInst!", entrys[i+1]->getInstructions().front()->isPhi())
    }
    header->removePreBasicBlock(latchs.front());
    header->addPreBasicBlock(latchs.back());

    // 更新新BB中的归纳变量
    for(Instruction *phi : phiSet) {
        for(int i = 1; i < time; i++) {
            Instruction *iterA = phiValIter[phi][i-1];
            Instruction *iterB = phiValIter[phi][i];

            vector<Value*> &ops = iterB->getOperands();
            for(int j = 0; j < ops.size(); j++) {
                if(ops[j] == phi) {
                    iterB->replaceOperand(j, iterA);
                }
            }
        }
    }

    // 更新header里面的phi
    for(Instruction *inst : header->getInstructions()) {
        if(!inst->isPhi())
            break;

        vector<Value*> &ops = inst->getOperands();
        for(int i = 1; i < ops.size(); i += 2) {
            if(ops[i] == latch) {
                inst->replaceOperand(i-1, newInstMap[dynamic_cast<Instruction*>(inst->getOperand(i-1))]);
                inst->replaceOperand(i,  newBBMap[dynamic_cast<BB*>(inst->getOperand(i))]);
            }
        }
    }
}

void LoopUnroll::unrollPartialLoop(Loop *loop, LoopTrip trip, int time) {
    LOG_WARNING("Unroll Partial Loop")
    Loop *newLoop = loop->copyLoop();
    unrollCommonLoop(loop, trip, time);
    
    // 更新原Loop
    // 目前仅考虑常数
    LoopCond *oldCond = loop->getConds()[0];
    
    int mod = abs(time * trip.iter);
    int newEnd = (trip.end / mod) * mod;
    if(trip.end > 0 && trip.iter < 0)
        newEnd += mod;
    else if(trip.end < 0 && trip.iter > 0)
        newEnd -= mod;
    
    if(oldCond->op == LoopCond::ge || oldCond->op == LoopCond::le)
        newEnd += -trip.iter;
    
    LoopCond *newCond = new LoopCond{oldCond->lhs, oldCond->op, dynamic_cast<Value*>(ConstantInt::get(newEnd))};
    loop->replaceCond(newCond);

    // 插入新Loop
    BB *oldExit = loop->getExits()[0];
    BB *newPreheader = newLoop->getPreheader();
    newPreheader->getPreBasicBlocks().clear();
    vector<BB*> oldExitPreBBToDel = {};
    list<BB*> oldExitPreBB = oldExit->getPreBasicBlocks();
    for(BB *pre : oldExitPreBB) {
        if(loop->contain(pre)) {
            Instruction *term = pre->getTerminator();
            vector<Value*> ops = term->getOperands();
            for(int i = 0; i < ops.size(); i++) {
                if(ops[i] == oldExit) {
                    term->replaceOperand(i, newPreheader);
                    pre->removeSuccBasicBlock(oldExit);
                    // 逻辑上不正确，但bug却没了？？
                    // oldExit->removePreBasicBlock(pre); 
                    pre->addSuccBasicBlock(newPreheader);
                    newPreheader->addPreBasicBlock(pre);
                    break;
                }
            }
        }
    }
    loop->getExits().clear();
    loop->getExits().push_back(newLoop->getPreheader());

    // 将循环外使用到的旧循环变量换为新循环变量
    umap<PhiInst*, PhiInst*> phiMap = {};
    list<Instruction*> instsO = loop->getHeader()->getInstructions();
    list<Instruction*> instsN = newLoop->getHeader()->getInstructions();
    auto iterO = instsO.begin();
    auto iterN = instsN.begin();
    for(; iterO != instsO.end(); iterO++, iterN++) {
        if(!(*iterO)->isPhi())
            break;
        phiMap.insert({dynamic_cast<PhiInst*>(*iterO), dynamic_cast<PhiInst*>(*iterN)});
    }

    for(auto [oldPhi, newPhi] : phiMap) {
        list<Use> useList = oldPhi->getUseList();
        for(Use u : useList) {
            if(!dynamic_cast<Instruction*>(u.val_))
                continue;
            
            Instruction *useInst = dynamic_cast<Instruction*>(u.val_);
            BB *useBB = useInst->getParent();
            if(!loop->contain(useBB) && !newLoop->contain(useBB)) {
                useInst->replaceOperand(u.arg_no_, newPhi);
            }
        }
    }
}


// 循环次数小于DIRECT_UNROLLING_TIME的循环
// 删除header和loop
//    preheader->header->start->...->end->exit
// -> preheader->start->...->end->exit
void LoopUnroll::unrolEntirelLoop(Loop *loop, LoopTrip trip) {
    unrollCommonLoop(loop, trip, trip.step);

    BB *blockStart;
    BB *blockEnd = loop->getSingleLatch();
    BB *preheader = loop->getPreheader();
    BB *loopExit = loop->getExits()[0];
    for(BB *bb : loop->getHeader()->getSuccBasicBlocks()) {
        if(bb != loop->getExits()[0]) {
            blockStart = bb;
            break;
        }
    }

    preheader->getTerminator()->replaceOperand(0, blockStart);
    preheader->getSuccBasicBlocks().clear();
    preheader->getSuccBasicBlocks().push_back(blockStart);
    blockStart->getPreBasicBlocks().clear();
    blockStart->getPreBasicBlocks().push_back(preheader);

    blockEnd->getTerminator()->replaceOperand(0, loopExit);
    blockEnd->getSuccBasicBlocks().clear();
    blockEnd->getSuccBasicBlocks().push_back(loopExit);
    loopExit->getPreBasicBlocks().clear();
    loopExit->getPreBasicBlocks().push_back(blockEnd);

    // 对phi指令进行拆分
    for(Instruction *phiInst : loop->getHeader()->getInstructions()) {
        if(!phiInst->isPhi())
            break;
        // use in loop
        Value *inVal = phiInst->getOperand(0);
        // use out of loop
        Instruction *iterI = dynamic_cast<Instruction*>(phiInst->getOperand(2));
        if(!iterI) continue;
        Value *outVal = phiInst->getOperand(2);
        
        list<Use> useList = phiInst->getUseList();
        for(Use u : useList) {
            Instruction *instU = dynamic_cast<Instruction*>(u.val_);
            if(!instU)  continue;
            BB *parent = instU->getParent();
            if(loop->contain(parent)) {
                instU->replaceOperand(u.arg_no_, inVal);
            } else {
                instU->replaceOperand(u.arg_no_, outVal);
            }
        }
    }
    for(Instruction *inst : loop->getHeader()->getInstructions()) {
        inst->removeUseOfOps();
    }
    loop->getHeader()->eraseFromParent();
    info_man_->getInfo<LoopInfo>()->removeLoop(loop);
}

// 只合并单块(必为latch、唯一入口为header、terminator是无条件跳转)
void LoopUnroll::unrollEntirelLoopInOneBB(Loop *loop, LoopTrip trip) {
    LOG_WARNING("Unroll Entirel Loop In One BB")
    BB *loopBlock = loop->getSingleLatch();
    // todo: remove vector
    umap<Instruction*, vector<Instruction*> > iterValMap = {};
    umap<Instruction*, Instruction*> instMap = {};
    vector<Instruction*> delIters = {};

    for(Instruction *inst : loop->getHeader()->getInstructions()) {
        if(!inst->isPhi())
            break;
        
        // 仅用于计数比较，完全展开时可将其删除
        if(inst->getUseList().size() == 2) {
            Instruction *phiIter1 = dynamic_cast<Instruction*>(inst->getUseList().front().val_);
            Instruction *phiIter2 = dynamic_cast<Instruction*>(inst->getUseList().back().val_);
            if(phiIter1 && phiIter2) {
                if(( phiIter1->isAdd() || phiIter1->isSub() ) && phiIter2->isCmp()) {
                    // 迭代的指令也只用于更新phi
                    if(phiIter1->getUseList().size() == 1 && phiIter1->getUseList().front().val_ == inst) {
                        delIters.push_back(phiIter1);
                        continue;
                    }
                } else if(( phiIter2->isAdd() || phiIter2->isSub() ) && phiIter1->isCmp()) {
                    if(phiIter2->getUseList().size() == 1 && phiIter2->getUseList().front().val_ == inst) {
                        delIters.push_back(phiIter2);
                        continue;
                    }
                }
            }
        }
        
        if(inst->getNumOperands() == 4 && inst->getOperand(3) == loop->getSingleLatch()) {
            Instruction *iter = dynamic_cast<Instruction*>(inst->getOperand(2));
            iterValMap.insert({iter, {inst, iter}});
        }
    }
    
    list<Instruction*> &blockInsts = loopBlock->getInstructions();
    Instruction *bodyTerm = blockInsts.back();
    blockInsts.pop_back();

    list<Instruction*> instsToCopy = list<Instruction*>(blockInsts.begin(), blockInsts.end());
    for(Instruction *instToDel : delIters) {
        blockInsts.remove(instToDel);
        instsToCopy.remove(instToDel);
    }

    for(int i = 1; i < trip.step; i++) {
        instMap.clear();
        for(Instruction *copyInst : instsToCopy) {
            Instruction *newInst = copyInst->copyInst(loopBlock);
            instMap[copyInst] = newInst;
            
            vector<Value*> &copyOps = newInst->getOperands(); 
            for(int j = 0; j < copyOps.size(); j++) {
                Instruction *opI = dynamic_cast<Instruction*>(copyOps[j]);
                if(opI && instMap.count(opI))
                    newInst->replaceOperand(j, instMap[opI]);
            }

            // 无迭代变量，不更新
            if(iterValMap.size() == 0)
                continue;

            // new iter
            if(iterValMap.count(copyInst))
                iterValMap[copyInst].push_back(newInst);

            // 对于出现的phi，替换为对应的迭代变量
            vector<Value*> &iterOps = newInst->getOperands();
            for(auto [fIter, iters] : iterValMap){
                for(int j = 0; j < iterOps.size(); j++) {
                    if(iterOps[j] == iters[0]) {
                        newInst->replaceOperand(j, iters[i]);
                    }
                }
            }
        }
    }

    // 删除header和loop
    //    preheader->header->loopBlock->exit
    // -> preheader->loopBlock->exit
    BB *preheader = loop->getPreheader();
    BB *loopExit = loop->getExits()[0];

    preheader->getTerminator()->replaceOperand(0, loopBlock);
    preheader->getSuccBasicBlocks().clear();
    preheader->getSuccBasicBlocks().push_back(loopBlock);
    loopBlock->getPreBasicBlocks().clear();
    loopBlock->getPreBasicBlocks().push_back(preheader);

    blockInsts.push_back(bodyTerm);
    loopBlock->getTerminator()->replaceOperand(0, loopExit);
    loopBlock->getSuccBasicBlocks().clear();
    loopBlock->getSuccBasicBlocks().push_back(loopExit);
    loopExit->getPreBasicBlocks().clear();
    loopExit->getPreBasicBlocks().push_back(loopBlock);

    // 对phi指令进行拆分
    for(Instruction *phiInst : loop->getHeader()->getInstructions()) {
        if(!phiInst->isPhi())
            break;
        // use in loop
        Value *inVal = phiInst->getOperand(0);
        // use out of loop
        Instruction *iterI = dynamic_cast<Instruction*>(phiInst->getOperand(2));
        if(!iterI || !iterValMap.count(iterI)) continue;
        Value *outVal = iterValMap[iterI][trip.step];
        
        list<Use> useList = phiInst->getUseList();
        for(Use u : useList) {
            Instruction *instU = dynamic_cast<Instruction*>(u.val_);
            if(!instU)  continue;
            BB *parent = instU->getParent();
            if(loop->contain(parent)) {
                instU->replaceOperand(u.arg_no_, inVal);
            } else {
                instU->replaceOperand(u.arg_no_, outVal);
            }
        }
    }
    for(Instruction *inst : loop->getHeader()->getInstructions()) {
        inst->removeUseOfOps();
    }
    loop->getHeader()->eraseFromParent();
    info_man_->getInfo<LoopInfo>()->removeLoop(loop);
}

// 删除loop
//    preheader->loopBlocks->exit
// -> preheader->exit
void LoopUnroll::removeLoop(Loop *loop) {
    BB *preheader = loop->getPreheader();
    BB *loopExit = loop->getExits()[0];

    preheader->getTerminator()->replaceOperand(0, loopExit);
    preheader->getSuccBasicBlocks().clear();
    preheader->getSuccBasicBlocks().push_back(loopExit);
    loopExit->getPreBasicBlocks().clear();
    loopExit->getPreBasicBlocks().push_back(preheader);

    // 对phi指令进行拆分
    for(Instruction *phiInst : loop->getHeader()->getInstructions()) {
        if(!phiInst->isPhi())
            break;
        Value *inVal = phiInst->getOperand(0);
        phiInst->replaceAllUseWith(inVal);
    }

    for(BB *block : loop->getBlocks()) {
        for(Instruction *inst : block->getInstructions()) {
            inst->removeUseOfOps();
        }
        block->eraseFromParent();
    }
    info_man_->getInfo<LoopInfo>()->removeLoop(loop);
}

void LoopUnroll::visitLoop(Loop *loop) {
    if(UNROLLING_TIME == 1)
        return;

    // for(Loop *inner : loop->getInners()) {
    //     visitLoop(inner);
    // }

    SCEV *scev = info_man_->getInfo<SCEV>();
    LoopTrip trip = loop->computeTrip(scev);
    LOG_WARNING(trip.print())

    // 暂不考虑break和子循环
    if(loop->getExits().size() > 1 || 
       loop->getInners().size() > 0)
        return;

    if(trip.step < 0) {
        return;
    } else if(trip.step == 0) {
        removeLoop(loop);
    } else if(loop->getBlocks().size() == 2 && 
             (loop->getSingleLatch()->getInstructions().size()-1) * trip.step < DIRECT_UNROLLING_SIZE) { 
         unrollEntirelLoopInOneBB(loop, trip);
    } else if(trip.step < DIRECT_UNROLLING_TIME) {
        unrolEntirelLoop(loop, trip);
    } else if(trip.step % UNROLLING_TIME == 0) {
        unrollCommonLoop(loop, trip, UNROLLING_TIME);
    } else {
        unrollPartialLoop(loop, trip, UNROLLING_TIME);
    }
}

Modify LoopUnroll::runOnFunc(Function* func) {
    Modify mod{};
    vector<Loop*> loops = info_man_->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops) {
        // 暂不考虑多重循环
        if(loop->getInners().size() == 0)
            visitLoop(loop);
    }

    mod.modify_bb = true;
    return mod;
}

