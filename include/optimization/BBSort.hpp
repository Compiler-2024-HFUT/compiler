#ifndef BBSORT_HPP
#define BBSORT_HPP

#include "optimization/PassManager.hpp"

class BBSort : public FunctionPass {
    bool sortBB(Function *func);
public:
    BBSort(Module* m, InfoManager *im) : FunctionPass(m, im) { }
    ~BBSort() { }
    virtual Modify runOnFunc(Function*func);
};

#endif