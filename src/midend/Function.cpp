#include "midend/Function.hpp"
#include "midend/IRprint.hpp"

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
    basic_blocks_.remove(bb); 
    for (auto pre : bb->getPreBasicBlocks()) {
        pre->removeSuccBasicBlock(bb);
    }
    for (auto succ : bb->getSuccBasicBlocks()) {
        succ->removePreBasicBlock(bb);
    }
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

std::string Argument::print() {
    std::string arg_ir;
    arg_ir += this->getType()->print();
    arg_ir += " %";
    arg_ir += this->getName();
    return arg_ir;
}