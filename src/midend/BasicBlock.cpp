#include <cassert>
#include <algorithm>

#include "../../include/midend/BasicBlock.hpp"
#include "../../include/midend/Function.hpp"
#include "../../include/midend/IRprint.hpp"

BasicBlock::BasicBlock(Module *m, const std::string &name = "", Function *parent = nullptr)
    : Value(Type::getLabelType(m), name), parent_(parent) {
    assert(parent && "currently parent should not be nullptr");
    parent_->addBasicBlock(this);
}

BasicBlock *BasicBlock::create(Module *m, const std::string &name, Function *parent) {
    auto prefix = name.empty() ? "" : "label_";
    return new BasicBlock(m, prefix + name, parent);
}

Module *BasicBlock::getModule() {
    return getParent()->getParent();
}

const Instruction *BasicBlock::getTerminator() const {
    if (instr_list_.empty()) {
        return nullptr;
    }
    switch (instr_list_.back()->getInstrType()) {
        case Instruction::ret: 
            return instr_list_.back();
        case Instruction::br: 
            return instr_list_.back();
        case Instruction::cmpbr:
            return instr_list_.back();
        case Instruction::fcmpbr:
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

void BasicBlock::deleteInstr(Instruction *instr) {
    instr_list_.remove(instr);
    instr->removeUseOfOps();
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