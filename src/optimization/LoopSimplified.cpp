#include "optimization/LoopSimplified.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"

#include <algorithm>
using std::find;
using std::remove;

// fix later
// exit可无条件跳转的最终目标
static BB *getExitDest(BB *bb) {
    // 返回表示exit可以一直无条件跳转至ret_label
    if(bb->getTerminator()->isRet())
        return bb;
    
    // 无条件跳转 
    if(bb->getTerminator()->getNumOperands() == 1 && bb->getInstructions().front() == bb->getTerminator() ) 
        return getExitDest( dynamic_cast<BB*>(bb->getTerminator()->getOperand(0)) );
    else    
        return bb;
}

// move to BBUtils.cpp later
// 在to和froms之间插入inserted后, 更新to的phi 以及 为inserted添加phi
static void updatePhiAfterInsert(vector<BB*> &froms, BB *inserted, BB *to) {
    list<Instruction*> &insts = to->getInstructions();
    vector<Instruction*> instsToRemove = {};
    for(Instruction* inst : insts) {
        if(!inst->isPhi())
            break;

        PhiInst *newPhi;

        // phi指令的incoming都包含在froms里面(经过splitBlock后，to的pre只有inserted和非froms中的pre)，
        // 将原来的phi复制到inserted，同时替换to对原phi的use
        if(to->getPreBasicBlocks().size() == 1){
            newPhi = dynamic_cast<PhiInst*>( inst->copyInst(inserted) );
            inserted->getInstructions().push_front(newPhi);

            newPhi->replaceAllUseWith(newPhi);
            instsToRemove.push_back(inst);
        // phi指令的incoming都包含在froms里面，将原来的phi复制到inserted，同时替换to对原phi的use
        }else if(froms.size() == 1) {
            vector<Value*> &ops = inst->getOperands();
            for(Value* &op : ops) {
                if( op == froms[0] ) {
                    op = inserted;
                }
            }
        // phi指令部分在froms里
        } else {
            PhiInst *oldPhi = dynamic_cast<PhiInst*>(inst);

            // 创建inserted中的phi
            newPhi = PhiInst::createPhi(inst->getType(), inserted);
            inserted->getInstructions().push_front(newPhi);

            // 移动BB到phi
            vector<Value*> ops = oldPhi->getOperands();
            for(int i = 1; i < ops.size(); i += 2) {
                if( find( froms.begin(), froms.end(), ops[i] ) != froms.end() ) {
                    newPhi->addPhiPairOperand(ops[i-1], ops[i]);
                    oldPhi->removePhiPairOperand(ops[i]);
                }
            }
            
            // 添加newPhi
            oldPhi->addPhiPairOperand(newPhi, inserted);
        }
    }

    for(Instruction *inst : instsToRemove) {
        insts.remove(inst);
    }
}

// move to BBUtils.cpp later
// 将block分成两个，
// 指定preds跳转至返回的新BB，新BB再跳转至block，
// block其它的preBB仍然跳至block
static BB *splitBlockByPreBB(BB *block, vector<BB*> &preds) {
    BB *newBB = BasicBlock::create("", block->getParent());
    BranchInst::createBr(block, newBB);

    for(BB *pre : preds) {
        Instruction *brInst = pre->getTerminator();
        LOG_ERROR("pred's terminator isn't a br", !brInst)

        // 修改pre跳转至newBB
        vector<Value*> &ops = brInst->getOperands();
        for(int i = 0; i < ops.size(); i++) {
            if(ops[i] == block) {
                brInst->setOperand(i, newBB);
            }
        }
        pre->removeSuccBasicBlock(block);
        pre->addSuccBasicBlock(newBB);
        
        block->removePreBasicBlock(pre);
    }

    // update phiInst, split them for newBB from block
    updatePhiAfterInsert(preds, newBB, block);

    return newBB;
}

// // 仅保留唯一的公共exit，并将中间跳转的exit删除，返回合并后的BB
// BB *LoopSimplified::mergeExits(Loop *loop) {
//     if(loop->getSingleExit() != nullptr)
//         return loop->getSingleExit();
//     if(loop->getExits().size()==1)
//         return loop->getExits()[0];
//     
//     vector<BB*> &exits = loop->getExits();
//     BB *singleExit = getExitDest(exits[0]);
//     list<BB*> funcBlocks = loop->getFunction()->getBasicBlocks();
// 
//     for(BB *exit : exits) {
//         if(exit == singleExit)
//             continue;
// 
//         BB *preBB = exit->getPreBasicBlocks().front();
//         if(exit->getPreBasicBlocks().size() > 1)
//             LOG_WARNING("Why splited exit has more than one preBB?")
//         exit->replaceAllUseWith(singleExit);
//         for(Value* &val : singleExit->getTerminator()->getOperands()) {
//             if(val = singleExit) {
//                 val = preBB;
//             }
//         }
//         remove(funcBlocks.begin(), funcBlocks.end(), exit);
//     }
//     exits.clear();
// 
//     return singleExit;
// }

BB *LoopSimplified::insertUniqueBackedge(Loop *loop) {
    BB *newLatch = BasicBlock::create("", loop->getFunction());
    BB *preheader = loop->getPreheader(), *header = loop->getHeader();
    vector<BB*> &latchs = loop->getLatchs();
    BranchInst *brInst = BranchInst::createBr(header, newLatch);

    // Latchs中基本块 都跳转至 newLatch
    for(BB *latch : loop->getLatchs()) {
        Instruction *toHeader = latch->getTerminator();
        LOG_ERROR("latch's br is a cond br!", toHeader->getNumOperands() == 3)

        toHeader->getOperands()[0] = newLatch;
        latch->removeSuccBasicBlock(header);
        latch->addSuccBasicBlock(newLatch);

        header->removePreBasicBlock(latch);
    }
    
    // 更新phi
    updatePhiAfterInsert(latchs, newLatch, header);

    return newLatch;
}

BB *LoopSimplified::insertPreheader(Loop *loop) {
    BB* header = loop->getHeader();
    BB *preheader;

    vector<BB*> preds = {};
    for(BB *pre : header->getPreBasicBlocks()) {
        // skip latch
        if(loop->contain(pre))
            continue;
        preds.push_back(pre);
    }

    preheader = splitBlockByPreBB(header, preds);
    return preheader;
}

void LoopSimplified::processLoop(Loop *loop) {
    BB *preheader;
    BB *singleLatch;
    vector<BB*> &exits = loop->getExits();

    // insert preheader
    if(!loop->getPreheader()) {
        preheader = insertPreheader(loop);
        loop->setPreheader(preheader);
    }

    bool shouldSplitExit;
    vector<BB*> preLoopBB;
    for(BB* &exit : exits) {
        shouldSplitExit = false;
        preLoopBB.clear();
        for(BB *pre : exit->getPreBasicBlocks()) {
            if(loop->contain(pre)) {
                preLoopBB.push_back(pre);
            } else {
                shouldSplitExit = true;
            }
        }
        if(shouldSplitExit) {
            exit = splitBlockByPreBB(exit, preLoopBB);
        }
    }

    // If this loop has multiple exits and the exits all go to the same block, 
    // attempt to merge the exits.
    bool isExitSingle = true;
    for(int i=1; i<exits.size(); i++) {
        if(getExitDest(exits[0]) != getExitDest(exits[i])) {
            isExitSingle = false;
            break;
        }
    }

    // // only one exit or exit can be merge
    // if(isExitSingle) {
    //     BB *newExit = mergeExits(loop);
    //     loop->setSingleExit(newExit);
    // }

    // more than one Latch, combine as one latch
    if(!loop->getSingleLatch()) {
        if(loop->getLatchs().size() == 1) {
            singleLatch = loop->getLatchs()[0];      
        } else {
            singleLatch = insertUniqueBackedge(loop);
        }

        loop->setSingleLatch(singleLatch);
    }
}

void LoopSimplified::runOnFunc(Function* func) {
    vector<Loop*> loops = info_man_->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops) {
        if(loop->isSimplifiedForm())
            continue;
        processLoop(loop);
        loop->setSimplified();
    }
}