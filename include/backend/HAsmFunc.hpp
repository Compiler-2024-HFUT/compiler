#ifndef HASMFUNC_HPP
#define HASMFUNC_HPP

#include <string>

#include "midend/Function.hpp"

#include "HAsmModule.hpp"
#include "HAsmBlock.hpp"
#include "HAsmLoc.hpp"

class HAsmModule;
class HAsmBlock;

class HAsmFunc {
public:
    explicit HAsmFunc(HAsmModule *parent, Function *f): parent_(parent), f_(f) {}
    void set_func_header(std::string header) { func_header_ = header; }

    Function *get_function() { return f_; }

    HAsmBlock *create_bb(BasicBlock *bb, Label *label);

    std::string get_asm_code();
    std::string print();

private:
    std::string func_header_;
    std::list<HAsmBlock*> blocks_list_;
    HAsmModule *parent_;
    Function *f_;
};

#endif