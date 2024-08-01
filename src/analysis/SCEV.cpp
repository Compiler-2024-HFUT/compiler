#include "analysis/SCEV.hpp"
#include "midend/Instruction.hpp"

#include <list>
using std::list;

static SCEVExpr *CRProd(SCEVExpr*, SCEVExpr*);

SCEVVal *SCEVVal::addSCEVVal(SCEVVal *rhs)  {
    SCEVVal *lhs = this;
    if(lhs->isUnk() || rhs->isUnk())
        return createUnkVal();
    
    ConstantInt *lhsInt = dynamic_cast<ConstantInt*>(lhs->sval);
    ConstantInt *rhsInt = dynamic_cast<ConstantInt*>(rhs->sval);
    PhiInst *lhsPhi = dynamic_cast<PhiInst*>(lhs->sval);
    PhiInst *rhsPhi = dynamic_cast<PhiInst*>(rhs->sval);

    if(lhs->isConst() && rhs->isConst()) {
        return createConVal(ConstantInt::get(lhsInt->getValue() + rhsInt->getValue()));
    } else if(lhs->isPhi() && rhs->isPhi()) {
        if(lhsPhi == rhsPhi)
            return createMulVal(ConstantInt::get(2), lhsPhi);
        else
            return createAddVal(lhsPhi, rhsPhi);
    } else if(lhs->isConst() && rhs->isPhi() || lhs->isConst() && rhs->isMul()) {
        return createAddVal(lhsInt, rhsPhi);
    } else if(lhs->isPhi() && rhs->isConst() || lhs->isMul() && rhs->isConst()) {
        return createAddVal(rhsInt, lhsPhi);
    } else if(lhs->isMul() && rhs->isMul()) {
        if(lhs->operands[1] == rhs->operands[1]) {
            return createMulVal(ConstantInt::get(2), lhs->operands[1]->sval);
        } else {
            return createAddVal({lhs, rhs}); 
        }
    } else if(lhs->isAdd() && rhs->isConst() || lhs->isConst() && rhs->isAdd()) {
        SCEVVal *addVal = (lhs->isAdd() ? lhs : rhs);
        SCEVVal *constVal = (lhs->isConst() ? lhs : rhs);
        vector<SCEVVal*> newOperands = {};

        if(addVal->operands[0]->isConst()) {
            newOperands.push_back( addVal->operands[0]->addSCEVVal(constVal) );
        } else {
            newOperands.push_back( constVal );
            for(SCEVVal *val : addVal->operands) {
                newOperands.push_back(val);
            }
        }
        return createAddVal(newOperands);
    } else if(lhs->isAdd() && rhs->isPhi() || lhs->isPhi() && rhs->isAdd()) {
        SCEVVal *addVal = (lhs->isAdd() ? lhs : rhs);
        SCEVVal *phiVal = (lhs->isPhi() ? lhs : rhs);
        vector<SCEVVal*> &oldOperands = addVal->operands; 
        vector<SCEVVal*> newOperands = vector<SCEVVal*>(oldOperands.begin(), oldOperands.end());
        
        bool addLast = true;
        for(int i = 0; i < newOperands.size(); i++) {
            if(newOperands[i]->isPhi() && newOperands[i]->sval == phiVal->sval ||
               newOperands[i]->isMul() && newOperands[i]->operands[1]->sval == phiVal->sval){
                newOperands[i] = newOperands[i]->addSCEVVal(phiVal);
                addLast = false;
            } 
        }
        if(addLast)
            newOperands.push_back(phiVal);
        
        return createAddVal(newOperands); 
    } else if(lhs->isAdd() && rhs->isMul() || lhs->isMul() && rhs->isAdd()) {
        SCEVVal *addVal = (lhs->isAdd() ? lhs : rhs);
        SCEVVal *mulVal = (lhs->isMul() ? lhs : rhs);
        vector<SCEVVal*> &oldOperands = addVal->operands; 
        vector<SCEVVal*> newOperands = vector<SCEVVal*>(oldOperands.begin(), oldOperands.end());

        bool addLast = true;
        for(int i = 0; i < newOperands.size(); i++) {
            if(newOperands[i]->isPhi() && newOperands[i]->sval == mulVal->operands[1]->sval ||
               newOperands[i]->isMul() && newOperands[i]->operands[1]->sval == mulVal->operands[1]->sval){
                newOperands[i] = newOperands[i]->addSCEVVal(mulVal);
                addLast = false;
            }
        }
        if(addLast)
            newOperands.push_back(mulVal);
        
        return createAddVal(newOperands); 
    }
    else {
        return createAddVal({lhs, rhs}); 
    }
}

// 仅处理 const*const / const*phi / const*mul / const*add
SCEVVal *SCEVVal::mulSCEVVal(SCEVVal *rhs) {
    SCEVVal *lhs = this;
    if(lhs->isUnk() || rhs->isUnk())
        return createUnkVal();
    
    ConstantInt *lhsInt = dynamic_cast<ConstantInt*>(lhs->sval);
    ConstantInt *rhsInt = dynamic_cast<ConstantInt*>(rhs->sval);
    PhiInst *lhsPhi = dynamic_cast<PhiInst*>(lhs->sval);
    PhiInst *rhsPhi = dynamic_cast<PhiInst*>(rhs->sval);

    if(lhs->isConst() && rhs->isConst()) {
        int sum = lhsInt->getValue() * rhsInt->getValue();
        return createConVal(ConstantInt::get(sum));
    } else if(lhs->isConst() && rhs->isPhi() || lhs->isPhi() && rhs->isConst()) {
        return createMulVal(lhs->sval, rhs->sval);
    } else if(lhs->isConst() && rhs->isMul() || lhs->isMul() && rhs->isConst()) {
        int prod = lhs->isConst() ? lhsInt->getValue() : rhsInt->getValue();
        prod *= lhs->isMul() ? 
                dynamic_cast<ConstantInt*>(lhs->operands[0]->sval)->getValue()  :
                dynamic_cast<ConstantInt*>(rhs->operands[0]->sval)->getValue();
        return createMulVal(ConstantInt::get(prod), lhs->isMul() ? lhs->operands[1]->sval : rhs->operands[1]->sval);
    } else if(lhs->isConst() && rhs->isAdd() || lhs->isAdd() && rhs->isConst()) {
        vector<SCEVVal*> &oldOperands = lhs->isAdd()? lhs->operands : rhs->operands;
        vector<SCEVVal*> newOperands = vector<SCEVVal*>(oldOperands.begin(), oldOperands.end());
        SCEVVal *constVal = lhs->isConst() ? lhs : rhs;
        for(int i = 0; i < newOperands.size(); i++) {
            newOperands[i] = newOperands[i]->mulSCEVVal(constVal);
        }
    } else {
        return createUnkVal();
    }
    return createUnkVal();
}

SCEVExpr *SCEVExpr::foldAdd(SCEVExpr *scev){
    if(this->loop && scev->loop && this->loop != scev->loop){
        LOG_ERROR("SCEVExpr in different(or empty) loop cann't add", 1)
    }

    if(this->isUnknown() || scev->isUnknown()) {
        return createUnknown(this->loop);
    }

    if(this->isValue() && scev->isValue()) {
        SCEVVal *lhs = this->val;
        SCEVVal *rhs = scev->val;

        SCEVVal *sum = lhs->addSCEVVal(rhs);
        if(sum->isUnk())    
            return createUnknown(this->loop);
        else
            return createValue(sum, this->loop);
    } else if(this->isAddRec() && scev->isAddRec()) {
        vector<SCEVExpr*> ops;
        SCEVExpr* tmp = nullptr;
        int trip = std::min(this->getOperands().size(), scev->getOperands().size());
        int len  = std::max(this->getOperands().size(), scev->getOperands().size());
        for(int i=0; i<trip; i++) {
            tmp = this->getOperand(i)->foldAdd(scev->getOperand(i));
            if(tmp->isUnknown()){ return createUnknown(this->loop); }
            ops.push_back(tmp);
        }
        
        tmp = (this->getOperands().size() > scev->getOperands().size()) ? this : scev;
        for(int i=trip; i<len; i++) { ops.push_back(tmp->getOperand(i)); }

        return createAddRec(std::move(ops), this->loop);
    } else if(this->isValue() && scev->isAddRec() || this->isAddRec() && scev->isValue()) {
        SCEVExpr* valExpr = this->isValue() ? this : scev;
        SCEVExpr* addRecExpr = this->isAddRec() ? this : scev;
        vector<SCEVExpr*> ops = {};

        SCEVExpr *sum = addRecExpr->getOperand(0)->foldAdd(valExpr);
        if(sum->isUnknown())
            return createUnknown(this->loop);
        else {
            ops.push_back(sum);
            ops.insert(ops.end(), addRecExpr->getOperands().begin()+1, addRecExpr->getOperands().end());

            return createAddRec(std::move(ops), this->loop);
        }
    } else {
        return createUnknown(this->loop);
    }
}

SCEVExpr *SCEVExpr::foldMul(SCEVExpr *scev){
    if(this->loop && scev->loop && this->loop != scev->loop){
        LOG_ERROR("SCEVExpr in different(or empty) loop cann't mul", 1)
    }

    if(this->isUnknown() || scev->isUnknown()) {
        return createUnknown(this->loop);
    }

    if(this->isValue() && scev->isValue()) {
        SCEVVal *lhs = this->val;
        SCEVVal *rhs = scev->val;

        SCEVVal *prod = lhs->mulSCEVVal(rhs);
        if(prod->isUnk())    
            return createUnknown(this->loop);
        else
            return createValue(prod, this->loop);
    } else if(this->isConst() && scev->isAddRec() || this->isAddRec() && scev->isConst()) {
        SCEVExpr* constExpr = this->isConst() ? this : scev;
        SCEVExpr* addRecExpr = this->isAddRec() ? this : scev;
        vector<SCEVExpr*> ops;
        
        for(SCEVExpr *expr : addRecExpr->getOperands()) {
            ops.push_back( expr->foldMul(constExpr) );
        }
        return createAddRec(std::move(ops), this->loop);
    } else if(this->isAddRec() && scev->isAddRec()) {
        return CRProd(this, scev);
    } else {
        return createUnknown(this->loop);
    }
}

static SCEVExpr *CRProd(SCEVExpr *a, SCEVExpr *b) {
    if(a->loop && b->loop && a->loop != b->loop){
        LOG_ERROR("SCEVExpr in different(or empty) loop cann't add", 1)
    }

    if(a->isConst() || b->isConst()) {
        return a->foldMul(b);
    }

    // keep num of a's ops more than b's ops
    if(a->getOperands().size() < b->getOperands().size()) { std::swap(a, b); }

    vector<SCEVExpr*> f1_ops(a->getOperands().begin()+1, a->getOperands().end());
    vector<SCEVExpr*> g1_ops(b->getOperands().begin()+1, b->getOperands().end());
    SCEVExpr *f1 = (f1_ops.size()==1) ? SCEVExpr::createConst(f1_ops[0]->getConst(), a->loop) : SCEVExpr::createAddRec(f1_ops, a->loop);
    SCEVExpr *g1 = (g1_ops.size()==1) ? SCEVExpr::createConst(g1_ops[0]->getConst(), a->loop) : SCEVExpr::createAddRec(g1_ops, a->loop);

    SCEVExpr *b1 = b->foldAdd(g1);
    if( b1->isUnknown() ) { return SCEVExpr::createUnknown(a->loop); }

    SCEVExpr *prod1 = CRProd(a, g1);
    SCEVExpr *prod2 = CRProd(f1, b1);
    if( prod1->isUnknown() || prod2->isUnknown() ) { return SCEVExpr::createUnknown(a->loop); }
    
    if( prod1->getOperands().size() != prod2->getOperands().size() ) {
        LOG_ERROR("wrong result! len(prod1) != len(prod2)", 1)
    }

    vector<SCEVExpr*> tmp;
    SCEVExpr *tmp_expr;
    int len = prod1->getOperands().size();
    tmp_expr = a->getOperand(0)->foldMul(b->getOperand(0));
    if(tmp_expr->isUnknown()) { return SCEVExpr::createUnknown(a->loop); }
    tmp.push_back( tmp_expr );
    for(int i=0; i<len; i++) {
        tmp_expr = prod1->getOperand(i)->foldAdd(prod2->getOperand(i));
        if(tmp_expr->isUnknown()) { return SCEVExpr::createUnknown(a->loop); }
        tmp.push_back( tmp_expr );
    }
    return SCEVExpr::createAddRec(std::move(tmp), a->loop);
}

void SCEV::analyseOnFunc(Function *func) {
    loops = infoManager->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops) {
        loopPhis[loop] = {};
        // 访问loop中的所有loop-phi（位于header的phi），包括子循环中的loop-phi
        visitLoop(loop, loop);

        // BFS遍历基本块，计算非loop-phi的SCEVExpr
        BB *header = loop->getHeader();
        BB *latch  = loop->getSingleLatch();
        umap<BB*, bool> isVisited = {};
        BB *curBB;
        list<BB*> bbStack = { header };
        LOG_ERROR("Analyzing SCEV After LoopSimplify!", !latch)

        for(BB *bb : loop->getBlocks())
            isVisited[bb] = false;

        while(!bbStack.empty()) {
            curBB = bbStack.front();
            bbStack.pop_front();
            isVisited[curBB] = true;

            for(BB *succ : curBB->getSuccBasicBlocks()) {
                if(!isVisited[succ] && loop->contain(succ))
                    bbStack.push_back(succ);
            }

            visitBlock(curBB, loop);
        }

    }
}

void SCEV::visitLoop(Loop *outer, Loop *inner) {
    for(Instruction *inst : inner->getHeader()->getInstructions()) {
        if(!inst->isPhi())
            break;

        // LoopPhi，暂时不访问
        loopPhis[outer].insert(dynamic_cast<PhiInst*>(inst));
        // exprMapping[inst] = getPhiSCEV( dynamic_cast<PhiInst*>(inst), outer );
        // LOG_WARNING(exprMapping[inst]->print())
    }

    for(Loop *inn : inner->getInners()) {
        visitLoop(outer, inn);
    }
}

void SCEV::visitBlock(BasicBlock *bb, Loop *loop) {
    Value *a, *b;
    for(Instruction *inst : bb->getInstructions()) {
        // loop-phi
        if(inst->isPhi() && loopPhis[loop].count(dynamic_cast<PhiInst*>(inst))) {
            exprMapping[inst] =  getPhiSCEV( dynamic_cast<PhiInst*>(inst), loop );
            continue;
        } 

        if(inst->isAdd() || inst->isSub() || inst->isMul()) {
            a = inst->getOperand(0);
            b = inst->getOperand(1);
            SCEVExpr *exprA = getExpr(a, loop);
            SCEVExpr *exprB = getExpr(b, loop);
            // a,b 不为空且不为unknown
            if( !exprA || !exprB || exprA->isUnknown() || exprB->isUnknown() ) {
                exprMapping[inst] = SCEVExpr::createUnknown(loop);
            } else {
                if(inst->isAdd()) {
                    exprMapping[inst] = exprA->foldAdd( exprB );
                } else if(inst->isSub()) {
                    exprMapping[inst] = exprA->foldAdd( exprB->getNegate() );
                } else {
                    exprMapping[inst] = exprA->foldMul( exprB );
                }
            }
        } else if(inst->isPhi() && exprMapping.count(inst)==0) {       // if-phi
            if(inst->getNumOperands() == 4 && inst->getOperand(0) == inst->getOperand(2)) {
                exprMapping[inst] = getExpr(inst->getOperand(0), loop);
            }
        } else if(!inst->isVoid()) {
            exprMapping[inst] = SCEVExpr::createUnknown(loop);
        }        
    }
}

SCEVExpr *SCEV::getPhiSCEV(PhiInst *phi, Loop *loop) {
    if(phi->getNumOperands() != 4) {
        return SCEVExpr::createUnknown(loop);
    }

    // 假设可求SCEV的phi格式为 phi i32 [const/phi/add/sub, b1], [bin, b2]
    ConstantInt *initConst = dynamic_cast<ConstantInt*>(phi->getOperand(0));
    PhiInst *initPhi = dynamic_cast<PhiInst*>(phi->getOperand(0));
    BinaryInst *initBin = dynamic_cast<BinaryInst*>(phi->getOperand(0));

    BinaryInst *bin = dynamic_cast<BinaryInst*>(phi->getOperand(2));
    if(!getExpr(phi->getOperand(0), loop) || !bin) {
        return SCEVExpr::createValue(SCEVVal::createPhiVal(phi), loop);
    }
        
    // 即假设use链为bin -> phi -> bin，不是可能出错
    int addNum;
    if( bin->isAdd() || bin->isSub() ) {
        ConstantInt *addInt = dynamic_cast<ConstantInt*>(bin->getOperand(0));
        PhiInst *usedPhi = dynamic_cast<PhiInst*>(bin->getOperand(1));
        if(!addInt || !usedPhi) {
            addInt = dynamic_cast<ConstantInt*>(bin->getOperand(1));
            usedPhi = dynamic_cast<PhiInst*>(bin->getOperand(0));
            if(!addInt || !usedPhi) {
                return SCEVExpr::createValue(SCEVVal::createPhiVal(phi), loop);
            }
        }
        if(usedPhi != phi) {
            return SCEVExpr::createValue(SCEVVal::createPhiVal(phi), loop);
        }
        addNum = bin->isSub() ? -addInt->getValue() : addInt->getValue();
    } else {
        return SCEVExpr::createValue(SCEVVal::createPhiVal(phi), loop);
    }
    
    if(initConst) {
        return SCEVExpr::createAddRec( { getExpr(initConst, loop), SCEVExpr::createConst(addNum, loop) }, loop);
    } else if(initPhi) {
        return SCEVExpr::createAddRec( { getExpr(initPhi, loop), SCEVExpr::createConst(addNum, loop) }, loop);
    } else if(initBin) {
        return SCEVExpr::createAddRec( { getExpr(initBin, loop), SCEVExpr::createConst(addNum, loop) }, loop);
    } else {
        return SCEVExpr::createValue(SCEVVal::createPhiVal(phi), loop);
    }
}

string SCEV::print(){
    module_->print();
    
    string scevStr = "";
    for(Loop *loop : loops) {
        for(BasicBlock *bb : loop->getBlocks()) {
            for(Instruction *inst : bb->getInstructions()) {
                if(getExpr(inst, loop) != nullptr) {
                    if(getExpr(inst, loop)->isUnknown())
                        continue;
                    // if(!inst->isPhi())
                    //     continue;
                    scevStr += STRING(inst->print()) + "\n";
                    scevStr += STRING_YELLOW(inst->getName()) + ": " + getExpr(inst, loop)->print() + "\n";
                }
            }
        }
    }
    return scevStr;
}