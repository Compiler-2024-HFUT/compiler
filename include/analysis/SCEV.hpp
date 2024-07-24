/*
    ScalarEvolution
    https://aping-dev.com/index.php/archives/380/
    https://llvm.org/devmtg/2018-04/slides/Absar-ScalarEvolution.pdf
    https://llvm.org/devmtg/2009-10/ScalarEvolutionAndLoopOptimization.pdf

    仅分析类似下面形式的while循环：
    i = (num);              // 必须在循环前定义或赋值，中间不能隔着循环
    while(i rel_op (num)) {
        ......              // i不能被赋值 
        i = i +/- (num);
    }
    测试用例中的循环基本是这种形式
*/

#ifndef SCEV_HPP
#define SCEV_HPP

#include "analysis/Info.hpp"
#include "analysis/LoopInfo.hpp"
#include "midend/Value.hpp"
#include "midend/Constant.hpp"
#include "midend/Instruction.hpp"
#include "midend/BasicBlock.hpp"
#include "utils/Logger.hpp"

#include <vector>
#include <map>
#include <string>
using std::vector;
using std::map;
using std::string;

struct SCEVExpr {
    enum ExprType { Const, AddRec, Unknown };

    bool isConst()  { return type == SCEVExpr::Const; }
    bool isAddRec() { return type == SCEVExpr::AddRec; }
    bool isUnknown() { return type == SCEVExpr::Unknown; }

    static SCEVExpr *createConst(int c, Loop *l) {
        return new SCEVExpr(SCEVExpr::Const, c, {}, l);
    }
    static SCEVExpr *createAddRec(vector<SCEVExpr*> op, Loop *l) {
        return new SCEVExpr(SCEVExpr::AddRec, 0, op, l);
    }
    static SCEVExpr *createUnknown(Loop *l) {
        return new SCEVExpr(SCEVExpr::Unknown, 0, {}, l);
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

    SCEVExpr *getNegate() {
        if(isUnknown()) { return createUnknown(loop); }
        if(isConst()) { return createConst(-cv, loop); }
        if(isAddRec()) {
            vector<SCEVExpr*> tmp;
            for(SCEVExpr *expr : operands) {
                tmp.push_back(expr->getNegate());
            }
            return createAddRec(tmp, loop);
        }
        return createUnknown(loop);
    }

    string print() {
        string printStr = "";
        switch (type)
        {
        case Const:
            printStr = STRING("Const( ") + STRING_NUM(cv) + " )";
            break;
        case AddRec:
            printStr += "AddRec( { ";
            for(SCEVExpr *expr : operands) {
                printStr += expr->print();
                printStr += ", +, ";
            }
            printStr += "\b\b\b\b\b } )";
            break;
        case Unknown:
            printStr = "Unknown";
            break;
        }
        printStr += STRING("<") + loop->getHeader()->getName() + ">";
        return printStr;
    }

    SCEVExpr *foldAdd(SCEVExpr *expr);
    SCEVExpr *foldMul(SCEVExpr *expr);

    ExprType type;
    int cv;                         // valid if type==const, 此时只为整数
    vector<SCEVExpr*> operands;     // valid if type==addrec,诸如{1, +, 2}
    
    Loop *loop;

    SCEVExpr(ExprType t, int c, vector<SCEVExpr*> op, Loop *l) : type(t), cv(c), operands(op), loop(l) { }
    ~SCEVExpr() { delete this; }
};

class SCEV: public FunctionInfo {
    map<Value*, SCEVExpr*> exprMapping;
    vector<Loop*> loops;

    void visitLoop(Loop *loop);                // 深度优先访问嵌套循环
    void visitBlock(BasicBlock *bb, Loop *loop);
    SCEVExpr *getPhiSCEV(PhiInst *phi, Loop *loop);
public:
    SCEV(Module *m, InfoManager *im): FunctionInfo(m, im) { }
    virtual ~SCEV() { }

    SCEVExpr *getExpr(Value *v, Loop *loop) {            // 安全地获取v对应的SCEVExpr
        if(exprMapping.count(v) && exprMapping[v]->loop == loop) {
            return exprMapping[v]; 
        }  
        if(dynamic_cast<ConstantInt*>(v)) {
            exprMapping[v] = SCEVExpr::createConst(dynamic_cast<ConstantInt*>(v)->getValue(), loop);
            return exprMapping[v];
        }
        return nullptr;
    };         

    virtual void analyseOnFunc(Function *func) override;
    virtual string print() override;
};

#endif