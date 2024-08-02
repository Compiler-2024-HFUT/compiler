/*
    After InstCombine
*/

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
        ......              // i不能被赋值
    }

    如果出现while(i rel_op (num)) {
        ......
        i = i + (num); 
        i = i + (num);
        ......
    }
    必须需要将两个i+(num)合并才能正常执行，也即基础的指令合并必须先于SCEV

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

struct SCEVVal {
    Value *sval;
    vector<SCEVVal*> operands;
    enum binType { Const, Phi, Add, Mul, Unk } op;

    void addAddVal(SCEVVal *val) {
        if(op!= binType::Add) LOG_ERROR("scev val isn't an add", 1)
        operands.push_back(val);
    }

    static SCEVVal *createUnkVal() { return new SCEVVal{nullptr, {}, binType::Unk}; }
    // 表示单独的数值，仅支持phi和整型常数
    static SCEVVal *createPhiVal(Value *phi) { 
        LOG_ERROR("Isn't a PhiInst", !dynamic_cast<PhiInst*>(phi)) 
        return new SCEVVal{phi, {}, binType::Phi}; 
    }
    static SCEVVal *createConVal(Value *con) { 
        LOG_ERROR("Isn't a ConstInt", !dynamic_cast<ConstantInt*>(con)) 
        return new SCEVVal{con, {}, binType::Const};
    }
    // 保持常数在前面
    static SCEVVal *createAddVal(Value *lhs, Value *rhs) { 
        ConstantInt *lhsInt = dynamic_cast<ConstantInt*>(lhs);
        ConstantInt *rhsInt = dynamic_cast<ConstantInt*>(rhs);
        if(lhsInt && rhsInt) {
            int sum = lhsInt->getValue() + rhsInt->getValue();
            return createConVal(ConstantInt::get(sum));
        } else if(lhsInt) {
            return new SCEVVal{nullptr, {createConVal(lhs), createPhiVal(rhs)}, binType::Add};
        } else if(rhsInt) {
            return new SCEVVal{nullptr, {createConVal(rhs), createPhiVal(lhs)}, binType::Add};
        } else {
            return new SCEVVal{nullptr, {createPhiVal(lhs), createPhiVal(rhs)}, binType::Add};
        }
    }
    static SCEVVal *createAddVal(vector<SCEVVal*> ops) {
        return new SCEVVal{nullptr, ops, binType::Add};
    }
    // mul op.size() == 2, op[0] is a const, op[1] is a phi
    static SCEVVal *createMulVal(Value *lhs, Value *rhs) { 
        ConstantInt *lhsInt = dynamic_cast<ConstantInt*>(lhs);
        ConstantInt *rhsInt = dynamic_cast<ConstantInt*>(rhs);
        if(lhsInt && rhsInt) {
            int prod = lhsInt->getValue() * rhsInt->getValue();
            return createConVal(ConstantInt::get(prod));
        } else if(lhsInt) {
            if(lhsInt->getValue() == 1)
                return createPhiVal(rhs);
            else
                return new SCEVVal{nullptr, {createConVal(lhs), createPhiVal(rhs)}, binType::Mul};
        } else if(rhsInt) {
            if(rhsInt->getValue() == 1)
                return createPhiVal(lhs);
            else
                return new SCEVVal{nullptr, {createConVal(rhs), createPhiVal(lhs)}, binType::Mul};
        } else {
            return createUnkVal();
        }
    }

    bool isAdd() { return op == binType::Add; }
    bool isMul() { return op == binType::Mul; }
    bool isUnk()  { return op == binType::Unk; }
    bool isConst()  { return op == binType::Const; }
    bool isPhi()  { return op == binType::Phi; }
    
    SCEVVal *getNegate() {
        if(this->isConst()) {
            ConstantInt *negInt = ConstantInt::get(-dynamic_cast<ConstantInt*>(sval)->getValue());
            return createConVal(negInt);
        } else if(this->isPhi()) {
            return createMulVal(ConstantInt::get(-1), sval);
        } else if(this->isMul()) {  
            ConstantInt *negInt = ConstantInt::get(-dynamic_cast<ConstantInt*>(operands[0]->sval)->getValue());
            return createMulVal(negInt, sval);
        } else if(this->isAdd()) {
            vector<SCEVVal*> newOperands;
            for(SCEVVal *op : operands) {
                newOperands.push_back(op->getNegate());
            }
            return createAddVal(newOperands);
        } else {
            return createUnkVal();
        }
    }

    SCEVVal *addSCEVVal(SCEVVal *rhs);
    SCEVVal *mulSCEVVal(SCEVVal *rhs);

    string print() {
        string str = "";

        if(this->isPhi()) {
            PhiInst *svalPhi = dynamic_cast<PhiInst*>(sval); 
            str += STRING(svalPhi->getName());
        } else if(this->isConst()) {
            ConstantInt *svalInt = dynamic_cast<ConstantInt*>(sval);        
            str += STRING_NUM(svalInt->getValue());
        } else if(this->isMul()) {
            str += "(";
            str += operands[0]->print();
            str += " * ";
            str += operands[1]->print();
            str += ")";
        } else if(this->isAdd()) {
            str += "(";
            for(SCEVVal *op : operands) {
                str += op->print();
                str += " + ";
            }
            str += "\b\b\b)";
        } else {
            str += "Unknown";
        }

        return str;
    }
    
    ~SCEVVal() {delete this;}
};

struct SCEVExpr {
    enum ExprType { Val, AddRec, Unknown };

    bool isValue()  { return type == SCEVExpr::Val; }
    bool isConst()  { return type == SCEVExpr::Val && val->isConst(); }
    bool isAddRec() { return type == SCEVExpr::AddRec; }
    bool isUnknown(){ return type == SCEVExpr::Unknown; }

    static SCEVExpr *createConst(int num, Loop *l) {
        return new SCEVExpr(SCEVExpr::Val, SCEVVal::createConVal(ConstantInt::get(num)), {}, l);
    }
    static SCEVExpr *createValue(SCEVVal *v, Loop *l) {
        return new SCEVExpr(SCEVExpr::Val, v, {}, l);
    }
    static SCEVExpr *createAddRec(vector<SCEVExpr*> op, Loop *l) {
        return new SCEVExpr(SCEVExpr::AddRec, nullptr, op, l);
    }
    static SCEVExpr *createUnknown(Loop *l) {
        return new SCEVExpr(SCEVExpr::Unknown, nullptr, {}, l);
    }

    SCEVVal *getValue() {
        if(!isValue()) LOG_ERROR("scev expr isn't an value", 1)
        return val;
    }
    int getConst() {
        if(!isValue() || !val->isConst()) LOG_ERROR("scev expr isn't a const", 1)
        return dynamic_cast<ConstantInt*>(val->sval)->getValue();
    }
    vector<SCEVExpr*> &getOperands() {
        if(!isAddRec()) LOG_ERROR("scev expr isn't an addrec", 1)
        return operands;
    }
    SCEVExpr *getOperand(int i) {
        if(!isAddRec()) LOG_ERROR("scev expr isn't an addrec", 1)
        return operands[i];
    }

    SCEVExpr *getNegate() {
        if(isUnknown()) { return createUnknown(loop); }
        if(isValue()) { return createValue(val->getNegate(), loop); }
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
        case Val:
            printStr = STRING("Value( ") + val->print() + " )";
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
    SCEVVal *val;                   // valid if type==value, 表示整数和phi的表达式，包括其单值
    vector<SCEVExpr*> operands;     // valid if type==addrec,诸如{1, +, 2}
    
    Loop *loop;

    SCEVExpr(ExprType t, SCEVVal *v, vector<SCEVExpr*> op, Loop *l) : type(t), val(v), operands(op), loop(l) { }
    ~SCEVExpr() { delete this; }
};

class SCEV: public FunctionInfo {
    umap<Loop*, umap<Value*, SCEVExpr*> > exprMapping;
    umap<Loop*, uset<PhiInst*> > loopPhis;
    vector<Loop*> loops;

    void visitLoop(Loop *loop);                // 深度优先访问嵌套循环
    void visitBlock(BasicBlock *bb, Loop *loop);
    SCEVExpr *getPhiSCEV(PhiInst *phi, Loop *loop);
public:
    SCEV(Module *m, InfoManager *im): FunctionInfo(m, im) { }
    virtual ~SCEV() { }

    SCEVExpr *getExpr(Value *v, Loop *loop) {            // 安全地获取v对应的SCEVExpr
        if(exprMapping[loop].count(v)) {
            return exprMapping[loop][v]; 
        }  
        if(dynamic_cast<ConstantInt*>(v)) {
            SCEVVal *val = SCEVVal::createConVal(v);
            exprMapping[loop][v] = SCEVExpr::createValue(val, loop);
            return exprMapping[loop][v];
        }
        if(dynamic_cast<PhiInst*>(v)) {
            SCEVVal *val = SCEVVal::createPhiVal(v);
            exprMapping[loop][v] = SCEVExpr::createValue(val, loop);
            return exprMapping[loop][v];
        }
        return nullptr;
    };         

    virtual void analyseOnFunc(Function *func) override;
    virtual string print() override;
};

#endif