#ifndef BASICBLOCK_HPP
#define BASICBLOCK_HPP

#include <list>
#include <iterator>
#include <set>
#include <unordered_set>

#include "Value.hpp"
#include "Instruction.hpp"


class Function;
class Instruction;
class Module;

class BasicBlock : public Value {
public:
    static BasicBlock *create(Module *m, const std::string &name, Function *parent);

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
    void incomingReset() { incoming_branch = 0; }
    bool isIncomingZero() { return incoming_branch == 0; }
    void incomingAdd(int num) { incoming_branch += num; }
    void incomingDecrement(){incoming_branch--;}
    int getIncomingBranch() { return incoming_branch; }
    void loopDepthReset() { loop_depth = 0; }
    int getLoopDepth() { return loop_depth; }
    void loopDepthAdd(int num) { loop_depth += num; }
    //& cfg api end

    //& dominate tree api begin
    void setLiveInInt(std::set<Value*> in){ilive_in = in;}
    void setLiveOutInt(std::set<Value*> out){ilive_out = out;}
    void setLiveInFloat(std::set<Value*> in){flive_in = in;}
    void setLiveOutFloat(std::set<Value*> out){flive_out = out;}
    std::set<Value*>& getLiveInInt(){return ilive_in;}
    std::set<Value*>& getLiveOutInt(){return ilive_out;}
    std::set<Value*>& getLiveInFloat(){return flive_in;}
    std::set<Value*>& getLiveOutFloat(){return flive_out;}
    //& dominate tree api end

    //& dominates frontier api begin
    void setIdom(BasicBlock* bb){idom_ = bb;}
    BasicBlock* getIdom(){return idom_;}
    void addDomFrontier(BasicBlock* bb){dom_frontier_.insert(bb);}
    void addRDomFrontier(BasicBlock* bb){rdom_frontier_.insert(bb);}
    void clearRDomFrontier(){rdom_frontier_.clear();}
    std::set<BasicBlock *> &getDomFrontier(){return dom_frontier_;}
    std::set<BasicBlock *> &getRDomFrontier(){return rdom_frontier_;}
    std::set<BasicBlock *> &getRDoms(){return rdoms_;}
    auto addRDom(BasicBlock* bb){return rdoms_.insert(bb);}
    void clearRDom(){rdoms_.clear();}
    //& dominates frontier api end

    //// Returns the terminator instruction if the block is well formed or null
    //// if the block is not well formed.
    const Instruction *getTerminator() const;
    Instruction *getTerminator() {
      return const_cast<Instruction *>(
          static_cast<const BasicBlock *>(this)->getTerminator()
      );
    }

    // std::list<Instruction*>::iterator begin() { return instr_list_.begin(); }
    // std::list<Instruction*>::iterator end() { return instr_list_.end(); }
    // std::list<Instruction*>::iterator getTerminatorItr() {
    //     auto itr = end();
    //     itr--;
    //     return itr;
    // }

    void setInstructionsList(std::list<Instruction*> &insts_list) {
        instr_list_.clear();
        instr_list_.assign(insts_list.begin(), insts_list.end());
    }
  
    //// add instruction
    void addInstruction(Instruction *instr);
    void addInstruction(std::list<Instruction *>::iterator instr_pos, Instruction *instr);
    void addInstrBegin(Instruction *instr);

    void deleteInstr(Instruction *instr);
    void eraseInstr(::std::list<Instruction*>::iterator instr_iter);

    std::list<Instruction *>::iterator findInstruction(Instruction *instr);

    bool empty() { return instr_list_.empty(); }
    int getNumOfInstrs() { return instr_list_.size(); }
    std::list<Instruction *>& getInstructions() { return instr_list_; }

    void eraseFromParent();

    void domFrontierReset() {
        dom_frontier_.clear();
    }

    virtual std::string print() override;

    ~BasicBlock(){
        for(auto ins:dead){
            delete ins;
        }
    }

private:
    explicit BasicBlock(Module *m, const std::string &name, Function *parent);
    
private:  
    std::list<BasicBlock *> pre_bbs_;
    std::list<BasicBlock *> succ_bbs_;
    std::list<Instruction *> instr_list_;
    BasicBlock* idom_ = nullptr;
    std::set<BasicBlock *> dom_frontier_;   //& dom_tree
    std::set<BasicBlock*> rdom_frontier_;
    std::set<BasicBlock*> rdoms_;
    Function *parent_;
    std::set<Value*> ilive_in;
    std::set<Value*> ilive_out;
    std::set<Value*> flive_in;
    std::set<Value*> flive_out;
    int incoming_branch = 0;
    int loop_depth = 0;

    std::unordered_set<Instruction*>dead;
};
#endif
