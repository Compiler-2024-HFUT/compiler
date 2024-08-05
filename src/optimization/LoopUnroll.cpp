#include "optimization/LoopUnroll.hpp"
#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/SCEV.hpp"

#include <list>
using std::list;

void LoopUnroll::unrollCommonLoop(Loop *loop, LoopTrip trip) {
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

    for(int i = 1; i < UNROLLING_TIME; i++) {
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

        // 用新复制的BB 替换掉 exit中phi指令原来的BB，包括相应的参数
        // 存在问题，exit及其succ中对firstIndval和firstIter没有替换！！
        for(BB *exit : exits) {
            for(Instruction *inst : exit->getInstructions()) {
                if(!inst->isPhi())
                    break;
                
                vector<Value*> &ops = inst->getOperands();
                for(int j = 1; j < ops.size(); j += 2) {
                    Instruction *valIn = dynamic_cast<Instruction*>(ops[i-1]);
                    BB *preBB = dynamic_cast<BB*>(ops[j]);
                    if(newBBMap.find(preBB) != newBBMap.end()) {
                        ops[i-1]->removeUse(inst);
                        ops[i]->removeUse(inst);
                        
                        ops[i] = newBBMap[preBB];
                        if(valIn && newInstMap[valIn])
                            ops[i-1] = newInstMap[valIn];
                    }
                }
            }

            for(BB *exitingBB : newExiting) {
                exit->addPreBasicBlock(exitingBB);
            }
        }
    }
    loop->addBlocks(blockToAdd);

    for(int i = 0; i < UNROLLING_TIME-1; i++) {
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
        for(int i = 1; i < UNROLLING_TIME; i++) {
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

void LoopUnroll::unrollPartialLoop(Loop *loop, LoopTrip trip) {
    LOG_WARNING("Unroll Partial Loop")
    Loop *newLoop = loop->copyLoop();
    unrollCommonLoop(loop, trip);
    
    // 更新原Loop
    // 目前仅考虑常数
    LoopCond *oldCond = loop->getConds()[0];
    
    int mod = abs(UNROLLING_TIME * trip.iter);
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
    for(BB *pre : oldExit->getPreBasicBlocks()) {
        if(loop->contain(pre)) {
            Instruction *term = pre->getTerminator();
            vector<Value*> ops = term->getOperands();
            for(int i = 0; i < ops.size(); i++) {
                if(ops[i] == oldExit) {
                    term->replaceOperand(i, newPreheader);
                    pre->removeSuccBasicBlock(oldExit);
                    oldExitPreBBToDel.push_back(pre);
                    pre->addSuccBasicBlock(newPreheader);
                    newPreheader->addPreBasicBlock(pre);
                    break;
                }
            }
        }
    }
    for(BB *pre : oldExitPreBBToDel)
        oldExit->removePreBasicBlock(pre);
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
        for(BB *bb : loop->getFunction()->getBasicBlocks()) {
            if(loop->contain(bb) || newLoop->contain(bb))
                continue;
            for(Instruction *inst : bb->getInstructions()){
                vector<Value*> ops = inst->getOperands();
                for(int i = 0; i < ops.size(); i++) {
                    if(ops[i] == oldPhi) {
                        inst->replaceOperand(i, newPhi);
                    }
                }
            }
        }
    }

    // phi的use好像有问题
    // for(auto [oldPhi, newPhi] : phiMap) {
    //     for(Use u : oldPhi->getUseList()) {
    //         if(!dynamic_cast<Instruction*>(u.val_))
    //             continue;
    //         
    //         Instruction *useInst = dynamic_cast<Instruction*>(u.val_);
    //         BB *useBB = useInst->getParent();
    //         if(!loop->contain(useBB) && !newLoop->contain(useBB)) {
    //             useInst->replaceOperand(u.arg_no_, newPhi);
    //         }
    //     }
    // }
}

// 只合并单块(必为latch、唯一入口为header、terminator是无条件跳转)
// 且块内指令数(不包括br)小于DIRECT_UNROLLING_SIZE
void LoopUnroll::unrolEntirelLoop(Loop *loop, LoopTrip trip) {
    
    return;
}

void LoopUnroll::removeLoop(Loop *loop) {
    return;
}

void LoopUnroll::visitLoop(Loop *loop) {
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
        return;
    // } else if(trip.step < DIRECT_UNROLLING_TIME) {
    //     unrollCommonLoop(loop, trip);
    } else if(trip.step % UNROLLING_TIME == 0) {
        unrollCommonLoop(loop, trip);
    } else {
        unrollPartialLoop(loop, trip);
    }

    // for(Loop *inner : loop->getInners()) {
    //     visitLoop(inner);
    // }
}

void LoopUnroll::runOnFunc(Function* func) {
    vector<Loop*> loops = info_man_->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops) {
        // 暂不考虑多重循环
        if(loop->getInners().size() == 0)
            visitLoop(loop);
    }
}

