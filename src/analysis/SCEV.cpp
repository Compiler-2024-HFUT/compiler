#include "analysis/SCEV.hpp"

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
    if(this->loop != scev->loop){
        LOG_ERROR("SCEVExpr in different loop cann't mul", -1)
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
            ops.push_back( expr->foldAdd(constExpr) );
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
        return a->foldAdd(b);
    }

    // keep num of a's ops more than b's ops
    if(a->getOperands().size() < b->getOperands().size()) { std::swap(a, b); }

    vector<SCEVExpr*> f1_ops(a->getOperands().begin()+1, a->getOperands().end());
    vector<SCEVExpr*> g1_ops(b->getOperands().begin()+1, b->getOperands().end());
    SCEVExpr *f1 = SCEVExpr::createAddRec(f1_ops, a->loop);
    SCEVExpr *g1 = SCEVExpr::createAddRec(g1_ops, a->loop);
    if( f1->isUnknown() || g1->isUnknown() ) { return SCEVExpr::createUnknown(a->loop); }

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
    tmp_expr = prod1->getOperand(0)->foldAdd(prod2->getOperand(0));
    if(tmp_expr->isUnknown()) { return SCEVExpr::createUnknown(a->loop); }
    tmp.push_back( tmp_expr );
    for(int i=1; i<len; i++) {
        tmp_expr = prod1->getOperand(i)->foldMul(prod2->getOperand(i));
        if(tmp_expr->isUnknown()) { return SCEVExpr::createUnknown(a->loop); }
        tmp.push_back( tmp_expr );
    }
    return SCEVExpr::createAddRec(std::move(tmp), a->loop);
}

void SCEV::analyseOnFunc(Function *func){
    vector<Loop*> loops = infoManager->getInfo<LoopInfo>()->getLoops(func);
    
}

string SCEV::print(){
    return "";
}