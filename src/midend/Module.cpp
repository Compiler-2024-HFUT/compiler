#include <cassert>
#include <cstdlib>
#include <list>
#include <memory>
#include <set>
#include <vector>

// #include "analysis/InfoManager.hpp"
#include "midend/Constant.hpp"
#include "midend/Function.hpp"
#include "midend/GlobalVariable.hpp"
#include "midend/IRBuilder.hpp"
#include "midend/Module.hpp"
#include "midend/BasicBlock.hpp"
#include "midend/Instruction.hpp"
Module::Module(std::string name) : module_name_(name) ,builder_(std::make_unique<IRBuilder>()),
//init instr_id2string 
instr_id2string_{
    {Instruction::OpID::ret, "ret"},
    {Instruction::OpID::br, "br"},
    {Instruction::OpID::add, "add"},
    {Instruction::OpID::sub, "sub"},
    {Instruction::OpID::mul, "mul"},
    {Instruction::OpID::mul64, "mul64"},
    {Instruction::OpID::sdiv, "sdiv"},
    {Instruction::OpID::srem, "srem"},
    {Instruction::OpID::fadd, "fadd"},
    {Instruction::OpID::fsub, "fsub"},
    {Instruction::OpID::fmul, "fmul"},
    {Instruction::OpID::fdiv, "fdiv"},
    {Instruction::OpID::alloca, "alloca"},
    {Instruction::OpID::load, "load"},
    {Instruction::OpID::store, "store"},
    {Instruction::OpID::memset, "memset"},
    {Instruction::OpID::cmp, "icmp"},
    {Instruction::OpID::fcmp, "fcmp"},
    {Instruction::OpID::phi, "phi"},
    {Instruction::OpID::call, "call"},
    {Instruction::OpID::getelementptr, "getelementptr"},
    {Instruction::OpID::land, "and"},
    {Instruction::OpID::lor, "or"},
    {Instruction::OpID::lxor, "xor"},
    {Instruction::OpID::asr, "ashr"},
    {Instruction::OpID::shl, "shl"},
    {Instruction::OpID::lsr, "lshr"},
    {Instruction::OpID::asr64, "asr64"},
    {Instruction::OpID::shl64, "shl64"},
    {Instruction::OpID::lsr64, "lsr64"},
    {Instruction::OpID::zext, "zext"},
    {Instruction::OpID::sitofp, "sitofp"},
    {Instruction::OpID::fptosi, "fptosi"},
    {Instruction::OpID::cmpbr, "cmpbr"},
    {Instruction::OpID::fcmpbr, "fcmpbr"},
    {Instruction::OpID::loadoffset, "loadoffset"},
    {Instruction::OpID::storeoffset, "storeoffset"},
    {Instruction::OpID::select, "select"},
    }
{
}

Function* Module::getMainFunction() {
    return *(functions_list_.rbegin());
}
void Module::deleteFunction(Function*f) {
    for(auto b:f->getBasicBlocks()){
        for(auto i:b->getInstructions()){
            i->removeUseOfOps();
        }
    }
    for(auto b:f->getBasicBlocks()){
        for(auto i:b->getInstructions()){
            delete i;
        }
        delete b;
    }

    functions_list_.remove(f);
}

Module::~Module() {
    // breakCheck();
}

void Module::addFunction(Function *f) {
    functions_list_.push_back(f);
}

void Module::addGlobalVariable(GlobalVariable *g) {
    globals_list_.push_back(g);
}

void Module::setPrintName() {
    for (auto &func : this->getFunctions()) {
        func->setInstrName();
    }
}

std::string Module::print() {
    std::string module_ir;
    module_ir+=
R"(define i32 @loadoffset_i(i32* %base_addr,i32 %offset){
  %load = getelementptr i32 , i32* %base_addr , i32 %offset 
  %ret = load i32, i32* %load
  ret i32 %ret
}
define float @loadoffset_f(float* %base_addr,i32 %offset){
  %load = getelementptr float , float* %base_addr , i32 %offset 
  %ret = load float, float* %load
  ret float %ret
}
define void @storeoffset_i(i32 %val ,i32* %base_addr, i32 %offset){
    %store = getelementptr i32 , i32* %base_addr , i32 %offset
    store i32 %val, i32* %store
    ret void
}
define void @storeoffset_f(float %val , float* %base_addr, i32 %offset){
    %store = getelementptr i32 , i32* %base_addr , i32 %offset
    store float %val, i32* %store
    ret void
}
)";
    for (auto &global_val : this->globals_list_) {
        module_ir += global_val->print();
        module_ir += "\n";
    }
    module_ir += "\n";
    for (auto &func : this->functions_list_) {
        module_ir += func->print();
        module_ir += "\n";
    }
    return module_ir;
}
void Module::breakCheck(){
    for(auto f:functions_list_){
        // for(auto b:f->getBasicBlocks()){
        //     for(auto i:b->getInstructions()){
                
        //     }
        // }
        if(f->isDeclaration())
            continue;
        auto entry=f->getEntryBlock();
        assert(entry->getPreBasicBlocks().empty());
    
        std::list<BasicBlock*>work_list{entry};
        std::set<BasicBlock*> reachable;
        std::vector<Instruction*> ins_list;
        do {
            auto curbb=work_list.front();
            work_list.pop_front();
            reachable.insert(curbb);
            for(auto s:curbb->getSuccBasicBlocks()){
                if(reachable.count(s))
                    continue;
                work_list.push_back(s);
            }
            ins_list.insert(ins_list.end(),curbb->getInstructions().begin(),curbb->getInstructions().end());
        }while(!work_list.empty());
        for(auto b:f->getBasicBlocks()){
            assert(reachable.count(b));
        }
        // for(auto ins:ins_list){
        //     for(auto [u,i]:ins->getUseList()){
        //         if(auto use_ins=dynamic_cast<Instruction*>(u)){
        //             assert(use_ins->getOperand(i)==ins);
        //             auto &ops=use_ins->getOperands();
        //             ops[i]=0;
        //         }
        //     }
        //     ins->getUseList().clear();
        // }

        for(auto i:ins_list){
            for(auto o:i->getOperands()){
                if(o==0)
                    continue;
                if(auto op_i=dynamic_cast<Instruction*>(o))
                {
                    
                    // assert(0);
                }else if(auto f=dynamic_cast<Function*>(o)){
                    f->removeUse(i);
                }else if(auto g=dynamic_cast<GlobalVariable*>(o)){
                    g->removeUse(i);
                }else if(auto constant=dynamic_cast<Constant*>(o)){
                    constant->removeUse(i);
                }else if(auto bb=dynamic_cast<BasicBlock*>(o)){
                    bb->removeUse(i);
                }else if(auto arg=dynamic_cast<Argument*>(o)){
                    arg->removeUse(i);
                }else {
                    assert(0);
                }
            }
        }
        for(auto i:ins_list){
            assert(i->useEmpty());
        }
    }
    for(auto g:globals_list_){
        assert(g->useEmpty());
    }
    for(auto f:functions_list_){
        assert(f->useEmpty());
        for(auto arg:f->getArgs()){
            assert(arg->useEmpty());
        }
        for(auto b:f->getBasicBlocks()){
            assert(b->useEmpty());
        }
    }
    auto constman=builder_.get();
}
std::string Module::printGra(){
    std::string ret="digraph {\n";
    for(auto f:getFunctions() ){
        if(f->isDeclaration())continue;
        ret+=f->printGra();
    }
    ret+="}\n";
    return ret;
}
// __attribute__((always_inline)) InfoManager *Module::getInfoMan(){return info_man_.get();}
