#ifndef  VAL_NUM_HPP
#define VAL_NUM_HPP
#include "analysis/Dominators.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "midend/Value.hpp"
#include "optimization/PassManager.hpp"
struct Expr{
    enum ExprOp:int32_t{
        ILLEGAL=-1,EMPTY=0,
        ADD,SUB,MUL,DIV,FADD,FSUB,FMUL,FDIV,REM,
        //预防与cmpop冲突
        EXPR_EQ,EXPR_NE,EXPR_GT,EXPR_GE,EXPR_LT,EXPR_LE,
        ZEXT,SITOFP,FPTOSI,
        AND,OR,XOR,
        ASR,SHL,LSR,ASR64,SHL64,LSR64,
        GEP,
    }op_;
    uint32_t lhs,rhs,third;
    Type* type_;

    bool operator==(Expr const &other)const{
        if(this->op_!=other.op_||type_!=other.type_||lhs!=other.lhs||rhs!=other.rhs||third!=other.third)
            return false;
        return true;
    }
    bool operator!=(Expr const &other)const{
        return !((*this)==other);
    }
    bool operator<(Expr const &other)const{
        if(op_<other.op_)return true;
        if(op_>other.op_) return false;
        if(lhs<other.lhs)return true;
        if(lhs>other.lhs)return false;
        if(rhs<other.rhs)return true;
        if(rhs>other.rhs)return false;
        if(third<other.third)return true;
        if(third>other.third)return false;
        return type_<other.type_;
    }
    Expr():op_(ExprOp::EMPTY),type_(nullptr),lhs(0),rhs(0),third(0){}
    Expr(ExprOp _op,Type*type,uint32_t first,uint32_t second=0,uint32_t third=0):op_(_op),type_(type),lhs(first),rhs(second),third(third){}

public:
    static ExprOp instop2exprop(Instruction::OpID instrop);
};
struct ValueTable{
    uint32_t next_num=1;
    ::std::map<Value*,uint32_t> value_hash;
    ::std::map<Expr,uint32_t> expressing_hash;
    ::std::vector<::std::vector<Value*>> number_value;
public:
    Expr creatExpr(BinaryInst* bin);
    Expr creatExpr(FpToSiInst* ins);
    Expr creatExpr(SiToFpInst* ins);
    Expr creatExpr(ZextInst* ins);
    Expr creatExpr(CmpInst* ins);
    Expr creatExpr(FCmpInst* ins);
    Expr creatExpr(GetElementPtrInst* ins);
    ::std::vector<Value*>& getNumVal(uint32_t num){
        return  number_value[num];
    }
    uint32_t getValueNum(Value*v);
    void clear(){
        next_num=1;
        value_hash.clear();
        expressing_hash.clear();
        number_value.clear();
        number_value.push_back({});
    }
};

class ValNumbering:public FunctionPass{
    ValueTable vn_table_;
    Dominators*dom;
    // ::std::map<BasicBlock*,::std::set<Value*>> basic_value_;
    __attribute__((__always_inline__)) void clear(){
        vn_table_.clear();
        // basic_value_.clear();
    }
    bool proInstr(Instruction*instr);
public:
    bool dvnt(Function*func,BasicBlock*bb);
    virtual void runOnFunc(Function *func) override;
    using FunctionPass::FunctionPass;
    void init()override{
        dom=info_man_->getInfo<Dominators>();
    }
    ~ValNumbering(){};
    // virtual ~ValNumbering(){}

};
#endif
