#include "analysis/InfoManager.hpp"
#include "analysis/LoopInfo.hpp"
#include "analysis/Dominators.hpp"
#include "analysis/SCEV.hpp"
#include "utils/Logger.hpp"

#include <unordered_map>
#include <list>
#include <algorithm>
using std::copy;
using std::list;

template<typename key, typename value>
using umap = std::unordered_map<key, value>;

void LoopInfo::analyseOnFunc(Function *func_) {
    DFS_CFG(func_->getEntryBlock(), 0);
    findRetreatEdges(func_);
    findBackEdges();
    findLoops(func_);
    combineLoops(func_);
    producedNestLoops(func_);
    findExits(func_);
}

// 深度优先遍历CFG，并标注层次，EntryBlock的level为0
void LoopInfo::DFS_CFG( BB* rootBB, int level ) {
    static umap<BB*, bool> isVisited;
    if(level == 0) {
        isVisited.clear();
        for(BB *bb : rootBB->getParent()->getBasicBlocks()) {
            isVisited[bb] = false;
        }
    }

    BBlevel[rootBB] = level;
    isVisited[rootBB] = true;

    for(BB* bb : rootBB->getSuccBasicBlocks()) {
        if(!isVisited[bb]) {
            DFS_CFG(bb, level+1);
        }
    }
}

// **反向**深度优先遍历CFG，从tail开始，到head或entry结束
// 遍历过程中遇到的BB既是Loop的BB
// 遍历过程中，使用的参数head、Loop不变，但参数tail逐渐靠近head
void LoopInfo::DFS_CFG( BB* tail, BB* head, Loop *loop, Function *func_ ) {
    static umap<BB*, bool> isVisited;
    // 第一个exit必然是
    if(loop->getLatchs()[0] == tail) {
        isVisited.clear();
        for(BB *bb : head->getParent()->getBasicBlocks()) {
            isVisited[bb] = false;
        }
    }

    isVisited[tail] = true;
    loop->addBlock(tail);

    for(BB* pre : tail->getPreBasicBlocks()) {
        // pre未被访问，并且pre不是head或entry
        if(!isVisited[pre] && pre != head && pre != func_->getEntryBlock() ) {
            DFS_CFG(pre, head, loop, func_);
        }
    }
}

// 如果某个BBtail的level小于其后继节点BBhead，那么（BBtail，BBhead）构成一条RetreatEdge
// 特殊地，如果tail == head，它们也组成一条RetreatEdge
void LoopInfo::findRetreatEdges(Function *func_) {
    retreatEdges.clear();
    for(BB* tail : func_->getBasicBlocks()) {
        for(BB* head : tail->getSuccBasicBlocks()) {
            if(BBlevel[tail] > BBlevel[head] || tail == head) {
                retreatEdges.push_back( {tail, head} );
            }
        }
    }
}

// Back edge: t->h (RetreatEdge) and h dominates t
void LoopInfo::findBackEdges() {
    backEdges.clear();
    Dominators *dom = infoManager->getInfo<Dominators>();

    for(pair<BB*, BB*> edge : retreatEdges) {
        auto &domSet = dom->getDomSet(edge.second);
        if( domSet.find(edge.first) != domSet.end() ) {
            backEdges.push_back(edge);
        }
    }
}

void LoopInfo::findLoops(Function *func_) {
    for(auto be : backEdges) {
        Loop *tmp = new Loop(be.first, be.second);
        DFS_CFG( be.first, be.second, tmp, func_ );
        tmp->addBlock(be.second);
        loops[func_].push_back(tmp);
    }
}

// 只查找最外层的Loop的exit
void LoopInfo::findExits(Function *func_) {
    for(Loop *loop : loops[func_]) {
        loop->findExits();
    }
}

// 同header的loop合并
void LoopInfo::combineLoops(Function *func_) {
    // loops按照header排序，使得同样header的loop相邻
    sort(loops[func_].begin(), loops[func_].begin(), [](Loop *a, Loop* b){ return a->getHeader() < b->getHeader(); });

    auto iter1 = loops[func_].begin(), iter2 = loops[func_].begin();
    while(iter1 != loops[func_].end()) { 
        iter2++;
        // 如果iter1是最后一个元素，iter2为end
        while(iter1 != loops[func_].end() && iter2 != loops[func_].end() && (*iter1)->getHeader() == (*iter2)->getHeader()) {
            // 合并同header的2个loop
            (*iter1)->addLatch( (*iter2)->getLatchs()[0] );
            for(auto bb : (*iter2)->getBlocks()) {
                (*iter1)->addBlock(bb);
            }
            // 下个元素
            iter2 = loops[func_].erase(iter2);
        }
        // 下一个header或end结束
        iter1 = iter2;
    }
}

void LoopInfo::producedNestLoops(Function *func_) {
    bool erased = false;

    // loops按照包含的基本块数量排序，少的在前
    sort(loops[func_].begin(), loops[func_].end(), [](Loop *a, Loop *b) { return (a->getBlocks().size() < b->getBlocks().size()); });

    // 判断a为b的真子集
    auto isSubset = [](const uset<BB*> &a, const uset<BB*> &b) {
        for (const auto& element : a) {
            if (b.find(element) == b.end()) {
                return false;
            }
        }
        return a.size() < b.size();
    };

    auto iter1 = loops[func_].begin(), iter2 = loops[func_].begin();
    while(iter1 != loops[func_].end()) {
        iter2 = iter1 + 1;
        while(iter2 != loops[func_].end()) {
            // loop1 header 属于 loop2 且 loop1的blocks属于loop2的blocks
            if( (*iter2)->getBlocks().count( (*iter1)->getHeader() ) && isSubset( (*iter1)->getBlocks(), (*iter2)->getBlocks() ) ) {

                (*iter2)->addInner( (*iter1) );
                (*iter1)->setOuter( (*iter2) );
                (*iter1)->updateDepth();
                
                // 标记移除已经被识别为子循环的iter1
                erased = true;
                break;
            }
            ++iter2;
        }

        // iter1是子循环，移除，iter1是下一个；不是，iter1移动到下一个
        if(erased && iter2 != loops[func_].end()) {
            iter1 = loops[func_].erase(iter1);
        } else {
            ++iter1;
        }
    }
}

string LoopInfo::print() {
    string loopinfo = "";
    module_->print();
    for(Function *f : module_->getFunctions()) {
        if(f->getBasicBlocks().size() == 0 || loops[f].size() == 0)
            continue;
        
        if(f->getBasicBlocks().size() != 0) {
            loopinfo += STRING_RED("Loops of " + f->getName() + " with LoopSize " + STRING_NUM(loops[f].size()) +":\n");
            for(auto loop : loops[f]) {
                loopinfo += STRING(loop->print() + "\n");
            }
        }
    }
    return loopinfo;
}

bool Loop::computeConds() {
    conditions.clear();

    list<Instruction*> &headerInsts = header->getInstructions();
    auto iter = headerInsts.end();
    iter--; iter--;
    LoopCond *cond = LoopCond::createLoopCond( *iter );
    if(!cond)   return false;
    conditions.push_back(cond);

    Instruction *br = dynamic_cast<BranchInst*>(headerInsts.back());
    LOG_ERROR("should be a brinst, or bb isn't a header?", !br)
        
    BB  *next = dynamic_cast<BB*>( br->getOperand(1) ),     // if_true
        *exit = dynamic_cast<BB*>( br->getOperand(2) );     // if_false

    if(isCondBlock(next)) {
        conditions.clear();
        return false;
    }
    return true;    

    /* 暂时只考虑单条件的情况
    while(isCondBlock(next)) {
        // ...
    }
    */
}

void Loop::setCondsAndUpdateBlocks(vector<LoopCond*> conds) {
    // ...
}

// 需要注意：
// 复制的body没有添加到blocks里面
// entry不在它潜在pre的succBBs里面
// latch也不在它潜在succ的preBBs里面
void Loop::copyBody(BB* &entry, BB* &singleLatch, vector<BB*> &exiting, map<BB*, BB*> &BBMap, map<Instruction*, Instruction*> &instMap) {
    BBMap.clear();
    instMap.clear();

    entry = nullptr;
    exiting.clear();
    singleLatch = nullptr;
    if(!isSimplifiedForm())
        LOG_WARNING("Can't get singleLatch before LoopSimplified!")
    
    // 复制body并建立BB映射和指令映射
    for(BB *bb : blocks) {
        if(bb == header) 
            continue;

        BB *newBB = bb->copyBB();
        if(bb->getPreBasicBlocks().size() == 1 && bb->getPreBasicBlocks().front() == header) {
            entry = newBB;
        }   
        BBMap.insert({bb, newBB});

        list bbInsts = bb->getInstructions();
        list newBBInsts = newBB->getInstructions();
        auto bbIter = bbInsts.begin();
        auto newBBIter = newBBInsts.begin();
        for(; 
            bbIter != bbInsts.end() && newBBIter != newBBInsts.end();
            bbIter++, newBBIter++) 
        {
            instMap.insert({*bbIter, *newBBIter});
        }
    }
    singleLatch = BBMap[latch];
    LOG_ERROR("LoopBody don't have entry??", entry == nullptr)

    // 替换块间关系
    uset<BB*> exitingSet = {};
    for(auto [bb, newBB] : BBMap) {
        for(BB* &pre : newBB->getPreBasicBlocks()) {
            if(BBMap[pre])  pre = BBMap[pre];
        }
        for(BB* &succ : newBB->getSuccBasicBlocks()) {
            if(BBMap[succ]) {
                succ = BBMap[succ];
            } else if(succ != header) {
            // newBB的succ不含于Loop的Body 且 不是header，那它是一个exiting 
                exitingSet.insert(newBB);       
            }
        }
    }
    copy(exitingSet.begin(), exitingSet.end(), exiting.begin());

    // 替换指令
    for(auto [oldInst, newInst] : instMap) {
        vector<Value*> &ops = newInst->getOperands();
        if(oldInst->isPhi()) {
            for(int i = 1; i < ops.size(); i += 2) {
                BB *incoming = dynamic_cast<BB*>(ops[i]);
                if(incoming && BBMap[incoming]) {
                    ops[i] = BBMap[incoming];
                } 
            }
        } else if (oldInst->isBr()) {
            Instruction *cond;
            BB *trueBB, *falseBB;
            if(newInst->getNumOperands() == 1) {
                trueBB =  dynamic_cast<BB*>(newInst->getOperands()[0]);
                if(trueBB && BBMap[trueBB]) {
                    newInst->replaceOperand(0, BBMap[trueBB]);
                }
            } else {
            // 条件跳转
                cond = dynamic_cast<Instruction*>(newInst->getOperands()[0]);
                trueBB = dynamic_cast<BB*>(newInst->getOperands()[1]);
                falseBB = dynamic_cast<BB*>(newInst->getOperands()[2]);

                if(cond && instMap[cond]) { newInst->replaceOperand(0, instMap[cond]); }
                if(trueBB && BBMap[trueBB]) { newInst->replaceOperand(1, BBMap[trueBB]); }
                if(falseBB && BBMap[falseBB]) { newInst->replaceOperand(2, BBMap[falseBB]); }
            }

        } else if (oldInst->isCmpBr() || oldInst->isFCmpBr()) {
            Instruction *op1, *op2;
            BB *trueBB, *falseBB;

            op1 = dynamic_cast<Instruction*>(newInst->getOperands()[0]);
            op2 = dynamic_cast<Instruction*>(newInst->getOperands()[1]);
            trueBB = dynamic_cast<BB*>(newInst->getOperands()[2]);
            falseBB = dynamic_cast<BB*>(newInst->getOperands()[3]);

            if(op1 && instMap[op1]) { newInst->replaceOperand(0, instMap[op1]); }
            if(op2 && instMap[op2]) { newInst->replaceOperand(1, instMap[op2]); }
            if(trueBB && BBMap[trueBB]) { newInst->replaceOperand(2, BBMap[trueBB]); }
            if(falseBB && BBMap[falseBB]) { newInst->replaceOperand(3, BBMap[falseBB]); }
        }
    }
}

void Loop::findExits() {
    for(BB *bb : blocks) {
        for(BB *succ : bb->getSuccBasicBlocks()) {
            if(!contain(succ)) {
                addExit(succ);
            }
        }
    }
}

LoopTrip Loop::computeTrip(SCEV *scev) {
    // 如果未计算cond，先计算
    if(conditions.size()==0 && !computeConds()) {
        return LoopTrip::createEmptyTrip(-1);
    }
    // 不处理cond数目>1的情况
    if(conditions.size() != 1) {
        return LoopTrip::createEmptyTrip(-1);
    }

    // 只处理(i relOp const) 或 (const relOp i)的情况，
    // 同时将cond的比较强制更正为(i relOp const)形式
    PhiInst *lhs;
    ConstantInt *rhs;
    LoopCond::opType relOp;
    if( (lhs = dynamic_cast<PhiInst*>(conditions[0]->lhs)) && (rhs = dynamic_cast<ConstantInt*>(conditions[0]->rhs)) ) {
        relOp = conditions[0]->op;
    } else if ( (lhs = dynamic_cast<PhiInst*>(conditions[0]->rhs)) && (rhs = dynamic_cast<ConstantInt*>(conditions[0]->lhs)) ) {
        relOp = (LoopCond::opType)(-conditions[0]->op);
    } else {
        return LoopTrip::createEmptyTrip(-1);
    }

    // 计算step 并 判断循环是否可能为死循环
    int start, end, step, iter;

    // 判断i是否为归纳变量
    SCEVExpr *expr = scev->getExpr(lhs, this);
    if( !expr || !expr->isAddRec() || 
        expr->getOperands().size()!=2 || 
        !expr->getOperand(0)->isConst() || !expr->getOperand(1)->isConst() ) {
            return LoopTrip::createEmptyTrip(-1);
    }

    // 获取初始值
    start = expr->getOperand(0)->getConst();
    iter  = expr->getOperand(1)->getConst();
    end   = rhs->getValue();

    switch (relOp) {
        case  LoopCond::opType::eq:
            if(start != end) {
                step = 0;
            } else if(start == end && iter == 0) {
                return LoopTrip::createEmptyTrip(-2);
            } else {
                step = 1;
            }
            break;
        case  LoopCond::opType::ne:
            if(start == end){
                step = 0;
            } else if(start != end && iter == 0 || (end - start) % iter != 0
                   || start > end && iter > 0 || start < end && iter < 0 ) {
                return LoopTrip::createEmptyTrip(-2);
            } else {
                step = (end - start) / iter;
            }
            break;
        case  LoopCond::opType::gt:
            if(start <= end) {
                step = 0;
            } else if(start > end && iter >= 0) {
                return LoopTrip::createEmptyTrip(-2);
            } else {
            // 12 > 3, -3 -> step=(3-12+1)/-3 + 1 = 3
                step = (end - start + 1) / iter + 1;
            }
            break;
        case  LoopCond::opType::ge:
            if(start < end) {
                step = 0;
            } else if(start >= end && iter >= 0) {
                return LoopTrip::createEmptyTrip(-2);
            } else {
            // 12 >= 3, -3 -> step=(3-12)/-3 + 1 = 4
                step = (end - start) / iter + 1;
            }
            break;
        case  LoopCond::opType::lt:
            if(start >= end) {
                step = 0;
            } else if(start < end && iter <= 0) {
                return LoopTrip::createEmptyTrip(-2);
            } else {
            // 1 < 5, +2 -> step = (5-1-1)/2 + 1 = 2
                step = (end - start - 1) / iter + 1;
            }
            break;
        case  LoopCond::opType::le:
            if(start > end) {
                step = 0;
            } else if(start <= end && iter <= 0) {
                return LoopTrip::createEmptyTrip(-2);
            } else {
            // 1 <= 5, +2 -> step = (5-1)/2 + 1 = 3 
                step = (end - start) / iter + 1;
            }
            break;
        default:
            assert(0);
            break;
    }

    return {start, end, iter, step};
}