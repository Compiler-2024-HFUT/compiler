#include "analysis/Dominators.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Value.hpp"
#include <cassert>
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

        {Instruction::OpID::land,ExprOp::AND},
        {Instruction::OpID::lor,ExprOp::OR},
        {Instruction::OpID::lxor,ExprOp::XOR},

        {Instruction::OpID::asr,ExprOp::ASR},
        {Instruction::OpID::shl,ExprOp::SHL},
        {Instruction::OpID::lsr,ExprOp::LSR},
        {Instruction::OpID::asr64,ExprOp::ASR64},
        {Instruction::OpID::shl64,ExprOp::SHL64},
        {Instruction::OpID::lsr64,ExprOp::LSR64},
        {Instruction::OpID::getelementptr,ExprOp::GEP},
        {Instruction::OpID::loadimm,ExprOp::LOADIMM},

    };
    auto iter=i_e.find(instrop);
    if(iter==i_e.end())
        assert(0);
    return iter->second;
}
uint32_t ValueTable::getValueNum(Value*v){
    {
        auto iter=value_hash.find(v);
        if(iter!=value_hash.end())
            return iter->second;
    }
    Expr e{};
    if(auto ins=dynamic_cast<Instruction*>(v)){
        if(auto bin=dynamic_cast<BinaryInst*>(ins)){
            e=creatExpr(bin);
        }else if(auto f2i=dynamic_cast<FpToSiInst*>(ins)){
            e=creatExpr(f2i);
        }else if(auto i2f=dynamic_cast<SiToFpInst*>(ins)){
            e=creatExpr(i2f);
        }else if(auto zext=dynamic_cast<ZextInst*>(ins)){
            e=creatExpr(zext);
        }else if(auto gep=dynamic_cast<GetElementPtrInst*>(ins)){
            e=creatExpr(gep);
        }else if(auto loadimm=dynamic_cast<LoadImmInst*>(ins)){
            e=creatExpr(loadimm);
        }
        /*else if(auto cmp=dynamic_cast<CmpInst*>(ins)){
            e=creatExpr(cmp);
        }else if(auto fcmp=dynamic_cast<FCmpInst*>(ins)){
            e=creatExpr(fcmp);
        }*/
        //alloc call phi br ret store load cmpbr fcmpbr loadoffset storeoffset select
    }
    if(e.op_!=Expr::ExprOp::EMPTY){
        if(auto iter=expressing_hash.find(e);iter!=expressing_hash.end()){
            uint32_t ret=iter->second;
            if(!value_hash.count(v)){
                value_hash.insert({v,ret});
                number_value[ret].push_back(v);
            }
            return ret;
        }else{
            value_hash.insert({v,next_num});
            expressing_hash.insert({e,next_num});
            if(e.op_==Expr::ADD||e.op_==Expr::MUL/*||e.op_==Expr::OR||e.op_==Expr::XOR||e.op_==Expr::AND*/){
                auto switch_e=e;
                switch_e.lhs=e.rhs;
                switch_e.rhs=e.lhs;
                switch_e.op_=e.op_;
                switch_e.type_=e.type_;
                expressing_hash.insert({switch_e,next_num});
            }
            number_value.push_back({v});
            ++next_num;
            return next_num-1;
        }
    }
    //constant,gobal等类型
    value_hash.insert({v,next_num});
    number_value.push_back({v});
    ++next_num;
    return next_num-1;
}
Modify ValNumbering::runOnFunc(Function*func){
    clear();
    if(func->getBasicBlocks().empty())return {};
    // auto runvn=[this](Function*func)->Modify{
        std::vector<PhiInst*> phi_set_;
        std::vector<std::pair<CallInst*,std::vector<int>>> call_set_;
        Modify ret{};
        std::vector<std::pair<PhiInst*,std::vector<Value*>>> phi_ins;
        auto entry=func->getEntryBlock();
        std::list<BasicBlock*> work_list{entry};
        while(!work_list.empty()){
            auto b=work_list.front();
            work_list.pop_front();
            for(auto ins:b->getInstructions()){
                if(ins->isBr()||ins->isRet()||ins->isStore()||ins->isLoad())
                    continue;
                else if(ins->isCall()){
                    auto callee=(Function*)(ins->getOperand(0));
                    if(!fa_->isPureFunc(callee))
                        continue;
                    std::vector<int>num;
                    for(auto o:ins->getOperands()){
                        num.push_back(this->vn_table_.getValueNum(o));
                    }
                    call_set_.push_back({(CallInst*)ins,std::move(num)});
                }else if(ins->isPhi())
                    phi_set_.push_back((PhiInst*)ins);
                else
                    vn_table_.getValueNum(ins);
            }
            auto &b_tree=dom->getDomTree(b);
            // std::copy(b_tree.begin(),b_tree.end(),work_list.end());
            for(auto bb:b_tree){
                work_list.push_back(bb);
            }
        }
        auto size=vn_table_.next_num;
        for(uint32_t id=1;id<size;++id){
            auto& vals=vn_table_.number_value[id];
            //这个值只有一个
            if(vals.size()==1)
                continue;
            BasicBlock* lca = nullptr;
            if(dynamic_cast<Constant*>(vals.front()))continue;
            for (auto _inst : vals) {
                auto inst=(Instruction*)_inst;
                if (lca == nullptr)
                    lca = inst->getParent();
                else {
                    lca = dom->findLCA(lca, inst->getParent());
                }
            }
            Instruction* replace_instr = nullptr;
            for (auto _instr : vals) {
                auto instr=(Instruction*)_instr;
                if (lca == instr->getParent()) {
                    replace_instr = instr;
                    break;
                }
            }
            if(replace_instr==nullptr){
                bool valid_ins=true;
                auto base = (Instruction*)(vals.front());
                for (auto operand : base->getOperands())
                    if(auto op_ins=dynamic_cast<Instruction*>(operand);op_ins&&!dom->isLdomR(op_ins->getParent(),lca)){
                        valid_ins = false;
                        break;
                    }
                if (!valid_ins)
                    continue;

                auto ins=(Instruction*)(vals.front());
                auto &inss=lca->getInstructions();
                auto br=inss.back();
                Instruction* is_cmp=nullptr;
                inss.pop_back();
                if(!inss.empty()){
                    is_cmp=inss.back();
                    if((is_cmp->isCmp()||is_cmp->isFCmp())&&br->getOperand(0)==is_cmp){
                        inss.pop_back();
                    }else{
                        is_cmp=0;
                    }
                }
                replace_instr=ins->copyInst(lca);
                vals.push_back(replace_instr);
                if(is_cmp){
                    inss.push_back(is_cmp);
                }
                inss.push_back(br);
            }
            assert(replace_instr!=nullptr);
            ret.modify_instr=true;
            while(!vals.empty()){
                auto val=vals.back();
                auto inst=(Instruction*)val;
                vals.pop_back();
                if(replace_instr!=inst){
                    inst->replaceAllUseWith(replace_instr);
                    inst->getParent()->deleteInstr(inst);
                    delete inst;
                }
            }
            vals.push_back(replace_instr);
        }
        // for(auto phi_ins:phi_set_){
        //     std::vector<Value*> phi_val;
        //     std::vector<BasicBlock*> phi_bb;
        //     int val_num=0;
        //     for(int i=0;i<phi_ins->getNumOperands();i+=2){
        //         auto v=phi_ins->getOperand(i);
        //         phi_val.push_back(v);
        //         phi_bb.push_back((BasicBlock*)(phi_ins->getOperand(i+1)));
        //         if(val_num==0)
        //             vn_table_.getValueNum(v);
        //         else if(vn_table_.getValueNum(v)!=val_num){
        //             val_num=0;
        //             break;
        //         }
        //     }
        //     //所有编号都相等则可以进行
        //     if(val_num==0)continue;
        //     BasicBlock*lca=phi_bb.back();
        //     phi_bb.pop_back();
        //     int offset=phi_bb.size();
        //     while(!phi_bb.empty()){
        //         lca=dom->findLCA(phi_bb.back(),lca);
        //         if(lca==0)
        //             break;

        //         if(lca==phi_bb.back()){
        //             offset=phi_bb.size()-1;
        //         }
        //         phi_bb.pop_back();
        //     }
        //     if(lca){
        //         LOG_WARNING("gvn_phi");
        //         ret.modify_instr=true;
        //         phi_ins->replaceAllUseWith(phi_val[offset]);
        //     }

        // }
        ret.modify_call|=procCall(call_set_);
        ret.modify_instr|=ret.modify_call;
        return ret;
    // };
    // return runvn(func);
}
bool ValNumbering::procCall(std::vector<std::pair<CallInst*,std::vector<int>>> &call_set_){
    bool ret=false;
    for(int i=0;i<call_set_.size();i++){
        auto  &[call_Ins ,call_arg] =call_set_[i];
        std::vector<CallInst*>erase;
        for(int j=i+1;j< call_set_.size();j++){
            auto & [other ,other_call_arg] =call_set_[j];
            if(call_arg.size()!=other_call_arg.size())
                continue;
            bool eq=true;
            for(int op_offset=0;op_offset<call_arg.size();op_offset++){
                if(call_arg[op_offset]!=other_call_arg[op_offset]){
                    eq=false;
                    break;
                }
            }
            if(eq){
                auto lca=dom->findLCA(call_Ins->getParent(),other->getParent());
                if(lca==0)
                    continue;
                if(lca==call_Ins->getParent()){
                    other->replaceAllUseWith(call_Ins);
                    erase.push_back(other);
                }else{
                    continue;
                    // bool valid_ins=true;
                    // for (auto operand : call_Ins->getOperands())
                    //     if(auto op_ins=dynamic_cast<Instruction*>(operand);op_ins&&!dom->isLdomR(op_ins->getParent(),lca)){
                    //         valid_ins = false;
                    //         break;
                    //     }
                    // if (!valid_ins)
                    //     continue;
                    // auto &inss=lca->getInstructions();
                    // auto br=inss.back();
                    // Instruction* is_cmp=nullptr;
                    // inss.pop_back();
                    // if(!inss.empty()){
                    //     is_cmp=inss.back();
                    //     if((is_cmp->isCmp()||is_cmp->isFCmp())&&br->getOperand(0)==is_cmp){
                    //         inss.pop_back();
                    //     }else{
                    //         is_cmp=0;
                    //     }
                    // }
                    // call_Ins->getParent()->getInstructions().remove(call_Ins);
                    // call_Ins->setParent(br->getParent());
                    // inss.push_back(call_Ins);
                    // other->replaceAllUseWith(call_Ins);
                    // erase.push_back(other);
                    // if(is_cmp){
                    //     inss.push_back(is_cmp);
                    // }
                    // inss.push_back(br);
                }
            }
        }
        for(auto era:erase){
            for(auto iter=call_set_.begin();iter!=call_set_.end();){
                auto cur=iter++;
                if(cur->first==era){
                    iter=call_set_.erase(cur);
                    break;
                }
            }
            era->getParent()->deleteInstr(era);
        }
        if(!erase.empty())
            ret=true;
    }
    return ret;
}


// static std::set<BasicBlock*> visited;
// bool ValNumbering::dvnt(Function*func,BasicBlock*bb){
//     if(visited.count(bb))
//         return false;
//     visited.insert(bb);
//     if(bb==func->getEntryBlock()){
//         for(auto arg:func->getArgs()){
//             vn_table_.getValueNum(arg);
//         }
//     }
//     for(auto ins:bb->getInstructions()){
//         // if()
//     }
// }

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
// Expr ValueTable::creatExpr(CmpInst*ins){
//     int32_t tmp=ins->getCmpOp()+10;
//     Expr::ExprOp op=static_cast<Expr::ExprOp>(ins->getCmpOp()+(int32_t)Expr::EXPR_EQ-CmpOp::EQ);
//     return Expr(op,ins->getType(),getValueNum(ins->getOperand(0)),getValueNum(ins->getOperand(1)));
// }
// Expr ValueTable::creatExpr(FCmpInst*ins){
//     Expr::ExprOp op=static_cast<Expr::ExprOp>(ins->getCmpOp()+(int32_t)Expr::EXPR_EQ-CmpOp::EQ);
//     return Expr(op,ins->getType(),getValueNum(ins->getOperand(0)),getValueNum(ins->getOperand(1)));
// }
Expr ValueTable::creatExpr(GetElementPtrInst*ins){
    if(ins->getNumOperands()==3){
        auto cons=dynamic_cast<ConstantInt*>(ins->getOperand(1));
        assert(cons);
        assert(cons->getValue()==0);
        auto ret= Expr(Expr::ExprOp::GEP,ins->getType(),getValueNum(ins->getOperand(0)),getValueNum(ins->getOperand(1)),getValueNum(ins->getOperand(2)));
        return ret;
    }else{
        return Expr(Expr::ExprOp::GEP,ins->getType(),getValueNum(ins->getOperand(0)),getValueNum(ins->getOperand(1)));
    }
}
Expr ValueTable::creatExpr(LoadImmInst*ins){
    return Expr(Expr::LOADIMM,ins->getType(),getValueNum(ins->getOperand(0)));
}