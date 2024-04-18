#include <memory>

#include "midend/Module.hpp"

Module::Module(std::string name) : module_name_(name) {
    void_ty_ = new Type(Type::VoidTyID, this);
    label_ty_ = new Type(Type::LabelTyID, this);
    int1_ty_ = new IntegerType(1, this);
    int32_ty_ = new IntegerType(32, this);
    float32_ty_ = new FloatType(this);

    //& init instr_id2string 
    instr_id2string_.insert({Instruction::OpID::ret, "ret"});
    instr_id2string_.insert({Instruction::OpID::br, "br"});

    instr_id2string_.insert({Instruction::OpID::add, "add"});
    instr_id2string_.insert({Instruction::OpID::sub, "sub"});
    instr_id2string_.insert({Instruction::OpID::mul, "mul"});
    instr_id2string_.insert({Instruction::OpID::mul64, "mul64"});
    instr_id2string_.insert({Instruction::OpID::sdiv, "sdiv"});
    instr_id2string_.insert({Instruction::OpID::srem, "srem" });

    instr_id2string_.insert({Instruction::OpID::fadd, "fadd"});
    instr_id2string_.insert({Instruction::OpID::fsub, "fsub"});
    instr_id2string_.insert({Instruction::OpID::fmul, "fmul"});
    instr_id2string_.insert({Instruction::OpID::fdiv, "fdiv"});

    instr_id2string_.insert({Instruction::OpID::alloca, "alloca"});
    instr_id2string_.insert({Instruction::OpID::load, "load"});
    instr_id2string_.insert({Instruction::OpID::store, "store"});
    instr_id2string_.insert({Instruction::OpID::memset, "memset"});

    instr_id2string_.insert({Instruction::OpID::cmp, "icmp"});
    instr_id2string_.insert({Instruction::OpID::fcmp, "fcmp"});
    instr_id2string_.insert({Instruction::OpID::phi, "phi"});
    instr_id2string_.insert({Instruction::OpID::call, "call"});
    instr_id2string_.insert({Instruction::OpID::getelementptr, "getelementptr"});

    instr_id2string_.insert({Instruction::OpID::land, "and"});
    instr_id2string_.insert({Instruction::OpID::lor, "or"});
    instr_id2string_.insert({Instruction::OpID::lxor, "xor"});

    instr_id2string_.insert({Instruction::OpID::asr, "ashr"});
    instr_id2string_.insert({Instruction::OpID::shl, "shl"});
    instr_id2string_.insert({Instruction::OpID::lsr, "lshr"});
    instr_id2string_.insert({Instruction::OpID::asr64, "asr64"});
    instr_id2string_.insert({Instruction::OpID::shl64, "shl64"});
    instr_id2string_.insert({Instruction::OpID::lsr64, "lsr64"});

    instr_id2string_.insert({Instruction::OpID::zext, "zext"});
    instr_id2string_.insert({Instruction::OpID::sitofp, "sitofp"});
    instr_id2string_.insert({Instruction::OpID::fptosi, "fptosi"});

    instr_id2string_.insert({Instruction::OpID::cmpbr, "cmpbr"});
    instr_id2string_.insert({Instruction::OpID::fcmpbr, "fcmpbr"});
    instr_id2string_.insert({Instruction::OpID::loadoffset, "loadoffset"});
    instr_id2string_.insert({Instruction::OpID::storeoffset, "storeoffset"});
}

Function* Module::getMainFunction() {
    return *(functions_list_.rbegin());
}
 

Module::~Module() {
    delete void_ty_;
    delete label_ty_;
    delete int1_ty_;
    delete int32_ty_;
    delete float32_ty_;
}


Type *Module::getVoidType() {
    return void_ty_;
}

Type *Module::getLabelType() {
    return label_ty_;
}

IntegerType *Module::getInt1Type() {
    return int1_ty_;
}

IntegerType *Module::getInt32Type() {
    return int32_ty_;
}

FloatType *Module::getFloatType() {
    return float32_ty_;
}

PointerType *Module::getInt32PtrType() {
    return getPointerType(int32_ty_);
}

PointerType *Module::getFloatPtrType() {
    return getPointerType(float32_ty_);
}

PointerType *Module::getPointerType(Type *contained) {
    if(pointer_map_.find(contained) == pointer_map_.end()) {
        pointer_map_[contained] = new PointerType(contained);
    }
    return pointer_map_[contained];
}

ArrayType *Module::getArrayType(Type *contained, unsigned num_elements) {
    if(array_map_.find({contained, num_elements}) == array_map_.end()) {
        array_map_[{contained, num_elements}] = new ArrayType(contained, num_elements);
    }
    return array_map_[{contained, num_elements}];
}

FunctionType *Module::getFunctionType(Type *retty, std::vector<Type *> &args) {
    if (not function_map_.count({retty, args})) {
        function_map_[{retty, args}] = new FunctionType(retty, args);
    }
    return function_map_[{retty, args}];
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