#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Value.hpp"
#include <cstdint>
#include <map>
#include <vector>
#include "optimization/ValueNumbering.hpp"
Expr::ExprOp Expr::instop2exprop(Instruction::OpID instrop){
    static const std::map<Instruction::OpID,ExprOp> i_e{
        {Instruction::OpID::add,ExprOp::ADD},
        {Instruction::OpID::fadd,ExprOp::FADD},
        
        {Instruction::OpID::sub,ExprOp::SUB},
        {Instruction::OpID::fsub,ExprOp::FSUB},
        
        {Instruction::OpID::mul,ExprOp::MUL},
        {Instruction::OpID::fmul,ExprOp::FMUL},
        
        {Instruction::OpID::sdiv,ExprOp::DIV},
        {Instruction::OpID::fdiv,ExprOp::FDIV},
        {Instruction::OpID::srem,ExprOp::REM},

        {Instruction::OpID::sitofp,ExprOp::SITOFP},
        {Instruction::OpID::fptosi,ExprOp::FPTOSI},
        {Instruction::OpID::zext,ExprOp::ZEXT},
    };
    auto iter=i_e.find(instrop);
    if(iter==i_e.end())
        return ExprOp::ILLEGAL;
    return iter->second;
}
uint32_t ValueTable::getValueNum(Value*v){
    {
        auto iter=value_hash.find(v);
        if(iter!=value_hash.end())
            return iter->second;
    }
    Expr e{};
    if(auto bin=dynamic_cast<BinaryInst*>(v)){
        e=creatExpr(bin);
    }else if(auto f2i=dynamic_cast<FpToSiInst*>(v)){
        e=creatExpr(f2i);
    }else if(auto i2f=dynamic_cast<SiToFpInst*>(v)){
        e=creatExpr(i2f);
    }else if(auto zext=dynamic_cast<ZextInst*>(v)){
        e=creatExpr(zext);
    }
    if(e.op_!=Expr::ExprOp::EMPTY){
        if(auto iter=expressing_hash.find(e);iter!=expressing_hash.end()){
            return iter->second;
        }else{
            value_hash.insert({v,next_num});
            expressing_hash.insert({e,next_num});
            number_value.push_back(v);
            ++next_num;
            return next_num-1;
        }
    }
    value_hash.insert({v,next_num});
    number_value.push_back(v);
    ++next_num;
    return next_num-1;
}
void ValNumbering::runOnFunc(Function*func){
    clear();
    if(func->getBasicBlocks().empty())return;
    bool change=true;
    auto entry=func->getEntryBlock();
    for(auto ins:entry->getInstructions()){
        vn_table_.getValueNum(ins);
    }
    auto dom=info_man_->getFuncDom(func);
    do{
        change=false;
        for(auto bb:dom->getDomTree(entry)){
            auto &ins_list=bb->getInstructions();
            
            for(auto _iter=ins_list.begin();_iter!=ins_list.end();){
                Instruction* instr=*_iter;
                auto cur_iter=_iter++;
                change=change|proInstr(instr);
            }
        }

    } while(change);
}
bool ValNumbering::proInstr(Instruction*instr){
    {
        auto op=instr->getInstrType();
        if(op==Instruction::OpID::fcmp||op==Instruction::OpID::cmp||op==Instruction::OpID::phi||op==Instruction::OpID::br||op==Instruction::OpID::call||op==Instruction::OpID::store||op==Instruction::OpID::load||op==Instruction::OpID::ret)
            return false;
    }
    auto num=vn_table_.getValueNum(instr);
    if(vn_table_.getNumVal(num)==instr){
        return false;
    }else{
        instr->replaceAllUseWith(vn_table_.getNumVal(num));
        instr->getParent()->deleteInstr(instr);
        return true;
    }

}
Expr ValueTable::creatExpr(BinaryInst*bin){
    return Expr(Expr::instop2exprop(bin->getInstrType()),bin->getType(),getValueNum(bin->getOperand(0)),getValueNum(bin->getOperand(1)));
}
Expr ValueTable::creatExpr(SiToFpInst*ins){
    return Expr(Expr::instop2exprop(ins->getInstrType()),ins->getType(),getValueNum(ins->getOperand(0)));
}
Expr ValueTable::creatExpr(FpToSiInst*ins){
    return Expr(Expr::instop2exprop(ins->getInstrType()),ins->getType(),getValueNum(ins->getOperand(0)));
}
Expr ValueTable::creatExpr(ZextInst*ins){
    return Expr(Expr::instop2exprop(ins->getInstrType()),ins->getType(),getValueNum(ins->getOperand(0)));
}