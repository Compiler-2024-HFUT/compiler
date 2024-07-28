/*
    after SCCP
*/

#ifndef LOOP_INVARIANT_HPP
#define LOOP_INVARIANT_HPP

#include "analysis/Info.hpp"
#include "analysis/LoopInfo.hpp"
#include "utils/Logger.hpp"

#include <algorithm>
using std::find;

class LoopInvariant : public FunctionInfo {
    umap<Loop*, vector<Instruction*> > invariants;

    bool isInstInvariant(Loop *loop, Instruction *inst);
    void computeInvariants(Loop *loop);
public:
    bool isInvariable(Loop *loop, Value *val) {
        if(dynamic_cast<Constant*>(val))
            return true;
        else if(dynamic_cast<Instruction*>(val)) {
            vector<Instruction*> &iiset = invariants[loop];
            return ( find(iiset.begin(), iiset.end(), val ) != iiset.end() );
        } else 
            return false;
    }

    vector<Instruction*> &getInvariants(Loop *loop) {
        return invariants[loop];
    }
    
    virtual void analyseOnFunc(Function *func) override;
    virtual string print() override;

    LoopInvariant(Module *m, InfoManager *im): FunctionInfo(m, im) {}
    ~LoopInvariant() {}
};


#endif