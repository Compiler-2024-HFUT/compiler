/*
    ScalarEvolution
    https://aping-dev.com/index.php/archives/380/
    https://llvm.org/devmtg/2018-04/slides/Absar-ScalarEvolution.pdf
    https://llvm.org/devmtg/2009-10/ScalarEvolutionAndLoopOptimization.pdf
*/

#ifndef SCEV_HPP
#define SCEV_HPP

#include "analysis/Info.hpp"
#include "analysis/LoopInfo.hpp"
#include "midend/Value.hpp"
#include "midend/Constant.hpp"
#include "utils/Logger.hpp"

#include <vector>
#include <map>
using std::vector;
using std::map;

struct SCEVExpr {
    enum ExprType { Const, Expr, AddRec, Unknown };

    bool isConst()  { return type == SCEVExpr::Const; }
    bool isExpr()   { return type == SCEVExpr::Expr; }
    bool isAddRec() { return type == SCEVExpr::AddRec; }
    bool isUnknown() { return type == SCEVExpr::Unknown; }

    static SCEVExpr *createConst(int c, Loop *l) {
        return new SCEVExpr(Const, c, nullptr, {}, l);
    }
    static SCEVExpr *createExpr(Value *v, Loop *l) {
        return new SCEVExpr(Expr, 0, v, {}, l);
    }
    static SCEVExpr *createAddRec(vector<SCEVExpr*> op, Loop *l) {
        return new SCEVExpr(AddRec, 0, nullptr, op, l);
    }
    static SCEVExpr *createUnknown(Loop *l) {
        return new SCEVExpr(Unknown, 0, nullptr, {}, l);
    }

    int getConst() {
        if(!isConst()) LOG_ERROR("scev expr isn't a const", 1)
        return cv;
    }
    const vector<SCEVExpr*> &getOperands() {
        if(!isAddRec()) LOG_ERROR("scev expr isn't an addrec", 1)
        return operands;
    }
    SCEVExpr *getOperand(int i) {
        if(!isAddRec()) LOG_ERROR("scev expr isn't an addrec", 1)
        return operands[i];
    }

    SCEVExpr *foldAdd(SCEVExpr *expr);
    SCEVExpr *foldMul(SCEVExpr *expr);

    ExprType type;
    int cv;                         // valid if type==const, 此时只为整数
    Value *val;                     // valid if type==expr,  此时可以是Inst(二元加减、乘法)，
    vector<SCEVExpr*> operands;     // valid if type==addrec,诸如{1, +, 2}
    
    Loop *loop;

    SCEVExpr(ExprType t, int c, Value *v, vector<SCEVExpr*> op, Loop *l) : type(t), cv(c), val(v), operands(op), loop(l) { }
    ~SCEVExpr() { delete this; }
};

class SCEV: public FunctionInfo {
    // map<Value*, SCEVExpr*> exprMapping;

public:
    SCEV(Module *m, InfoManager *im): FunctionInfo(m, im) { }
    virtual ~SCEV() { }

    void getExpr(Value *v);         // 获取v对应的SCEVExpr

    virtual void analyseOnFunc(Function *func) override;
    virtual string print() override;
};

#endif