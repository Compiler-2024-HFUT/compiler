#ifndef HASMMODULE_HPP
#define HASMMODULE_HPP

#include "midend/Module.hpp"
#include "midend/Function.hpp"

#include "HAsmFunc.hpp"

class HAsmFunc;
class HAsmBlock;


class HAsmModule {
public:
    explicit HAsmModule(Module *m): m_(m) {}
    void set_hasm_header(std::string hasm_header) { hasm_header_ = hasm_header; }
    void add_to_data_section(std::string global_var_def) { hasm_data_section_def_.push_back(global_var_def); } 
    Module *get_module() { return m_; }
    HAsmFunc *create_func(Function *func);

    std::string get_asm_code();
    std::string print();

private:
    Module *m_;
    std::list<HAsmFunc*> funcs_list_;
    std::string hasm_header_;
    std::vector<std::string> hasm_data_section_def_;
};

#endif 