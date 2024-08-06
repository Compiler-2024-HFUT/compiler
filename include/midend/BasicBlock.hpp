#ifndef BASICBLOCK_HPP
#define BASICBLOCK_HPP

#include <list>
#include <algorithm>

#include "Value.hpp"

class Function;
class Instruction;
class Module;
class IRVisitor;
class BasicBlock : public Value {
public:
    static BasicBlock *create(const std::string &name, Function *parent);

    //// return parent or null(if none)
    Function *getParent() { return parent_; }

    Module *getModule();

    //& cfg api begin
    std::list<BasicBlock *>& getPreBasicBlocks() { return pre_bbs_; }
    std::list<BasicBlock *>& getSuccBasicBlocks() { return succ_bbs_; } 

    void addPreBasicBlock(BasicBlock *bb) { pre_bbs_.push_back(bb);}
    void addSuccBasicBlock(BasicBlock *bb) { succ_bbs_.push_back(bb);}

    void removePreBasicBlock(BasicBlock *bb) { pre_bbs_.remove(bb); }
    void removeSuccBasicBlock(BasicBlock *bb) { succ_bbs_.remove(bb); }

    void loopDepthReset() { loop_depth = 0; }
    int getLoopDepth() { return loop_depth; }
    void loopDepthAdd(int num) { loop_depth += num; }

    //// Returns the terminator instruction if the block is well formed or null
    //// if the block is not well formed.
    const Instruction *getTerminator() const;
    Instruction *getTerminator() {
      return const_cast<Instruction *>(
          static_cast<const BasicBlock *>(this)->getTerminator()
      );
    }

    void setInstructionsList(std::list<Instruction*> &insts_list) {
        instr_list_.clear();
        instr_list_.assign(insts_list.begin(), insts_list.end());
    }
  
    //// add instruction
    __attribute__((always_inline)) void addInstruction(Instruction *instr) {instr_list_.push_back(instr);}
    __attribute__((always_inline)) void addInstruction(std::list<Instruction*>::iterator instr_pos, Instruction *instr) {instr_list_.insert(instr_pos, instr);}
    __attribute__((always_inline)) void addInstrBegin(Instruction *instr) {instr_list_.push_front(instr);}
    void addInstrBeforeTerminator(Instruction *instr);
    void addInstrAfterPhiInst(Instruction *instr);

    void deleteInstr(Instruction *instr);
    ::std::list<Instruction*>::iterator eraseInstr(::std::list<Instruction*>::iterator instr_iter);
    ::std::list<Instruction*>::iterator insertInstr(::std::list<Instruction*>::iterator instr_iter,Instruction*instr);

    __attribute__((always_inline)) std::list<Instruction *>::iterator findInstruction(Instruction *instr){return std::find(instr_list_.begin(), instr_list_.end(), instr);}
    void replaceInsWith(Instruction* old_ins,Instruction* new_ins);

    bool empty() { return instr_list_.empty(); }
    int getNumOfInstrs() { return instr_list_.size(); }
    std::list<Instruction *>& getInstructions() { return instr_list_; }

    void eraseFromParent();

    BasicBlock *copyBB();



    //后端遍历
    virtual void accept(IRVisitor &visitor) final;

private:
    explicit BasicBlock(const std::string &name, Function *parent);
    
private:  
    std::list<BasicBlock *> pre_bbs_;
    std::list<BasicBlock *> succ_bbs_;
    std::list<Instruction *> instr_list_;
    Function *parent_;
   // int incoming_branch = 0;
    int loop_depth = 0;

};
#endif
