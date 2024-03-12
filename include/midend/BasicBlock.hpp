//BB代表一个基本快，BBs代表若干个基本快
#ifndef BASICBLOCK_HPP
#define BASICBLOCK_HPP

#include <list>
#include <iterator>
#include <set>

#include "Value.hpp"




class Function;
class Instruction;
class Module;

class BasicBlock : public Value {
public:
    static BasicBlock *create(Module *m, const std::string &name, Function *parent);

    //// return parent or null(if none)
    Function *getParent() { return parent; }

    Module *getModule();

    //& cfg api begin
    std::list<BasicBlock *>& getPreBBs() { return preBBs; }
    std::list<BasicBlock *>& getSuccBBs() { return succBBs; } 

    void addPreBB(BasicBlock *bb) { preBBs.push_back(bb);}
    void addSuccBB(BasicBlock *bb) { succBBs.push_back(bb);}

    void removePreBB(BasicBlock *bb) { preBBs.remove(bb); }
    void removeSuccBB(BasicBlock *bb) { succBBs.remove(bb); }
    void incomingReset() { incomingBranch = 0; }
    bool isIncomingZero() { return incomingBranch == 0; }
    void incomingAdd(int num) { incomingBranch += num; }
    void incomingDecrement(){incomingBranch--;}
    int getIncomingBranch() { return incomingBranch; }
    void loopDepthReset() { loopDepth = 0; }
    int getLoopDepth() { return loopDepth; }
    void loopDepthAdd(int num) { loopDepth += num; }
    //& cfg api end

    //& dominate tree api begin
    void setLiveInInt(std::set<Value*> in){iliveIn = in;}
    void setLiveOutInt(std::set<Value*> out){iliveOut = out;}
    void setLiveInFloat(std::set<Value*> in){fliveIn = in;}
    void setLiveOutFloat(std::set<Value*> out){fliveOut = out;}
    std::set<Value*>& getLiveInInt(){return iliveIn;}
    std::set<Value*>& getLiveOutInt(){return iliveOut;}
    std::set<Value*>& getLiveInFloat(){return fliveIn;}
    std::set<Value*>& getLiveOutFloat(){return fliveOut;}
    //& dominate tree api end

    //& dominates frontier api begin
    void setIdom(BasicBlock* bb){idom = bb;}
    BasicBlock* getIdom(){return idom;}
    void addDomFrontier(BasicBlock* bb){domFrontier.insert(bb);}
    void addRdomFrontier(BasicBlock* bb){rdomFrontier.insert(bb);}
    void clearrdomfrontier(){rdomFrontier.clear();}
    std::set<BasicBlock *> &getDomFrontier(){return domFrontier;}
    std::set<BasicBlock *> &getRdomFrontier(){return rdomFrontier;}
    std::set<BasicBlock *> &getRdoms(){return rdoms;}
    auto addrdom(BasicBlock* bb){return rdoms.insert(bb);}
    void clearrdom(){rdoms.clear();}
    //& dominates frontier api end

    //// Returns the terminator instruction if the block is well formed or null
    //// if the block is not well formed.
    const Instruction *getTerminator() const;
    Instruction *getTerminator() {
      return const_cast<Instruction *>(
          static_cast<const BasicBlock *>(this)->getTerminator()
      );
    }

    std::list<Instruction*>::iterator begin() { return instrList.begin(); }
    std::list<Instruction*>::iterator end() { return instrList.end(); }
    std::list<Instruction*>::iterator getTerminatorItr() {
        auto itr = end();
        itr--;
        return itr;
    }

    void setInstructionsList(std::list<Instruction*> &instslist) {
        instrList.clear();
        instrList.assign(instslist.begin(), instslist.end());
    }
  
    //// add instruction
    void addInstruction(Instruction *instr);
    void addInstruction(std::list<Instruction *>::iterator instrpos, Instruction *instr);
    void addInstrBegin(Instruction *instr);

    void deleteinstr(Instruction *instr);

    std::list<Instruction *>::iterator findInstruction(Instruction *instr);

    bool empty() { return instrList.empty(); }
    int getNumoOfInstrs() { return instrList.size(); }
    std::list<Instruction *>& getInstructions() { return instrList; }

    void eraseFromParent();

    void domFrontierReset() {
        domFrontier.clear();
    }

    virtual std::string print() override;


private:
    explicit BasicBlock(Module *m, const std::string &name, Function *parent);
    
private:  
    std::list<BasicBlock *> preBBs;
    std::list<BasicBlock *> succBBs;
    std::list<Instruction *> instrList;
    BasicBlock* idom = nullptr;
    std::set<BasicBlock *> domFrontier;   //& domtree
    std::set<BasicBlock*> rdomFrontier;
    std::set<BasicBlock*> rdoms;
    Function *parent;
    std::set<Value*> iliveIn;
    std::set<Value*> iliveOut;
    std::set<Value*> fliveIn;
    std::set<Value*> fliveOut;
    int incomingBranch = 0;
    int loopDepth = 0;
};



#endif