#include "analysis/Dataflow.hpp"
#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "utils/Logger.hpp"

void LiveVar::analyseOnFunc(Function *func_) {
    initUseAndDef(func_);
    findFixedPoint(func_);
}

void LiveVar::initUseAndDef(Function *func_) {
    for(BB *bb : func_->getBasicBlocks()) {
        Defs[bb] = {};
        UEUses[bb] = {};
        PhiDefs[bb] = {}; 
        PhiUses[bb] = {}; 

        for(Instruction *inst : bb->getInstructions()) {
            // def
            if( !inst->isVoid() ) {
                Defs[bb].insert(inst);
                if( inst->isPhi() ) {
                    PhiDefs[bb].insert(inst);
                }
            }
            
            // UEUses
            if( !inst->isPhi() ) {
                for(Value *op : inst->getOperands()) {
                    if (dynamic_cast<GlobalVariable *>(op) || dynamic_cast<Constant*>(op) ||
                        dynamic_cast<BasicBlock*>(op) || dynamic_cast<Function*>(op) ) continue;

                    if( !Defs[bb].count(op) ) {
                        UEUses[bb].insert(op);
                    }
                }
            }
        }
    }
}

void LiveVar::findFixedPoint(Function *func_) {
    for(BB *bb : func_->getBasicBlocks()) {
        liveIn[bb] = {{}, {}};
        liveOut[bb] = {{}, {}};

        for(Value *v : UEUses[bb]) {
            if(v->getType()->isFloatType()) {
                liveIn[bb].second.insert(v);
            } else {
                liveIn[bb].first.insert(v);
            }
        }
    }
    
    bool changed = true;
    while(changed) {
        changed = false;
        for(BB *bb : func_->getBasicBlocks()) {
            for(BB *succ : bb->getSuccBasicBlocks()) {
                // int
                for(Value *v : liveIn[succ].first) {
                    if(!PhiDefs[succ].count(v) && !liveOut[bb].first.count(v)) {
                        changed = true;
                        liveOut[bb].first.insert(v);

                        if( !Defs[bb].count(v) ) {
                            liveIn[bb].first.insert(v);
                        }
                    }
                }
            
                // float
                for(Value *v : liveIn[succ].second) {
                    if(!PhiDefs[succ].count(v) && !liveOut[bb].second.count(v)) {
                        changed = true;
                        liveOut[bb].second.insert(v);

                        if( !Defs[bb].count(v) ) {
                            liveIn[bb].second.insert(v);
                        }
                    }
                }

                // PhiUse
                for(Value *succPhiV : PhiDefs[succ]) {
                    Instruction *succPhiI = dynamic_cast<PhiInst*>(succPhiV);
                    for(int i = 0; i < succPhiI->getNumOperands(); i += 2) {
                        BasicBlock *pre = dynamic_cast<BasicBlock*>(succPhiI->getOperand(i+1));
                        Value *op = succPhiI->getOperand(i);

                        if(op->getType()->isFloatType()) {
                            if(!liveOut[bb].second.count(op) && pre == bb) {
                                changed = true;
                                liveOut[bb].second.insert(op);
                                if( !Defs[bb].count(op) ) {
                                    liveIn[bb].second.insert(op);
                                }
                            }
                        } else {
                            if(!liveOut[bb].first.count(op) &&  pre == bb) {
                                changed = true;
                                liveOut[bb].first.insert(op);
                                if( !Defs[bb].count(op) ) {
                                    liveIn[bb].first.insert(op);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for(BasicBlock *bb : func_->getBasicBlocks()) {
        for(Value *phiV : PhiDefs[bb]) {
            Instruction *phiI = dynamic_cast<PhiInst*>(phiV);
            for(int i = 0; i < phiI->getNumOperands(); i += 2) {
                Value *op = phiI->getOperand(i);
                if(op->getType()->isFloatType()) {
                    liveIn[bb].second.erase(op);
                } else {
                    liveIn[bb].first.erase(op);
                }
            }
        }
    }

}

string LiveVar::print() {
    string lvStr = "";
    module_->print();
    for(Function *f : module_->getFunctions()) {
        for(BB *bb : f->getBasicBlocks()) {
            lvStr += bb->print();
            
            lvStr += STRING_RED(";LiveIn of INT:\n;");
            for(Value *v : liveIn[bb].first) {
                lvStr += STRING_YELLOW(v->getName());  lvStr += ' '; 
            }
            lvStr += STRING_RED("\n;LiveIn of FLOAT:\n;");
            for(Value *v : liveIn[bb].second) {
                lvStr += STRING_YELLOW(v->getName());  lvStr += ' ';
            }
            lvStr += STRING_RED("\n;LiveOut of INT:\n;");
            for(Value *v : liveOut[bb].first) {
                lvStr += STRING_YELLOW(v->getName());  lvStr += ' ';
            }
            lvStr += STRING_RED("\n;LiveOut of FLOAT:\n;");
            for(Value *v : liveOut[bb].second) {
                lvStr += STRING_YELLOW(v->getName());  lvStr += ' ';
            }

            /*
            lvStr += STRING_RED("\n;Defs:\n;");
            for(Value *v : Defs[bb]) {
                lvStr += STRING_YELLOW(v->getName());  lvStr += ' ';
            }
            lvStr += STRING_RED("\n;UEUses:\n;");
            for(Value *v : UEUses[bb]) {
                lvStr += STRING_YELLOW(v->getName());  lvStr += ' ';
            }
            lvStr += STRING_RED("\n;PhiDefs:\n;");
            for(Value *v : PhiDefs[bb]) {
                lvStr += STRING_YELLOW(v->getName());  lvStr += ' ';
            }
            lvStr += STRING_RED("\n;PhiUses:\n;");
            for(Value *v : PhiUses[bb]) {
                lvStr += STRING_YELLOW(v->getName());  lvStr += ' ';
            }
            */
            lvStr += '\n'; 
        }
    }
    return lvStr;
}
