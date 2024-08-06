/*
    after SCCP
*/

#ifndef LOOP_INVARIANT_HPP
#define LOOP_INVARIANT_HPP

#include "analysis/Info.hpp"
#include "analysis/LoopInfo.hpp"
#include "utils/Logger.hpp"

#include <list>
#include <algorithm>
using std::list;
using std::find;

class LoopInvariant : public FunctionInfo {
    umap<Loop*, list<Instruction*> > invariants;

    bool isValueInvariant(Loop *loop, Value *val);
    bool isInstInvariant(Loop *loop, Instruction *inst);
    void computeInvariants(Loop *loop);
public:
    bool isInvariable(Loop *loop, Value *val) {
        if(dynamic_cast<Constant*>(val))
            return true;
        else if(dynamic_cast<Instruction*>(val)) {
            list<Instruction*> &iiset = invariants[loop];
            return ( find(iiset.begin(), iiset.end(), val ) != iiset.end() );
        } else 
            return false;
    }

    list<Instruction*> &getInvariants(Loop *loop) {
        return invariants[loop];
    }
    
    virtual void analyseOnFunc(Function *func) override;
    virtual string print() override;

    LoopInvariant(Module *m, InfoManager *im): FunctionInfo(m, im) {}
    ~LoopInvariant() {}
};


#endif