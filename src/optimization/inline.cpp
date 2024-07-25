#include "optimization/inline.hpp"
#include "analysis/Info.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "midend/Value.hpp"
#include <list>
#include <sys/cdefs.h>
#include <vector>
using ::std::list,::std::map;
::std::set<CallInst *> FuncInline::getCallInfo(Module* m){
::std::set<CallInst *>  call_info;
    for(auto f:module_->getFunctions())
        for(auto b:f->getBasicBlocks())
            for(auto ins:b->getInstructions() ){
                if(!ins->isCall()) continue;
                // auto ins_call=call_info.find(f);
                auto call=static_cast<CallInst*>(ins);
                if(call_info.find(call)!=call_info.end())
                    continue;
                // if(ins_call==call_info.end()){
                call_info.insert(call);
                // }else{
                //     ins_call->second.push_back({call,(Function*)call->getOperand(0)});
                // }
            }
    return call_info;
}
//返回内联后增加的指令,可能用不到
::std::vector<CallInst*> insertFunc(CallInst* call,std::list<Function*> calleds){
    auto call_func=static_cast<Function*>(call->getOperand(0));
    //间接递归
    for(auto called:calleds)
        if(called==call_func)
            return {};
    auto cur_bb=call->getParent();
    auto cur_func=cur_bb->getParent();
    //直接递归
    if(cur_func==call_func)
        return {};
    // ::std::list<BasicBlock*>succ_bbs;
    Instruction *new_instr;
    BasicBlock *new_bb;
    BasicBlock *ret_bb;
    std::map<Value *, Value *> old_new;
    std::vector<BasicBlock *> new_bbs;
    std::vector<CallInst*> _newcall;

    auto iter_inster=++(cur_bb->findInstruction(call));
    ::std::vector<decltype(iter_inster)> _list;
    while(iter_inster!=cur_bb->getInstructions().end()){
        auto cur_iter=iter_inster++;
        _list.push_back(cur_iter);
    }
    list<BasicBlock*>_succ_bbs=cur_bb->getSuccBasicBlocks();

    {
        int arg_idx = 1;
        for (auto arg: call_func->getArgs()){
            old_new.insert({arg,call->getOperand(arg_idx++)});
        }
    }

    // auto func_list{};
    for(auto old_bb: call_func->getBasicBlocks()){
        new_bb = BasicBlock::create( "", cur_func);
        old_new.insert({old_bb,new_bb});
        new_bbs.push_back(new_bb);
        for(auto old_instr: old_bb->getInstructions()){
            new_instr = old_instr->copyInst(new_bb);
            if(old_instr->isPhi()){
                new_bb->addInstruction(new_instr);
            }
            old_new.insert({old_instr,  new_instr});
        }
    }
    for(auto new_bb:new_bbs){
        for(auto instr:new_bb->getInstructions()){
            if(instr->isBr()&&instr->getNumOperands()==1){
                auto true_bb = static_cast<BasicBlock*>(old_new[instr->getOperand(0)]);
                instr->replaceOperand(0,true_bb);
                true_bb->addPreBasicBlock(new_bb);
                new_bb->addSuccBasicBlock(true_bb);
            }else if(instr->isBr()/*||instr->isCmpBr()||instr->isFCmpBr()*/){
                if(old_new[instr->getOperand(0)]!=nullptr)
                    instr->replaceOperand(0, old_new[instr->getOperand(0)]);
                auto false_bb = static_cast<BasicBlock*>(old_new[instr->getOperand(2)]);
                instr->replaceOperand(2,false_bb);
                false_bb->addPreBasicBlock(new_bb);
                new_bb->addSuccBasicBlock(false_bb);
                auto true_bb = static_cast<BasicBlock*>(old_new[instr->getOperand(1)]);
                instr->replaceOperand(1,true_bb);
                true_bb->addPreBasicBlock(new_bb);
                new_bb->addSuccBasicBlock(true_bb);
            }else if(instr->isRet()){
                call->removeOperands(0,0);
                if (instr->getNumOperands()==1){
                    if(old_new[instr->getOperand(0)]!=nullptr)
                        call->replaceAllUseWith(old_new[instr->getOperand(0)]);
                    else
                        call->replaceAllUseWith(instr->getOperand(0));
                }
                ret_bb=instr->getParent();
                new_bb->deleteInstr(instr);

                delete instr;
                break;
            }else{
                for (int i = 0; i < instr->getNumOperands(); i++){
                    if(old_new[instr->getOperand(i)]!=nullptr)
                        instr->replaceOperand(i,old_new[instr->getOperand(i)]);
                }
                if(instr->isCall())
                    _newcall.push_back(static_cast<CallInst*>(instr));
            }
        }
    }
    for(auto iter:_list){
        cur_bb->getInstructions().erase(iter);
        auto instr=*iter;
        instr->setParent(ret_bb);
        ret_bb->addInstruction(instr);
    }
    cur_bb->getSuccBasicBlocks().clear();
    BranchInst::createBr(new_bbs.front(),cur_bb);

    for(auto succ_bb:_succ_bbs){
        succ_bb->removePreBasicBlock(cur_bb);
        succ_bb->addPreBasicBlock(ret_bb);
        ret_bb->addSuccBasicBlock(succ_bb);

        for(auto instr:succ_bb->getInstructions()){
            if(instr->isPhi()){
                auto &ops=instr->getOperands();
                for(int i=0;i<ops.size();i++){
                    if(ops[i]==cur_bb){
                        instr->replaceOperand(i,ret_bb);
                    }
                }
            }else
                break;
        }

    }

    // calleds.push_back(call_func);
    // for(auto incall:_incall){
    //     if(isEmpty((Function*)incall->getOperand(0)))continue;
    //     insertFunc(incall,calleds);
    // }
    call->getParent()->deleteInstr(call);
    delete call;
    return _newcall;
}
Modify FuncInline::run(){
    // auto fan=info_man_->getInfo<FuncAnalyse>();
    func_call_=getCallInfo(module_);
    for(auto call:func_call_){
        if(isEmpty((Function*)call->getOperand(0)))continue;
        insertFunc(call,{call->getParent()->getParent()});
    }
    Modify ret{};
    ret.modify_instr=true;
    ret.modify_bb=true;
    ret.modify_call=true;
    auto main_func=module_->getMainFunction();
    auto f_list=module_->getFunctions();
    for(auto f:f_list){
        if(f->useEmpty()&&f!=main_func){
            module_->deleteFunction(f);
            delete f;
        }
    }
    return ret;
    // auto &fs=module_->getFunctions();
    // for(auto iter_f=fs.begin();iter_f!=fs.end();){
    //     auto cf=iter_f++;
    //     auto f=*cf;
    //     if(f->useEmpty()&&f!=module_->getMainFunction()){
    //         module_->deleteFunction(f);
    //     }
    // }

}

__attribute__((always_inline)) bool isEmpty(Function*f){
    return f->getBasicBlocks().empty();
}
