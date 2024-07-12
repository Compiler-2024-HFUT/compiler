#include "analysis/Dataflow.hpp"
#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
#include "utils/Logger.hpp"

#include <algorithm>
using std::set_union;
using std::set_difference;

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
    }

    for(BB *bb : func_->getBasicBlocks()) {
        for(Instruction *inst : bb->getInstructions()) {
            // def
            if( !inst->isVoid() ) {
                if( inst->isPhi() ) {
                    PhiDefs[bb].insert(inst);
                } else {
                    Defs[bb].insert(inst);
                }
            }
            
            // use
            if( inst->isPhi() ) {
                for(int i=0; i<inst->getOperands().size(); i+=2) {
                    Value *op = inst->getOperand(i);
                    if (dynamic_cast<GlobalVariable *>(op) || dynamic_cast<Constant*>(op) ||
                        dynamic_cast<BasicBlock*>(op) || dynamic_cast<Function*>(op) ) continue;
                    
                    PhiUses[bb].insert( op );
                }
            } else {
                for(Value *op : inst->getOperands()) {
                    if (dynamic_cast<GlobalVariable *>(op) || dynamic_cast<Constant*>(op) ||
                        dynamic_cast<BasicBlock*>(op) || dynamic_cast<Function*>(op) ) continue;

                    if( !Defs[bb].count(op) && !PhiDefs[bb].count(op) ) {
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

        for(Value *v : PhiDefs[bb]) {
            if(v->getType()->isFloatType()) {
                liveIn[bb].second.insert(v);
            } else {
                liveIn[bb].first.insert(v);
            }
        }

        for(Value *v : UEUses[bb]) {
            if(v->getType()->isFloatType()) {
                liveIn[bb].second.insert(v);
            } else {
                liveIn[bb].first.insert(v);
            }
        }

        
        for(Value *v : PhiUses[bb]) {
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
            // int
            for(BB *succ : bb->getSuccBasicBlocks()) {
                for(Value *v : liveIn[succ].first) {
                    if(!PhiDefs[succ].count(v) && !liveOut[bb].first.count(v)) {
                        changed = true;
                        liveOut[bb].first.insert(v);

                        if(!Defs[bb].count(v)) {
                            liveIn[bb].first.insert(v);
                        }
                    }
                }
            }
            
            // float
            for(BB *succ : bb->getSuccBasicBlocks()) {
                for(Value *v : liveIn[succ].second) {
                    if(!PhiDefs[succ].count(v) && !liveOut[bb].second.count(v)) {
                        changed = true;
                        liveOut[bb].second.insert(v);

                        if(!Defs[bb].count(v)) {
                            liveIn[bb].second.insert(v);
                        }
                    }
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
            
            lvStr += '\n'; 
        }
    }
    return lvStr;
}
