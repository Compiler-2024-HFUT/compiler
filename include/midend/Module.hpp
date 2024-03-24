#ifndef MODULE_HPP
#define MODULE_HPP

#include <list>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <unordered_map>

#include "Type.hpp"
#include "GlobalVariable.hpp"
#include "Value.hpp"
#include "Function.hpp"


class GlobalVariable;
class Function;
class Instruction;

struct pair_hash {
    template <typename T>
    std::size_t operator()(const std::pair<T, Module *> val) const {
        auto lhs = std::hash<T>()(val.first);
        auto rhs = std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(val.second));
        return lhs ^ rhs;
    }
};

class Module {
public:
    explicit Module(std::string name);
    ~Module();

    Type *getVoidType();
    Type *getLabelType();
    IntegerType *getInt1Type();
    IntegerType *getInt32Type();
    FloatType *getFloatType();
    PointerType *getInt32PtrType();
    PointerType *getFloatPtrType();
    PointerType *getPointerType(Type *contained);
    ArrayType *getArrayType(Type *contained, unsigned num_elements);
    FunctionType *getFunctionType(Type *retty, std::vector<Type *>&args);

    void addFunction(Function *f);
    std::list<Function*> &getFunctions() { return functions_list_; }

    Function *getMainFunction();

    void addGlobalVariable(GlobalVariable *g);
    std::list<GlobalVariable*> &getGlobalVariables() { return globals_list_; }

    std::string getInstrOpName(Instruction::OpID instr) { return instr_id2string_[instr]; }

    void setPrintName();
    void setFileName(std::string name) { source_file_name_ = name; }
    std::string getFileName() { return source_file_name_; }

    virtual std::string print();

public:
    std::unordered_map<std::pair<int, Module *>, std::unique_ptr<ConstantInt>, pair_hash> cached_int;
    std::unordered_map<std::pair<bool, Module *>, std::unique_ptr<ConstantInt>, pair_hash> cached_bool;
    std::unordered_map<std::pair<float, Module *>, std::unique_ptr<ConstantFP>, pair_hash> cached_float;
    std::unordered_map<Type *, std::unique_ptr<ConstantZero>> cached_zero;

private:
    std::list<GlobalVariable *> globals_list_;                  //& The Global Variables in the module
    std::list<Function *> functions_list_;                      //& The Functions in the module
    std::map<std::string, Value*> value_symbol_table_;          //& Symbol table for values
    std::map<Instruction::OpID, std::string> instr_id2string_;  //& Instruction from opid to string

    std::string module_name_;                                   //& Human readable identifier for the module
    std::string source_file_name_;                              //& Original source file name for module, for test and debug

private:
    Type *label_ty_;
    Type *void_ty_;
    IntegerType *int1_ty_;
    IntegerType *int32_ty_;
    FloatType *float32_ty_;

    std::map<Type *, PointerType *> pointer_map_;
    std::map<std::pair<Type*, int>, ArrayType*> array_map_;
    std::map<std::pair<Type*, std::vector<Type*>>, FunctionType*> function_map_;
};

#endif