
#ifndef MOVE_ALLOCA_HPP
#define MOVE_ALLOCA_HPP

#include "analysis/InfoManager.hpp"
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"

#include <vector>
using ::std::vector;

class MoveAlloca : public FunctionPass{
    private:
        void moveAlloc(Function*functon);

    public:
        MoveAlloca(Module *m,InfoManager*im): FunctionPass(m,im){}
        ~MoveAlloca(){};
        Modify runOnFunc(Function *function) override;
};

#endif
