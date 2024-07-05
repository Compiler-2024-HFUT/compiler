#include <algorithm>
#include <cassert>
#include <map>
#include "midend/Module.hpp"
#include "midend/Function.hpp"
#include "midend/IRprint.hpp"
#include "midend/BasicBlock.hpp"

void Function::buildArgs() {
    auto *func_ty = getFunctionType();
    unsigned num_args = getNumOfArgs();
    for (int i = 0; i < num_args; i++) {
        auto arg = new Argument(func_ty->getParamType(i), "", this, i);
        arguments_.push_back(arg);
        if(arg->getType()->isFloatType()) {
            f_args_.push_back(arg);
        } else {
            i_args_.push_back(arg);
        }
    }
}

Function::Function(FunctionType *ty, const std::string &name, Module *parent)
    : Value(ty, name), parent_(parent), seq_cnt_(0) {
    parent->addFunction(this);
    buildArgs();
}

Function::~Function() {
    for (auto *arg : arguments_)
        delete arg;
}

Function *Function::create(FunctionType *ty, const std::string &name, Module *parent) {
    return new Function(ty, name, parent);
}

void Function::addBasicBlock(BasicBlock *bb) {
    basic_blocks_.push_back(bb);
}

void Function::removeBasicBlock(BasicBlock *bb) {
    basic_blocks_.erase(std::find(basic_blocks_.begin(),basic_blocks_.end(),bb));
    for (auto pre : bb->getPreBasicBlocks()) {
        pre->removeSuccBasicBlock(bb);
    }
    for (auto succ : bb->getSuccBasicBlocks()) {
        succ->removePreBasicBlock(bb);
    }
}
BasicBlock *Function::getRetBlock() const {
    for(auto b:basic_blocks_)
        if(b->getSuccBasicBlocks().empty())
            return b;
    return nullptr;
}
void Function::setInstrName() {
    std::map<Value *, int> seq;
    for (const auto &arg : this->getArgs()) {
        if (seq.find(&*arg) == seq.end()) {
            auto seq_num = seq.size() + seq_cnt_;
            if (arg->setName("arg" + std::to_string(seq_num))) {
                seq.insert({&*arg, seq_num});
            }
        }
    }
    for (auto &bb1 : basic_blocks_) {
        auto bb = bb1;
        if (seq.find(bb) == seq.end()) {
            auto seq_num = seq.size() + seq_cnt_;
            #ifdef DEBUG
                std::string f_name=this->getName();
            if (bb->setName(f_name+"_label_" + std::to_string(seq_num))) {
                seq.insert({bb, seq_num});
            }
            #else
                if (bb->setName("label" + std::to_string(seq_num))) {
                    seq.insert({bb, seq_num});
                }
            #endif

        }
        for (auto &instr : bb->getInstructions()) {
            if (!instr->isVoid() && seq.find(instr) == seq.end()) {
                auto seq_num = seq.size() + seq_cnt_;
                #ifdef DEBUG
                std::string f_name=this->getName();
                    if (instr->setName(f_name+"_op_" + std::to_string(seq_num))) {
                        seq.insert({instr, seq_num});
                    }
                #else
                if (instr->setName("op" + std::to_string(seq_num))) {
                    seq.insert({instr, seq_num});
                }
                #endif

            }
        }
    }
    seq_cnt_ += seq.size();
}

std::string Function::print() {
    setInstrName();
    std::string func_ir;
    if (this->isDeclaration()) {
        func_ir += "declare ";
    } else {
        func_ir += "define ";
    }

    func_ir += this->getReturnType()->print();
    func_ir += " ";
    func_ir += printAsOp(this, false);
    func_ir += "(";

    //// print arg
    if (this->isDeclaration()) {
        for (int i = 0; i < this->getNumOfArgs(); i++) {
            if (i)
                func_ir += ", ";
            func_ir += static_cast<FunctionType *>(this->getType())->getParamType(i)->print();
        }
    } else {
        for (auto arg = this->argBegin(); arg != argEnd(); arg++) {
            if (arg != this->argBegin()) {
                func_ir += ", ";
            }
            func_ir += (*arg)->print();
        }
    }
    func_ir += ")";

    //// print bb
    if (this->isDeclaration()) {
        func_ir += "\n";
    } else {
        func_ir += " {";
        func_ir += "\n";
        for (auto &bb : this->getBasicBlocks()) {
            func_ir += bb->print();
        }
        func_ir += "}";
    }

    return func_ir;
}
std::string Function::printGra(){
    ::std::string ret;
#ifdef DEBUG
    setInstrName();
    ::std::set<BasicBlock*> bb_set;
    // ::std::vector<::std::pair<BasicBlock*,BasicBlock*>>graph;
    ::std::map<BasicBlock* , ::std::set<BasicBlock*>>graph;
    for(auto bb:getBasicBlocks()){
        bb_set.insert(bb);
    }
    auto entry=getEntryBlock();
    ::std::function<void (BasicBlock* bb,::std::map<BasicBlock* , ::std::set<BasicBlock*>> &graph)> draw=[&draw](BasicBlock* bb,::std::map<BasicBlock* , ::std::set<BasicBlock*>> &graph){
        if(graph.find(bb)!=graph.end())
            return ;

        auto it=graph.insert({bb,{}}).first;
        for(auto succ_bb:bb->getSuccBasicBlocks()){
            if(it->second.find(succ_bb)==it->second.end()){
                it->second.insert(succ_bb);
            }else{
                exit(123);
            }
        }

        for(auto succ_bb:bb->getSuccBasicBlocks()){
            draw(succ_bb,graph);
        }

    };
    draw(entry,graph);
    // ret="digraph ";
    // ret+=getName();ret+="{\n";
    for(auto bb:bb_set){
        ret+=bb->getName();ret+=";\n";
    }
    for(auto [bb,bbset]:graph){
        for(auto succ_bb:bbset){
            ret+=bb->getName()+"->"+succ_bb->getName()+";\n";
        }
    }
    // ret+="}\n";
#endif
    return ret;
}
std::string Argument::print() {
    std::string arg_ir;
    arg_ir += this->getType()->print();
    arg_ir += " %";
    arg_ir += this->getName();
    return arg_ir;
}
