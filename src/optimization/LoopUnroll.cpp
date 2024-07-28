#include "optimization/LoopUnroll.hpp"
#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/SCEV.hpp"

void LoopUnroll::unrollCommonLoop(Loop *loop, LoopTrip trip) {
    vector<BB*> blockToAdd = {};

    BB *header = loop->getHeader();
    vector<BB*> exits = loop->getExits();
    BB *latch = loop->getSingleLatch();
    LOG_ERROR("You should LoopSimplified Before Unrolling" , latch == nullptr)

    auto iter = header->getInstructions().begin();
    Instruction *firstIndVal = *iter;
    // 暂时不考虑迭代的非基础归纳变量
    iter++;
    if((*iter)->isPhi()) {
        return;
    }

    vector<Value*> &ops = firstIndVal->getOperands();
    vector<Instruction*> indValsIter = {};  // 指令 i = i + iter 的集合
    for(int i = 1; i < ops.size(); i += 2) {
        if(ops[i] == latch) {
            indValsIter.push_back(dynamic_cast<Instruction*>(ops[i-1]));
        }
    }

    vector<BB*> entrys = { nullptr };
    vector<BB*> latchs = { latch };
    vector< vector<BB*> > exitings = { exits };
    map<BB*, BB*> BBMap;
    map<Instruction*, Instruction*> instMap;

    for(int i = 1; i < UNROLLING_TIME; i++) {
        BB *newEntry = nullptr;
        BB *newLatch = nullptr;
        vector<BB*> newExiting = {};
        
        loop->copyBody(newEntry, newLatch, newExiting, BBMap, instMap);
        entrys.push_back(newEntry);
        latchs.push_back(newLatch);
        exitings.push_back(newExiting);

        indValsIter.push_back( instMap[indValsIter[0]] );

        // 替换复制的Body中出现的IndVal
        for(auto [oldBB, newBB] : BBMap) {
            if(!newBB)
                continue;
            for(Instruction *inst : newBB->getInstructions()) {
                vector<Value*> &opers = inst->getOperands();
                for(int j = 0; j < opers.size(); j++) {
                    if(opers[j] == firstIndVal) {
                        inst->replaceOperand(j, indValsIter[i-1]);
                    }
                }
            }
        }

        // 将复制的BB添加到loop的blocks里
        for(auto [oldBB, newBB] : BBMap) {
            // BBMap可能因为查找BBMap[header]而导致插入{header，nullptr}
            if(newBB != nullptr)
                blockToAdd.push_back(newBB);
        }

        // 用新复制的BB 替换掉 exit中phi指令原来的BB，包括相应的参数
        // 待测试。。。
        for(BB *exit : exits) {
            for(Instruction *inst : exit->getInstructions()) {
                if(!inst->isPhi())
                    break;
                
                vector<Value*> &ops = inst->getOperands();
                for(int j = 1; j < ops.size(); j += 2) {
                    Instruction *valIn = dynamic_cast<Instruction*>(ops[i-1]);
                    BB *preBB = dynamic_cast<BB*>(ops[j]);
                    if(BBMap.find(preBB) != BBMap.end()) {
                        ops[i-1]->removeUse(inst);
                        ops[i]->removeUse(inst);
                        
                        ops[i] = BBMap[preBB];
                        if(valIn && instMap[valIn])
                            ops[i-1] = instMap[valIn];
                    }
                }
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
    for(int i = 1; i < UNROLLING_TIME; i++) {
        Instruction *iterA = indValsIter[i-1];
        Instruction *iterB = indValsIter[i];

        vector<Value*> &ops = iterB->getOperands();
        for(int j = 0; j < ops.size(); j++) {
            if(ops[j] == firstIndVal) {
                iterB->replaceOperand(j, iterA);
            }
        }
    }

    // 更新header里面的phi
    vector<Value*> &opst = firstIndVal->getOperands();
    for(int i = 0; i < opst.size(); i += 2) {
        if(opst[i] == indValsIter.front()) {
            firstIndVal->replaceOperand(i, indValsIter.back());
            firstIndVal->replaceOperand(i+1, latchs.back());
        }
    }
}

void LoopUnroll::unrollPartialLoop(Loop *loop, LoopTrip trip) {
    return;
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

    if(trip.step < 0) {
        return;
    } else if(trip.step == 0) {
        removeLoop(loop);
        return;
    } else if(trip.step < DIRECT_UNROLLING_TIME) {
        unrollCommonLoop(loop, trip);
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

