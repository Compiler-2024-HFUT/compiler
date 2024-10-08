#include "analysis/SCEV.hpp"
#include "midend/Instruction.hpp"

#include <list>
using std::list;

static SCEVExpr *CRProd(SCEVExpr*, SCEVExpr*);

// 返回endInst，也即Value的运算结果
Value *SCEVVal::transToValue(BB *bb) {
    if(this->isConst() || this->isInv())
        return this->sval;
    
    Value *midValue;
    if(this->isMul()) {
        midValue = BinaryInst::createMul(operands[0]->sval, operands[1]->sval, bb);
        bb->getInstructions().pop_back();
        bb->addInstrBeforeTerminator(dynamic_cast<Instruction*>(midValue));
    } else {
    // AddVal
        midValue = operands[0]->transToValue(bb);

        for(int i = 1; i < operands.size(); i++) {
            midValue = BinaryInst::createAdd(midValue, operands[i]->transToValue(bb), bb);
            bb->getInstructions().pop_back();
            bb->addInstrBeforeTerminator(dynamic_cast<Instruction*>(midValue));
        }
        // LOG_WARNING(midValue->print());
    }
    return midValue;
}

SCEVVal *SCEVVal::addSCEVVal(SCEVVal *rhs)  {
    SCEVVal *lhs = this;
    if(lhs->isUnk() || rhs->isUnk())
        return createUnkVal();
    
    ConstantInt *lhsInt = dynamic_cast<ConstantInt*>(lhs->sval);
    ConstantInt *rhsInt = dynamic_cast<ConstantInt*>(rhs->sval);
    Value *lhsInv = lhs->sval;
    Value *rhsInv = rhs->sval;

    if( lhsInt && lhsInt->getValue() == 0 || rhsInt && rhsInt->getValue() == 0) {
        if(lhsInt && lhsInt->getValue() == 0)
            return rhs;
        else    
            return lhs;
    }

    if(lhs->isConst() && rhs->isConst()) {
        return createConVal(ConstantInt::get(lhsInt->getValue() + rhsInt->getValue()));
    } else if(lhs->isInv() && rhs->isInv()) {
        if(lhsInv == rhsInv)
            return createMulVal(ConstantInt::get(2), lhsInv);
        else
            return createAddVal(lhsInv, rhsInv);
    } else if(lhs->isConst() && rhs->isInv() || lhs->isInv() && rhs->isConst()) {
        if(lhs->isConst())
            return createAddVal(lhsInt, rhsInv);
        else
            return createAddVal(rhsInt, lhsInv);
    } else if(lhs->isConst() && rhs->isMul() || lhs->isMul() && rhs->isConst()) {
        if(lhs->isConst())
            return createAddVal({lhs, rhs});
        else
            return createAddVal({rhs, lhs});
    } else if(lhs->isMul() && rhs->isMul()) {
        if(lhs->operands[1]->sval == rhs->operands[1]->sval) {
            int sum = dynamic_cast<ConstantInt*>(lhs->operands[0]->sval)->getValue()
                    + dynamic_cast<ConstantInt*>(rhs->operands[0]->sval)->getValue();
            return createMulVal(ConstantInt::get(sum), lhs->operands[1]->sval);
        } else {
            return createAddVal({lhs, rhs}); 
        }
    } else if(lhs->isInv() && rhs->isMul() || lhs->isMul() && rhs->isInv()) {
        SCEVVal *phiVal = (lhs->isInv()? lhs : rhs);
        SCEVVal *mulVal = (lhs->isMul()? lhs : rhs);
        vector<SCEVVal*> &mulOperands = mulVal->operands;
        
        if(mulOperands[1]->sval == phiVal->sval) {
            int sum = dynamic_cast<ConstantInt*>(mulOperands[0]->sval)->getValue()+ 1;
            return createMulVal(ConstantInt::get(sum), mulOperands[1]->sval);
        } else {
            return createAddVal({phiVal, mulVal});
        }
    } else if(lhs->isAdd() && rhs->isConst() || lhs->isConst() && rhs->isAdd()) {
        SCEVVal *addVal = (lhs->isAdd() ? lhs : rhs);
        SCEVVal *constVal = (lhs->isConst() ? lhs : rhs);
        vector<SCEVVal*> newOperands = {};

        if(addVal->operands[0]->isConst()) {
            newOperands.push_back( addVal->operands[0]->addSCEVVal(constVal) );
            for(int i = 1; i < addVal->operands.size(); i++) {
                newOperands.push_back(addVal->operands[i]);
            }
        } else {
            newOperands.push_back( constVal );
            for(SCEVVal *val : addVal->operands) {
                newOperands.push_back(val);
            }
        }

        return createAddVal(newOperands);
    } else if(lhs->isAdd() && rhs->isInv() || lhs->isInv() && rhs->isAdd()) {
        SCEVVal *addVal = (lhs->isAdd() ? lhs : rhs);
        SCEVVal *phiVal = (lhs->isInv() ? lhs : rhs);
        vector<SCEVVal*> &oldOperands = addVal->operands; 
        vector<SCEVVal*> newOperands = vector<SCEVVal*>(oldOperands.begin(), oldOperands.end());
        
        bool addLast = true;
        for(int i = 0; i < newOperands.size(); i++) {
            if(newOperands[i]->isInv() && newOperands[i]->sval == phiVal->sval ||
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
            if(newOperands[i]->isInv() && newOperands[i]->sval == mulVal->operands[1]->sval ||
               newOperands[i]->isMul() && newOperands[i]->operands[1]->sval == mulVal->operands[1]->sval){
                newOperands[i] = newOperands[i]->addSCEVVal(mulVal);
                addLast = false;
            }
        }
        if(addLast)
            newOperands.push_back(mulVal);
        
        return createAddVal(newOperands); 
    } else if(lhs->isAdd() && rhs->isAdd()) {
        vector<SCEVVal*> &rhsOperands = rhs->operands;
        SCEVVal *res = lhs;
        for(SCEVVal *op : rhsOperands) {
            res = res->addSCEVVal(op);
        }
        return res;
    } else {
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
    Value *lhsInv = lhs->sval;
    Value *rhsInv = rhs->sval;

    // val * 0
    if( lhsInt && lhsInt->getValue() == 0 || rhsInt && rhsInt->getValue() == 0) {
        return createConVal(ConstantInt::get(0));
    }
    // val * 1
    if( lhsInt && lhsInt->getValue() == 1 )
        return rhs;
    else if( rhsInt && rhsInt->getValue() == 1 )
        return lhs;

    if(lhs->isConst() && rhs->isConst()) {
        int sum = lhsInt->getValue() * rhsInt->getValue();
        return createConVal(ConstantInt::get(sum));
    } else if(lhs->isConst() && rhs->isInv() || lhs->isInv() && rhs->isConst()) {
        if(lhs->isConst())
            return createMulVal(lhsInt, rhsInv);
        else
            return createMulVal(rhsInt, lhsInv);
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
        return createAddVal(newOperands);
    } else {
        return createUnkVal();
    }
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
    loops[func] = infoManager->getInfo<LoopInfo>()->getLoops(func);
    for(Loop *loop : loops[func]) {
        loopPhis[loop] = {};
        // 访问loop中的所有loop-phi（位于header的phi），包括子循环中的loop-phi
        visitLoop(loop);

        BB *header = loop->getHeader();
        BB *latch  = loop->getSingleLatch();
        umap<BB*, bool> isVisited = {};
        BB *curBB;
        list<BB*> bbStack = { header };
        LOG_ERROR("Analyzing SCEV After LoopSimplify!", !latch)

        // 每个BB对应的最里层loop
        umap<BB*, Loop*> bbLoop = {};
        std::function<void(Loop*)> findBBLoop = [&](Loop *loop) {
            for(Loop *inner : loop->getInners()) {
                findBBLoop(inner);
            }

            for(BB *bb : loop->getBlocks()) {
                if(bbLoop.count(bb))
                    continue;
                bbLoop[bb] = loop;
            }
        };
        findBBLoop(loop);

        // BFS遍历基本块，计算非loop-phi的SCEVExpr
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

            visitBlock(curBB, bbLoop[curBB]);
        }
    }
}

void SCEV::visitLoop(Loop *loop) {
    for(Instruction *inst : loop->getHeader()->getInstructions()) {
        if(!inst->isPhi())
            break;

        loopPhis[loop].insert(dynamic_cast<PhiInst*>(inst));
        exprMapping[loop][inst] = getPhiSCEV( dynamic_cast<PhiInst*>(inst), loop );
        // LOG_WARNING(exprMapping[loop][inst]->print())
    }

    for(Loop *inner : loop->getInners()) {
        visitLoop(inner);
    }
}

void SCEV::visitBlock(BasicBlock *bb, Loop *loop) {
    Value *a, *b;
    for(Instruction *inst : bb->getInstructions()) {
        // loop-phi
        if(inst->isPhi() && loopPhis[loop].count(dynamic_cast<PhiInst*>(inst))) {
            continue;
        } 

        if(inst->isAdd() || inst->isSub() || inst->isMul()) {
            a = inst->getOperand(0);
            b = inst->getOperand(1);
            SCEVExpr *exprA = getExpr(a, loop);
            SCEVExpr *exprB = getExpr(b, loop);
            // a,b 不为空且不为unknown
            if( !exprA || !exprB || exprA->isUnknown() || exprB->isUnknown() ) {
                exprMapping[loop][inst] = SCEVExpr::createUnknown(loop);
            } else {
                if(inst->isAdd()) {
                    exprMapping[loop][inst] = exprA->foldAdd( exprB );
                } else if(inst->isSub()) {
                    exprMapping[loop][inst] = exprA->foldAdd( exprB->getNegate() );
                } else {
                    exprMapping[loop][inst] = exprA->foldMul( exprB );
                }
            }
        } else if(inst->isPhi() && exprMapping[loop].count(inst)==0) {       // if-phi
            if(inst->getNumOperands() == 4 && inst->getOperand(0) == inst->getOperand(2)) {
                exprMapping[loop][inst] = getExpr(inst->getOperand(0), loop);
            }
        } else if(!inst->isVoid()) {
            exprMapping[loop][inst] = SCEVExpr::createUnknown(loop);
        }        
    }
}

SCEVExpr *SCEV::getPhiSCEV(PhiInst *phi, Loop *loop) {
    if(phi->getNumOperands() != 4) {
        return SCEVExpr::createUnknown(loop);
    }

    // 假设可求SCEV的phi格式为 phi i32 [const/inv, b1], [bin, b2]
    LoopInvariant *inv = infoManager->getInfo<LoopInvariant>(); 
    ConstantInt *initConst = dynamic_cast<ConstantInt*>(phi->getOperand(0));
    Value *initInv  = phi->getOperand(0);

    BinaryInst *bin = dynamic_cast<BinaryInst*>(phi->getOperand(2));
    if(!initConst && !inv->isInvariable(loop, initInv) || !bin) {
        return SCEVExpr::createValue(SCEVVal::createInvVal(phi), loop);
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
                return SCEVExpr::createValue(SCEVVal::createInvVal(phi), loop);
            }
        }
        if(usedPhi != phi) {
            return SCEVExpr::createValue(SCEVVal::createInvVal(phi), loop);
        }
        addNum = bin->isSub() ? -addInt->getValue() : addInt->getValue();
    } else {
        return SCEVExpr::createValue(SCEVVal::createInvVal(phi), loop);
    }
    
    if(initConst) {
        return SCEVExpr::createAddRec( { getExpr(initConst, loop), SCEVExpr::createConst(addNum, loop) }, loop);
    } else if(initInv) {
        return SCEVExpr::createAddRec( { getExpr(initInv, loop), SCEVExpr::createConst(addNum, loop) }, loop);
    } else {
        return SCEVExpr::createValue(SCEVVal::createInvVal(phi), loop);
    }
}

string SCEV::print(){
    module_->print();
    
    std::function<string(Loop *)> printInner = [&](Loop *l) -> string {
        string innerStr = "";
        for(Loop *inner : l->getInners()) {
            innerStr += STRING_RED("Loop<" + inner->getHeader()->getName() + ">:\n");
            for(BasicBlock *bb : inner->getBlocks()) {
                for(Instruction *inst : bb->getInstructions()) {
                    if(getExpr(inst, inner) != nullptr) {
                        if(getExpr(inst, inner)->isUnknown())
                            continue;
                        innerStr += STRING(inst->print()) + "\n";
                        innerStr += STRING_YELLOW(inst->getName()) + ": " + getExpr(inst, inner)->print() + "\n";
                    }
                }
            }
            innerStr += printInner(inner);
        }
        return innerStr;
    };
    
    string scevStr = "";
    for(Function *f : module_->getFunctions()) {
        if(f->getBasicBlocks().size() == 0)
            continue;
        for(Loop *loop : loops[f]) {
            scevStr += STRING_RED("Loop<" + loop->getHeader()->getName() + ">:\n");
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
            scevStr += printInner(loop);
        }
    }
    
    return scevStr;
}