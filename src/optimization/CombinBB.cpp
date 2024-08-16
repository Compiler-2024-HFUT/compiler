#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/Instruction.hpp"
#include "optimization/CombinBB.hpp"
#include "optimization/util.hpp"
#include <cassert>
#include <list>
// void CombinBB::runOnFunc(Function*func){
//     auto &bb_list=func->getBasicBlocks();
//     if(bb_list.size()<2)return;
//     bool change=true;
//     // while(change){
//         change=false;
//         for(auto iter=bb_list.begin();iter!=bb_list.end();){
//             auto b=*(iter++);
//             if(b->getPreBasicBlocks().size()>1||b->getSuccBasicBlocks().size()!=1) {continue;}
//             auto succ_bb=*(b->getSuccBasicBlocks().begin());
//             if(succ_bb->getPreBasicBlocks().size()!=1) {continue;}
//             auto pre_bb=b;
//             // ::std::cout<<pre_bb->getName()<<succ_bb->getName()<<std::endl;
//             change=true;
//             {
//                 auto &instrs=pre_bb->getInstructions();
//                 //erase terminator of prebb
//                 auto termin=*instrs.rbegin();
//                 instrs.pop_back();
//                 delete termin;
            
//                 for(auto instr:succ_bb->getInstructions()){
//                     instr->setParent(pre_bb);
//                     instrs.push_back(instr);
//                 }
//             }
//             auto &pre_succs=pre_bb->getSuccBasicBlocks();
//             pre_succs.clear();
//             pre_succs=succ_bb->getSuccBasicBlocks();
//             for(auto bb:pre_succs){
//                 // bb->replacPreBasicBlock(succ_bb,pre_bb);
//                 // bb->addPreBasicBlock(pre_bb);
//             }
//             // bb_list.remove(succ_bb);
//             if(succ_bb==*iter){
//                 iter=--(bb_list.erase(iter));
//             }
//         }
//     // }
// }
CombinBB::CombinBB(Module *m, InfoManager *im) : FunctionPass(m, im){}
Modify CombinBB::runOnFunc(Function*func){
    Modify ret{};
    auto &bb_list=func->getBasicBlocks();
    if(bb_list.size()<2)return ret;
    bool change=true;
    while(change){
        change=false;
        for(auto iter=bb_list.begin();iter!=bb_list.end();){
            auto to_erase=*iter;
            if(to_erase->getPreBasicBlocks().size()!=1){++iter;continue;}
            BasicBlock* pre_bb=to_erase->getPreBasicBlocks().front();
            if(pre_bb->getSuccBasicBlocks().size()!=1) {++iter;continue;}

            change=true;
            ret.modify_instr=true;
            ret.modify_bb=true;
            {
                deleteIns(pre_bb,pre_bb->getTerminator());
                auto &erase_instrs=to_erase->getInstructions();
                auto &add_instrs=pre_bb->getInstructions();
                while(!erase_instrs.empty()){
                    Instruction* ins=erase_instrs.front();
                    erase_instrs.pop_front();
                    ins->setParent(pre_bb);
                    add_instrs.push_back(ins);
                }
            }
            std::list<BasicBlock*>succs=to_erase->getSuccBasicBlocks();
            pre_bb->getSuccBasicBlocks().swap(to_erase->getSuccBasicBlocks());
            to_erase->getPreBasicBlocks().clear();
            to_erase->getSuccBasicBlocks().clear();

            auto use=to_erase->getUseList();
            for(auto _iter=use.begin();_iter!=use.end();){
                auto u=*_iter;++_iter;
                if(auto phi=dynamic_cast<PhiInst*>(u.val_)){
                    phi->replaceOperand(u.arg_no_,pre_bb);
                    continue;
                }
                assert(0&&"错误的引用");
            }
            ++iter;

            func->getBasicBlocks().remove(to_erase);
            // delete to_erase;
            for(auto succ:pre_bb->getSuccBasicBlocks()){
                for(auto &pre:succ->getPreBasicBlocks()){
                    if(pre==to_erase)pre=pre_bb;
                }
            }
        }
    }
    // for (BasicBlock* block : func->getBasicBlocks()) {
    //     auto &inslist=block->getInstructions();
    //     if(block->getPreBasicBlocks().size()!=1||block->getSuccBasicBlocks().size()!=1)
    //         continue;
    //     auto prebb=block->getPreBasicBlocks().front();
    //     auto succbb=block->getSuccBasicBlocks().front();
    //     if(succbb->getSuccBasicBlocks().empty())
    //         continue;
    //     if (inslist.size() == 1 && inslist.front()->isBr()&&inslist.front()->getNumOperands()==1) {
    //         BranchInst* br = (BranchInst*)(inslist.front());
    //         BasicBlock* nextBlock = (BasicBlock*)(br->getOperand(0));
    //         auto pre_termin=prebb->getTerminator();
    //         if((!pre_termin->isBr())||pre_termin->getNumOperands()!=3)
    //             continue;
    //         if(std::find(succbb->getSuccBasicBlocks().begin(),succbb->getSuccBasicBlocks().end(),block)!=succbb->getSuccBasicBlocks().end())
    //             continue;
    //         if(std::find(succbb->getSuccBasicBlocks().begin(),succbb->getSuccBasicBlocks().end(),prebb)!=succbb->getSuccBasicBlocks().end())
    //             continue;
    //         BasicBlock*other;
    //         if(pre_termin->getOperand(1)==block){
    //             other=(BasicBlock*)pre_termin->getOperand(2);
    //         }else{
    //             other=(BasicBlock*)pre_termin->getOperand(1);
    //         }
    //         if(std::find(succbb->getSuccBasicBlocks().begin(),succbb->getSuccBasicBlocks().end(),other)!=succbb->getSuccBasicBlocks().end())
    //             continue;
    //         if(pre_termin->getOperand(1)==block){
    //             if(pre_termin->getOperand(2)==succbb)
    //                 continue;
    //             pre_termin->replaceOperand(1,succbb);
    //         }else if(pre_termin->getOperand(2)==block){
    //             if(pre_termin->getOperand(1)==succbb)
    //                 continue;
    //             pre_termin->replaceOperand(2,succbb);
    //         }
    //         block->replaceAllUseWith(prebb);
    //         prebb->removeSuccBasicBlock(block);
    //         prebb->addSuccBasicBlock(succbb);
    //         succbb->removePreBasicBlock(block);
    //         succbb->addPreBasicBlock(prebb);
    //         block->getPreBasicBlocks().clear();
    //         block->getSuccBasicBlocks().clear();
    //         ret.modify_bb=true;
    //         block->getInstructions().front()->removeUseOfOps();
    //         block->getInstructions().clear();
    //         func->getBasicBlocks().remove(block);
    //         break;
    //     }
    // }
    return ret;
}