#include "analysis/SCEV.hpp"
#include "midend/Instruction.hpp"

static SCEVExpr *CRProd(SCEVExpr*, SCEVExpr*);

SCEVExpr *SCEVExpr::foldAdd(SCEVExpr *scev){
    if(this->loop && scev->loop && this->loop != scev->loop){
        LOG_ERROR("SCEVExpr in different(or empty) loop cann't add", 1)
    }

    if(this->isUnknown() || scev->isUnknown()) {
        return createUnknown(this->loop);
    }

    if(this->isConst() && scev->isConst()) {
        return createConst( this->getConst()+scev->getConst(), this->loop );
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
    } else if(this->isConst() && scev->isAddRec() || this->isAddRec() && scev->isConst()) {
        SCEVExpr* constExpr = this->isConst() ? this : scev;
        SCEVExpr* addRecExpr = this->isAddRec() ? this : scev;
        vector<SCEVExpr*> ops;

        ops.push_back( addRecExpr->getOperand(0)->foldAdd(constExpr) );
        ops.insert(ops.end(), addRecExpr->getOperands().begin()+1, addRecExpr->getOperands().end());

        return createAddRec(std::move(ops), this->loop);
    }
    
    return createUnknown(this->loop);
}

SCEVExpr *SCEVExpr::foldMul(SCEVExpr *scev){
    if(this->loop && scev->loop && this->loop != scev->loop){
        LOG_ERROR("SCEVExpr in different(or empty) loop cann't mul", 1)
    }

    if(this->isUnknown() || scev->isUnknown()) {
        return createUnknown(this->loop);
    }

    if(this->isConst() && scev->isConst()) {
        return createConst( this->getConst()*scev->getConst(), this->loop );
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
    }

    return createUnknown(this->loop);
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
    for(Loop *l : loops) {
        visitLoop(l);
        for(BasicBlock *bb : l->getBlocks()) {
            visitBlock(bb, l);
        }
    }
}

void SCEV::visitLoop(Loop *loop) {
    for(Instruction *inst : loop->getHeader()->getInstructions()) {
        if(inst->isPhi()) {             // Loop_Phi
            exprMapping[inst] = getPhiSCEV( dynamic_cast<PhiInst*>(inst), loop );
            LOG_WARNING(exprMapping[inst]->print());
        }
    }

    if(!loop->getInners().size())   return;
    for(Loop *l : loop->getInners()) {
        visitLoop(l);
    }
}

void SCEV::visitBlock(BasicBlock *bb, Loop *loop) {
    Value *a, *b;
    for(Instruction *inst : bb->getInstructions()) {
        if(inst->isPhi() && exprMapping.count(inst) != 0)           // loop-phi
            continue;

        if(inst->isAdd() || inst->isSub() || inst->isMul()) {
            a = inst->getOperand(0);
            b = inst->getOperand(1);
            // a,b 不为空且不为unknown
            if( !getExpr(a, loop) || !getExpr(b, loop) || getExpr(a, loop)->isUnknown() || getExpr(b, loop)->isUnknown() ) {
                exprMapping[inst] = SCEVExpr::createUnknown(loop);
            } else {
                if(inst->isAdd()) {
                    exprMapping[inst] = getExpr(a, loop)->foldAdd( getExpr(b, loop) );
                } else if(inst->isSub()) {
                    exprMapping[inst] = getExpr(a, loop)->foldAdd( getExpr(b, loop)->getNegate() );
                } else {
                    exprMapping[inst] = getExpr(a, loop)->foldMul( getExpr(b, loop) );
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

    ConstantInt *initVal = dynamic_cast<ConstantInt*>(phi->getOperand(0));
    BinaryInst *f = dynamic_cast<BinaryInst*>(phi->getOperand(2));
    if(!initVal || !f) {
        initVal = dynamic_cast<ConstantInt*>(phi->getOperand(2));
        if(!initVal || !f) {  // phi的两个数至少一个是整数, 一个是二元运算指令
            return SCEVExpr::createUnknown(loop);
        } else {
            f = dynamic_cast<BinaryInst*>(phi->getOperand(0));
        }
    }

    // 暂时只识别i = i +/- (num)的情况
    // 即假设use链为f -> phi -> f，不是可能出错
    int addNum;
    if( f->isAdd() || f->isSub() ) {
        ConstantInt *addInt = dynamic_cast<ConstantInt*>(f->getOperand(0));
        PhiInst *usedPhi = dynamic_cast<PhiInst*>(f->getOperand(1));
        if(!addInt || !usedPhi) {
            addInt = dynamic_cast<ConstantInt*>(f->getOperand(1));
            usedPhi = dynamic_cast<PhiInst*>(f->getOperand(0));
            if(!addInt || !usedPhi) {
                return SCEVExpr::createUnknown(loop);
            }
        }
        if(usedPhi != phi) {
            return SCEVExpr::createUnknown(loop);
        }
        addNum = f->isSub() ? -addInt->getValue() : addInt->getValue();
    }
    
    return SCEVExpr::createAddRec( { getExpr(initVal, loop), SCEVExpr::createConst(addNum, loop) }, loop);
}

string SCEV::print(){
    module_->print();
    string scevStr = "";
    for(Loop *loop : loops) {
        for(BasicBlock *bb : loop->getBlocks()) {
            for(Instruction *inst : bb->getInstructions()) {
                if(getExpr(inst, loop) != nullptr) {
                    // if(getExpr(inst, loop)->isUnknown())
                        // continue;
                    scevStr += STRING(inst->print()) + "\n";
                    scevStr += STRING_YELLOW(inst->getName()) + ": " + getExpr(inst, loop)->print() + "\n";
                }
            }
        }
    }
    return scevStr;
}