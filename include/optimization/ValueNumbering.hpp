#ifndef  VAL_NUM_HPP
#define VAL_NUM_HPP
#include "midend/Module.hpp"
#include "optimization/PassManager.hpp"
struct Expr{
    enum class ExprOp:int32_t{
        ILLEGAL=-1,EMPTY=0,
        ADD,SUB,MUL,DIV,FADD,FSUB,FMUL,FDIV,REM,
        ZEXT,SITOFP,FPTOSI,
    }op_;
    uint32_t first_vn_,second_vn_,unuse;
    Type* type_; 
    
    bool operator==(Expr const &other)const{
        if(this->op_!=other.op_||type_!=other.type_||first_vn_!=other.first_vn_||second_vn_!=other.second_vn_||unuse!=other.unuse)
            return false;
        return true;
    }
    bool operator!=(Expr const &other)const{
        return !((*this)==other);
    }
    bool operator<(Expr const &other)const{
        if(this->op_<other.op_||type_->getTypeId()<other.type_->getTypeId()||first_vn_<other.first_vn_||second_vn_<other.second_vn_||unuse<other.unuse)
            return true;
        return false;
    }
    Expr():op_(ExprOp::EMPTY),unuse(0){}
    Expr(ExprOp _op,Type*type,uint32_t first=0,uint32_t second=0):op_(_op),type_(type),first_vn_(first),second_vn_(second),unuse(0){}

public:
    static ExprOp instop2exprop(Instruction::OpID instrop);
};
struct ValueTable{
    uint32_t next_num=1;
    ::std::map<Value*,uint32_t> value_hash;
    ::std::map<Expr,uint32_t> expressing_hash;
    ::std::vector<Value*> number_value{0};
public:
    Expr creatExpr(BinaryInst* bin);
    Expr creatExpr(FpToSiInst* ins);
    Expr creatExpr(SiToFpInst* ins);
    Expr creatExpr(ZextInst* ins);
    Value* getNumVal(uint32_t num){
        return  number_value[num];
    }
    uint32_t getValueNum(Value*v);
    void clear(){
        next_num=1;
        value_hash.clear();
        expressing_hash.clear();
        number_value.clear();
        number_value.push_back(0);
    }
};

class ValNumbering:public FunctionPass{
    ValueTable vn_table_;
    ::std::map<BasicBlock*,::std::set<Value*>> basic_value_;
    __attribute__((__always_inline__)) void clear(){
        vn_table_.clear();
        basic_value_.clear();
    }
    bool proInstr(Instruction*instr);
public:
    virtual void runOnFunc(Function *func) override;
    // using FunctionPass::FunctionPass;
    ValNumbering(Module *m) : FunctionPass(m){}
    ~ValNumbering(){};
    // virtual ~ValNumbering(){}

};
#endif