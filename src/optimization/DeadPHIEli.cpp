#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "midend/Value.hpp"
#include <set>
#include "optimization/DeadPHIEli.hpp"


void replacePhiWith(PhiInst*old,Value* new_val){
    auto &ul=old->getUseList();
    for(auto __u_iter=ul.begin();__u_iter!=ul.end();){
        auto use=*(__u_iter++);
        if(auto phi= dynamic_cast<PhiInst*>(use.val_)){
            phi->replaceOperand(use.arg_no_,new_val);
            // if(auto instr=dynamic_cast<Instruction*>(use.val_))
            //     phi->replaceOperand(use.arg_no_+1,instr->getParent());
        }else{
            auto instr=(Instruction*)(use.val_);
            instr->replaceOperand(use.arg_no_,new_val);
        }
    }
}
bool is_phi_cycle(PhiInst*phi,::std::set<PhiInst*>&phi_set){
    if(phi->useEmpty())return true;
    if(phi->getUseNum()!=1)return false;

    if(phi_set.count(phi))
        return true;
    else if(auto use=dynamic_cast<PhiInst*>(phi->getUseList().back().val_)){
        phi_set.insert(phi);
        return is_phi_cycle(use,phi_set);
    }

    return false;
}
void DeadPHIEli::runOnFunc(Function*func){
    auto &bb_list=func->getBasicBlocks();
    if(bb_list.size()<2)return;
    std::list<PhiInst*> phi_set;
    for(auto b:bb_list){
        for(auto ins:b->getInstructions()){
            if(auto phi=dynamic_cast<PhiInst*>(ins))
            phi_set.push_back(phi);
        }
    }
    bool change=true;
    while(change){
        change=false;
        for(auto __iter=phi_set.begin();__iter!=phi_set.end();){
            auto phi=*__iter;
            auto iter=__iter++;
            std::map<Value*,int> phi_incom{};
            // for(int i=0;i<phi->getNumOperands();i+=2){
            //     if(phi_incom.count(phi->getOperand(i))){
            //         // if(phi->getOperand(phi_incom[phi->getOperand(i)]+1))
            //         phi->removeOperands(i,i+1);
            //         change=true;    
            //     }
            //     else phi_incom.insert({phi->getOperand(i),i});
            //         // if(phi->getOperand(i+1)==phi->getOperand(i).)
            // }
            // if(phi->getNumOperands()==2&&phi->getOperand(0)!=phi){
            //     replacePhiWith(phi,phi->getOperand(0));
            //     change=true;
            // }
            if(phi->useEmpty()){
                phi_set.erase(iter);
                phi->getParent()->deleteInstr(phi);
                delete  phi;
                change=true;
            }else if(phi->useOne()&&dynamic_cast<PhiInst*>(phi->getUseList().back().val_)){
                std::set<PhiInst*> visited{};
                if(is_phi_cycle(phi,visited)){
                    auto u=phi->getUseList().back();
                    auto ins=static_cast<PhiInst*>(u.val_);
                    ins->removeOperands(u.arg_no_,u.arg_no_+1);
                    phi_set.erase(iter);
                    visited.erase(phi);
                    phi->getParent()->deleteInstr(phi);
                    delete  phi;
                    change=true;
                }
            }
        }
    }
    // for(auto phi:phi_set){
    //     for(int i=0;i<phi->getNumOperands();i+=2){
    //         if(auto ins=dynamic_cast<Instruction*>(phi)){
    //             phi->replaceOperand(i+1,ins->getParent());
    //         }
    //     }
    // }
    // for(auto phi:phi_set){
    //     ::std::cout<<phi->getNumOperands()<<::std::endl;
    // }

}