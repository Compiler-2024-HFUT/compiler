#include "optimization/instrResolve.hpp"
#include "midend/BasicBlock.hpp"
std::vector<Instruction*> InstrResolve::resolveAdd(Instruction*instr){
    auto lhs=dynamic_cast<Instruction*>(instr->getOperand(0));
    if(lhs==0)
        return {};
    if(!lhs->isMul()){
        // if(lhs->isAdd()){
        //     //(a+c1)+b==>a+b+c1
        //     if(auto lhs_r=dynamic_cast<ConstantInt*>(lhs->getOperand(1));lhs_r){
        //         auto ins1=BinaryInst::create(Instruction::OpID::add,lhs->getOperand(0),instr->getOperand(1));
        //         auto ins2=BinaryInst::create(Instruction::OpID::add,ins1,lhs_r);
        //         ins1->setParent(instr->getParent());
        //         ins2->setParent(instr->getParent());
        //         return {ins1,ins2};
        //     }
        //     // auto rc=dynamic_cast<ConstantInt*>(instr->getOperand(1));
        //     // auto lhs_lhs=dynamic_cast<Instruction*>(lhs->getOperand(0));
        //     // auto lhs_rhs=dynamic_cast<Instruction*>(lhs->getOperand(1));
        //     // //((a+c1)+b)+c2==>a+b+(c1+c2)==>a+b+c3
        //     // if(lhs_lhs==0||lhs_rhs==0||rc)
        //     //     return{};
        //     // if(lhs_lhs->isAdd()&&dynamic_cast<ConstantInt*>(lhs_lhs->getOperand(1))){
        //     //     if(auto llr=dynamic_cast<ConstantInt*>(lhs_lhs->getOperand(1))){
        //     //         auto ret1=BinaryInst::create(Instruction::OpID::add,lhs_lhs->getOperand(0),lhs->getOperand(1));
        //     //         auto ret2=BinaryInst::create(Instruction::OpID::add,ret1,ConstantInt::get(llr->getValue()+rc->getValue()));
        //     //         ret1->setParent(instr->getParent());
        //     //         ret2->setParent(instr->getParent());
        //     //         return std::vector<Instruction*>{ret1,ret2};
        //     //     }
        //     // }else if(lhs_rhs->isAdd()&&dynamic_cast<ConstantInt*>(lhs_rhs->getOperand(1))){
        //     //     if(auto lrr=dynamic_cast<ConstantInt*>(lhs_rhs->getOperand(1))){
        //     //         auto ret1=BinaryInst::create(Instruction::OpID::add,lhs->getOperand(0),lhs_rhs->getOperand(0));
        //     //         auto ret2=BinaryInst::create(Instruction::OpID::add,ret1,ConstantInt::get(lrr->getValue()+rc->getValue()));
        //     //         ret1->setParent(instr->getParent());
        //     //         ret2->setParent(instr->getParent());
        //     //         return std::vector<Instruction*>{ret1,ret2};
        //     //     }
        //     // }
        //     //(a+(b+c1))+c2
        //     //a+(b+c1)==>a+b+c1
        // }
        // if(auto rhs=dynamic_cast<Instruction*>(instr->getOperand(1));rhs&&rhs->isAdd()){
        //     if(auto rhs_r=dynamic_cast<ConstantInt*>(rhs->getOperand(1));rhs_r){
        //         auto ins1=BinaryInst::create(Instruction::OpID::add,lhs,rhs->getOperand(0));
        //         auto ins2=BinaryInst::create(Instruction::OpID::add,ins1,rhs_r);
        //         ins1->setParent(instr->getParent());
        //         ins2->setParent(instr->getParent());
        //         return {ins1,ins2};
        //     }
        // }
    return {};
    }
    auto rhs=dynamic_cast<Instruction*>(instr->getOperand(1));
    if(rhs==0)
        return{};
    auto lhs_lhs=dynamic_cast<Instruction*>(lhs->getOperand(0));
    auto lhs_rhs=dynamic_cast<ConstantInt*>(lhs->getOperand(1));
    if(lhs_lhs==0||lhs_rhs==0)
        return {};
    if(!lhs_lhs->isAdd())
        return{};
    if(auto lhs_lhs_lhs=dynamic_cast<Instruction*>(lhs_lhs->getOperand(0))){
        auto lhs_lhs_rhs=dynamic_cast<ConstantInt*>(lhs_lhs->getOperand(1));
        if(lhs_lhs_rhs){
            auto ins1=BinaryInst::create(Instruction::OpID::mul,lhs_lhs_lhs,lhs_rhs);
            auto ins2=BinaryInst::create(Instruction::OpID::add,ins1,ConstantInt::get(lhs_rhs->getValue()*lhs_lhs_rhs->getValue()));
            auto ins3=BinaryInst::create(Instruction::OpID::add,ins2,rhs);
            ins1->setParent(instr->getParent());
            ins2->setParent(instr->getParent());
            ins3->setParent(instr->getParent());
            return std::vector<Instruction*>{ins1,ins2,ins3};
        }
    }
    return{};
}
std::vector<Instruction*> InstrResolve::resolveRAdd(Instruction*instr){
    auto lhs=dynamic_cast<Instruction*>(instr->getOperand(0)),rhs=dynamic_cast<Instruction*>(instr->getOperand(1));
    if(lhs==0||rhs==0)
        return {};
    if(!rhs->isMul())
        return {};
    auto rhs_lhs=dynamic_cast<Instruction*>(rhs->getOperand(0));
    auto rhs_rhs=dynamic_cast<ConstantInt*>(rhs->getOperand(1));
    if(rhs_lhs==0||rhs_rhs==0)
        return {};
    if(!rhs_lhs->isAdd())
        return{};

    if(auto rhs_lhs_lhs=dynamic_cast<Instruction*>(rhs_lhs->getOperand(0))){
        auto rhs_lhs_rhs=dynamic_cast<ConstantInt*>(rhs_lhs->getOperand(1));
        if(rhs_lhs_rhs){
            auto ins1=BinaryInst::create(Instruction::OpID::mul,rhs_lhs_lhs,rhs_rhs);
            auto ins2=BinaryInst::create(Instruction::OpID::add,ins1,ConstantInt::get(rhs_rhs->getValue()*rhs_lhs_rhs->getValue()));
            auto ins3=BinaryInst::create(Instruction::OpID::add,ins2,lhs);
            ins1->setParent(instr->getParent());
            ins2->setParent(instr->getParent());
            ins3->setParent(instr->getParent());
            return std::vector<Instruction*>{ins1,ins2,ins3};
        }
    }    
    return{};
}
// Instruction* resolveMul(Instruction*instr){

// }

Modify InstrResolve::runOnFunc(Function*func){
    Modify ret{};
    for(auto b:func->getBasicBlocks()){
        for(auto iter=b->getInstructions().begin();iter!=b->getInstructions().end();){
            auto ins=*iter;
            auto cur_iter=iter;
            iter++;
            std::vector<Instruction*> new_ins;
            if(ins->isAdd())
                new_ins=resolveAdd(ins);
            else continue;
            // else if(i->isMul()){
            //     ret=resolveMul(i);
            // }
            if(new_ins.empty())
                new_ins=resolveRAdd(ins);
            if(new_ins.empty())
                continue;
            ret.modify_instr=true;
            ins->replaceAllUseWith(new_ins.back());
            b->getInstructions().insert(cur_iter,new_ins.begin(),new_ins.end());
        }
    }
    return ret;
}