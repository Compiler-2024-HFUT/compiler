#include "optimization/GenLoadImm.hpp"
#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Constant.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include <list>
#include <utility>
#include <vector>
Value* __cacheGetIntImm(std::vector<std::pair<int,Instruction*>>&cache, int new_imm){
    for(auto imm:cache){
        if(imm.first==new_imm){
            //是否合理
            cache.push_back(imm);
            return imm.second;
        }
    }
    return 0;
}
Instruction* __genNewIntImm(std::vector<std::pair<int,Instruction*>>&cache, int new_imm){
    Instruction* ret=0;
    //addi zero可以赋值的
    if(new_imm<2048&&new_imm>-2049){
        ret=LoadImmInst::createLoadImm(Type::getInt32Type(),ConstantInt::get(new_imm));
        cache.push_back({new_imm,ret});
        return ret;
    }
    //优先查看以前的
    for(int i=cache.size()-1;i>=0;--i){
        auto [old_imm,ins]=cache[i];
        int gap=new_imm-old_imm;
        if(gap<2048&&gap>-2049){
            ret=BinaryInst::create(Instruction::OpID::add,ins,ConstantInt::get(gap));
            cache.push_back({new_imm,ret});
            break;
        }
    }
    if(ret!=0)
        return ret;

    for(int i=cache.size()-1;i>=0;--i){
        auto [old_imm,ins]=cache[i];
        if(new_imm==-old_imm){
            ret=BinaryInst::create(Instruction::OpID::sub,ConstantInt::get(0),ins);
            cache.push_back({new_imm,ret});
            break;
        }
    }
    if(ret!=0)
        return ret;
    //以前的,移位可以得到
    for(int i=cache.size()-1;i>=0;--i){
        auto [old_imm,ins]=cache[i];
        if(old_imm<<1==new_imm){
            ret=BinaryInst::create(Instruction::OpID::shl,ins,ConstantInt::get(1));
            cache.push_back({new_imm,ret});
            break;
        }
    }
    if(ret!=0)
        return ret;
    for(int i=cache.size()-1;i>=0;--i){
        auto [old_imm,ins]=cache[i];
        if(old_imm>>1==new_imm){
            ret=BinaryInst::create(Instruction::OpID::asr,ins,ConstantInt::get(1));
            cache.push_back({new_imm,ret});
            break;
        }
    }
    if(ret!=0)
        return ret;
    ret=LoadImmInst::createLoadImm(Type::getInt32Type(),ConstantInt::get(new_imm));
    cache.push_back({new_imm,ret});
    return ret;
}
bool GenLoadImm::runOnBB(BasicBlock*b,std::vector<std::pair<int,Instruction*>>caches){
    bool ret=false;

    auto genImm=[this](std::list<Instruction*>::iterator insert_pos,
    Instruction*ins,int const_op_num,int const_val,std::vector<std::pair<int,Instruction*>>&caches)->void
    {
        if(auto imm=__cacheGetIntImm(caches,const_val)){
            ins->replaceOperand(const_op_num,imm);
        }else{
            auto newimmIns=__genNewIntImm(caches,const_val);
            newimmIns->setParent(ins->getParent());
            ins->replaceOperand(const_op_num,newimmIns);
            ins->getParent()->insertInstr(insert_pos,newimmIns);
        }
    };
    auto &ins_list=b->getInstructions();
    for(auto iter=ins_list.begin();iter!=ins_list.end();){
        auto cur_iter=iter;
        auto ins=*cur_iter;
        ++iter;
        if(ins->isDiv()||ins->isRem()||ins->isSub()||ins->isCmp()||ins->isXor()||ins->isOr()||ins->isAnd()){
            int const_op_num=-1;
            int const_val;
            if(dynamic_cast<ConstantInt*>(ins->getOperand(0))){
                const_op_num=0;
            }else if(dynamic_cast<Constant*>(ins->getOperand(1))){
                const_op_num=1;
            }
            if(const_op_num==-1)
                continue;
            const_val=((ConstantInt*)ins->getOperand(const_op_num))->getValue();
            if(const_val==0)
                continue;
            genImm(cur_iter,ins,const_op_num,const_val,caches);
            // if(auto imm=__cacheGetIntImm(caches,const_val)){
            //     ins->replaceOperand(const_num,imm);
            // }else{
            //     auto newimmIns=__genNewIntImm(caches,const_val);
            //     newimmIns->setParent(b);
            //     ins->replaceOperand(const_num,newimmIns);
            //     ins_list.insert(cur_iter,newimmIns);
            // }
        }
        else if(ins->isAdd()){
            if(ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(0))){
                auto op1=ins->getOperand(1);
                ins->removeAllOperand();
                ins->addOperand(op1);
                ins->addOperand(cons);
            }
            if(ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(1))){
                if(cons->getValue()<2048&&cons->getValue()>=-2048){
                    continue;
                }
                genImm(cur_iter, ins, 1,cons->getValue() , caches);
            }
        }else if(ins->isMul()){
            if(ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(0))){
                auto op1=ins->getOperand(1);
                ins->removeAllOperand();
                ins->addOperand(op1);
                ins->addOperand(cons);
            }
            if(ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(1))){
                if(cons->getValue()==0){
                    ins->replaceAllUseWith(ConstantInt::get(0));
                    ins_list.erase(cur_iter);
                    delete ins;
                    continue;
                }
                genImm(cur_iter, ins, 1,cons->getValue() , caches);
            }
        }
        else if(ins->isStore()||ins->isStoreOffset()){
            auto const_val=dynamic_cast<ConstantInt*>(ins->getOperand(0));
            if(const_val==0)
                continue;
            auto val=const_val->getValue();
            if(val==0)
                continue;
            genImm(cur_iter,ins,0,val,caches);
        }else if(ins->isGep()){
            if(ins->getNumOperands()==2){
                ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(1));
                if(cons==0)
                    continue;
                if(cons->getValue()>511||cons->getValue()<-512){
                    genImm(cur_iter, ins, 1,cons->getValue() , caches);
                }
            }else if(ins->getNumOperands()==3){
                ConstantInt* cons=dynamic_cast<ConstantInt*>(ins->getOperand(2));
                if(cons==0)
                    continue;
                if(cons->getValue()>511||cons->getValue()<-512){
                    genImm(cur_iter, ins, 2,cons->getValue() , caches);
                }
            }
        }
        else if(ins->isFDiv()||ins->isFAdd()||ins->isFMul()||ins->isFDiv()||ins->isFCmp()){
            if(dynamic_cast<Constant*>(ins->getOperand(0))){
                auto imm=LoadImmInst::createLoadImm(ins->getOperand(0)->getType(),ins->getOperand(0),b);
                ins_list.pop_back();
                ins_list.insert(cur_iter,imm);
                ret=true;
                ins->replaceOperand(0,imm);
            }else if(dynamic_cast<Constant*>(ins->getOperand(1))){
                auto imm=LoadImmInst::createLoadImm(ins->getOperand(0)->getType(),ins->getOperand(1),b);
                ins_list.pop_back();
                ins_list.insert(cur_iter,imm);
                ret=true;
                ins->replaceOperand(1,imm);
            }
        }else if(ins->isLsl()||ins->isAsr()||ins->isLsr()){
            if(dynamic_cast<ConstantInt*>(ins->getOperand(0))){
                auto imm=LoadImmInst::createLoadImm(ins->getType(),ins->getOperand(0),b);
                ins_list.pop_back();
                ins_list.insert(cur_iter,imm);
                ret=true;
                ins->replaceOperand(0,imm);
            }
        }
    }
    // for(auto succ:b->getSuccBasicBlocks()){
    //     if(succ->getPreBasicBlocks().size()==1){
            
    //     }
    // }
    return ret;
}
Modify GenLoadImm::runOnFunc(Function *function){
    if(function->isDeclaration())
        return{};
    Modify ret;
//     auto islog2=[](int num){
//     if(num<=0){
//         return 0;
//     }
//     int ret=0;
//     int curnum=1;
//     while(curnum<num){
//         curnum=curnum<<1;
//         ++ret;
//     }
//     if(curnum!=num)
//         return 0;
//     return ret;
// };
    for(auto b:function->getBasicBlocks()){
        runOnBB(b,{});
    }
    return ret;
}