#ifndef MODULE_HPP
#define MODULE_HPP

#include <list>
#include <map>
#include <string>
#include <memory>

#include "Type.hpp"
#include "GlobalVariable.hpp"
#include "Value.hpp"
#include "Instruction.hpp"
#include "analysis/InfoManager.hpp"
class GlobalVariable;
class Function;
class Instruction;
class Module {
public:
    explicit Module(std::string name);
    ~Module();


    void addFunction(Function *f);
    std::list<Function*> &getFunctions() { return functions_list_; }

    Function *getMainFunction();

    void deleteFunction(Function*f);

    void addGlobalVariable(GlobalVariable *g);
    std::list<GlobalVariable*> &getGlobalVariables() { return globals_list_; }

    std::string getInstrOpName(Instruction::OpID instr) const { return instr_id2string_.find(instr)->second; }

    void setPrintName();
    void setFileName(std::string name) { source_file_name_ = name; }
    std::string getFileName() { return source_file_name_; }

    virtual std::string print();

    InfoManager *getInfoMan();

private:
    std::list<GlobalVariable *> globals_list_;                  //& The Global Variables in the module
    std::list<Function *> functions_list_;                      //& The Functions in the module
    std::map<std::string, Value*> value_symbol_table_;          //& Symbol table for values
    std::map<Instruction::OpID, std::string> const instr_id2string_;  //& Instruction from opid to string

    std::string module_name_;                                   //& Human readable identifier for the module
    std::string source_file_name_;                              //& Original source file name for module, for test and debug

private:
    std::unique_ptr<IRBuilder> builder_;
    std::unique_ptr<InfoManager> info_man_;
};

#endif