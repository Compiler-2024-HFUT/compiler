#include <cassert>
#include <algorithm>
#include <map>

#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/IRprint.hpp"
#include "midend/Instruction.hpp"

BasicBlock::BasicBlock(const std::string &name, Function *parent = nullptr)
    : Value(Type::getLabelType(), name), parent_(parent) {
    assert(parent && "currently parent should not be nullptr");
    parent_->addBasicBlock(this);
}

BasicBlock *BasicBlock::create( const std::string &name, Function *parent) {
    auto prefix = name.empty() ? "" : "label_";
    return new BasicBlock(prefix + name, parent);
}

Module *BasicBlock::getModule() {
    return parent_->getParent();
}

const Instruction *BasicBlock::getTerminator() const {
    if (instr_list_.empty()) {
        return nullptr;
    }
    switch (instr_list_.back()->getInstrType()) {
        case Instruction::OpID::ret: 
            return instr_list_.back();
        case Instruction::OpID::br: 
            return instr_list_.back();
        case Instruction::OpID::cmpbr:
            return instr_list_.back();
        case Instruction::OpID::fcmpbr:
            return instr_list_.back();
        default: 
            return nullptr;
    }
}

void BasicBlock::addInstruction(Instruction *instr) {
    instr_list_.push_back(instr);
}

void BasicBlock::addInstruction(std::list<Instruction*>::iterator instr_pos, Instruction *instr) {
    instr_list_.insert(instr_pos, instr);
}
void BasicBlock::addInstrBegin(Instruction *instr) {
    instr_list_.push_front(instr);
}

void BasicBlock::addInstrBeforeTerminator(Instruction *instr) {
    Instruction *term = instr_list_.back();
    instr_list_.pop_back();

    switch (term->getInstrType()) {
        case Instruction::OpID::ret:
        case Instruction::OpID::cmpbr:
        case Instruction::OpID::fcmpbr:
            instr_list_.push_back(instr);
            instr_list_.push_back(term);
        break;

        case Instruction::OpID::br:
            if(term->getOperands().size() == 0) {
            // 无条件跳转
                instr_list_.push_back(instr);
                instr_list_.push_back(term);
            } else {
            // 条件跳转，判断前一条指令是否为cmp
                Instruction *cmp = instr_list_.back();
                if(dynamic_cast<CmpInst*>(cmp) || dynamic_cast<FCmpInst*>(cmp)) {
                    instr_list_.pop_back();
                    instr_list_.push_back(instr);
                    instr_list_.push_back(cmp);
                    instr_list_.push_back(term);
                } else {
                    instr_list_.push_back(instr);
                    instr_list_.push_back(term);
                }
            }

        default: assert(0 && "Unknown Terminator!");
    }
}

void BasicBlock::deleteInstr(Instruction *instr) {
    instr_list_.erase(findInstruction(instr));
    instr->removeUseOfOps();
    // dead.insert(instr);
}
::std::list<Instruction*>::iterator BasicBlock::eraseInstr(::std::list<Instruction*>::iterator instr_iter) {
    auto instr=*instr_iter;
    //new iter
    auto ret=instr_list_.erase(instr_iter);
    instr->removeUseOfOps();
    // dead.insert(instr);
    return ret;
}

::std::list<Instruction*>::iterator BasicBlock::insertInstr(::std::list<Instruction*>::iterator instr_iter,Instruction* instr) {
    return instr_list_.insert(instr_iter,instr);
}
std::list<Instruction*>::iterator BasicBlock::findInstruction(Instruction *instr) {
    return std::find(instr_list_.begin(), instr_list_.end(), instr);
}

void BasicBlock::eraseFromParent() { 
    this->getParent()->removeBasicBlock(this); 
}

std::string BasicBlock::print() {
    std::string bb_ir;
    bb_ir += this->getName();
    bb_ir += ":";
    //// print prebb
    if (!this->getPreBasicBlocks().empty()) {
        bb_ir += "                                                ; preds = ";
    }
    for (auto bb : this->getPreBasicBlocks()) {
        if (bb != *this->getPreBasicBlocks().begin())
            bb_ir += ", ";
        bb_ir += printAsOp(bb, false);
    }

    //// print prebb
    if (!this->getParent()) {
        bb_ir += "\n";
        bb_ir += "; Error: Block without parent!";
    }
    bb_ir += "\n";
    for (auto &instr : this->getInstructions()) {
        bb_ir += "  ";
        bb_ir += instr->print();
        bb_ir += "\n";
    }

    return bb_ir;
}

// 复制完后的BB无法直接使用，以下内容需要进一步替换：
// 1. PreBB、SuccBB仍是原来的BB的PreBB、SuccBB
// 2. phi指令、跳转指令br、cmpbr、fcmpbr的目标
// 3. 所有指令中不是定义在BB中的op
BasicBlock *BasicBlock::copyBB() {
    BasicBlock *newBB = BasicBlock::create("", this->getParent());

    std::map<Instruction*, Instruction*> instMap = {};
    for(Instruction *inst : instr_list_) {
        Instruction *newInst = inst->copyInst(newBB);
        instMap.insert({inst, newInst});
    }

    for(Instruction *newInst : newBB->getInstructions()) {
        std::vector<Value*> &ops = newInst->getOperands();
        for(int i = 0; i < ops.size(); i++) {
            Instruction *opI = dynamic_cast<Instruction*>(ops[i]);
            if(opI && instMap[opI]) {
                newInst->replaceOperand(i, instMap[opI]);
            }
        }
    }

    for(BasicBlock *bb : getPreBasicBlocks())
        newBB->addPreBasicBlock(bb);
    for(BasicBlock *bb : getSuccBasicBlocks())
        newBB->addSuccBasicBlock(bb);

    return newBB;
}