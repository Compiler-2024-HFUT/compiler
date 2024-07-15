//该文件里写寄存器分配
#ifndef REGALLOC_HPP
#define REGALLOC_HPP


#include <queue>

#include "midend/Module.hpp"
#include "midend/Instruction.hpp"
#include "midend/BasicBlock.hpp"

struct Range {
    int from;
    int to;
    Range(int from, int to): from(from), to(to) {}
};

class Interval {
public:
    explicit Interval(Value *value): value(value), is_fp(value->getType()->isFloatType()) {}

public:
    void add_range(int from, int to);
    void add_use_pos(int pos);
    bool covers(int id);
    bool covers(Instruction *inst);
    bool intersects(Interval *other);
    void union_interval(Interval *other);
    bool is_float_type() { return is_fp; }

public:
    int reg_id = -1;
    std::list<Range*> range_list;
    std::list<int> use_position_list;
    Value *value;
    bool is_fp;
};

const int ireg_priority[] = {
    0, 0,
    0, 0,
    0, 25,
    24, 23,
    0, 0,
    8, 7,
    6, 5,
    4, 3,
    2, 1,
    18, 17,
    16, 15,
    14, 13,
    12, 11,
    10, 9,
    22, 21,
    20, 19
};

const int freg_priority[] = {
    30, 29,
    28, 27,
    26, 25,
    24, 23,
    0, 0,
    8, 7,
    6, 5,
    4, 3,
    2, 1,
    18, 17,
    16, 15,
    14, 13,
    12, 11,
    10, 9,
    22, 21,
    20, 19
};

const std::vector<int> all_available_ireg_ids = {
    5, 6, 7, 
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31 
};
const std::vector<int> all_available_freg_ids = {
    0, 1, 2, 3, 4, 5, 6, 7,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31
};

class RegAllocDriver {
public:
    explicit RegAllocDriver(Module *m): m(m) {}
public:
    void compute_reg_alloc();
    std::map<Value*, Interval*>& get_ireg_alloc_in_func(Function *func) { return ireg_alloc[func]; }
    std::map<Value*, Interval*>& get_freg_alloc_in_func(Function *func) { return freg_alloc[func]; }
    std::list<BasicBlock*>& get_bb_order_in_func(Function *func) { return bb_order[func]; } 

private:
    std::map<Function*, std::list<BasicBlock*>> bb_order;
    std::map<Function*, std::map<Value*, Interval*>> ireg_alloc;
    std::map<Function*, std::map<Value*, Interval*>> freg_alloc;
    Module *m;
};


struct cmp_interval {
    bool operator()(const Interval *a, const Interval *b) const {
        auto a_from = (*(a->range_list.begin()))->from;
        auto b_from = (*(b->range_list.begin()))->from;
        if(a_from!=b_from){
            return a_from < b_from;
        }else{
            return a->value->getName() < b->value->getName();
        }
    }
};


struct cmp_ireg {
    bool operator()(const int reg1,const int reg2) const{
        return ireg_priority[reg1] > ireg_priority[reg2];
    }
};

struct cmp_freg {
    bool operator()(const int reg1,const int reg2)const{
        return freg_priority[reg1] > freg_priority[reg2];
    }
};

struct cmp_range{
    bool operator()(const Range* a,const Range* b) const {
        return a->from > b->from;
    }
};


struct cmp_block_depth{
    bool operator()(BasicBlock* b1, BasicBlock* b2){
        return b1->getLoopDepth() < b2->getLoopDepth();
    }
};

class RegAlloc {
public:
    explicit RegAlloc(Function* func): func(func) {}

public:
    void execute();
    std::map<Value*, Interval*>& get_ireg_alloc() { return ival2Inter; }
    std::map<Value*, Interval*>& get_freg_alloc() { return fval2Inter; }
    std::list<BasicBlock*>& get_block_order() { return block_order; }

private:
    void init();
    void compute_block_order();
    void number_operations();
    void compute_bonus_and_cost();
    void build_intervals();
    void walk_intervals();
    void add_interval(Interval *interval) { interval_list.insert(interval); }
    void add_reg_to_pool(Interval *interval);
    bool try_alloc_free_reg();

private:
    std::set<int> unused_ireg_id = { all_available_ireg_ids.begin(), all_available_ireg_ids.end() };
    std::set<int, cmp_ireg> remained_all_ireg_ids = { all_available_ireg_ids.begin(), all_available_ireg_ids.end() };
    std::map<int,std::set<Interval*>> ireg2ActInter;

    std::set<int> unused_freg_id = { all_available_freg_ids.begin(), all_available_freg_ids.end() };
    std::set<int, cmp_freg> remained_all_freg_ids = { all_available_freg_ids.begin(), all_available_freg_ids.end() };
    std::map<int,std::set<Interval*>> freg2ActInter;

    std::set<Interval*> active = {};
    Interval *current = nullptr;
    std::map<Value*, Interval*> ival2Inter;
    std::map<Value*, Interval*> fval2Inter;
    Function* func;
    std::list<BasicBlock*> block_order={};
    std::set<Interval*,cmp_interval> interval_list;

    //& spill cost
    const double load_cost = 5.0;
    const double store_cost = 3.0;
    const double loop_scale = 100.0;
    const double mov_cost = 1.0;

    std::map<Value* ,double> spill_cost;

    //& bonus
    std::map<Value* ,std::map<Value*, double>> phi_bonus;
    std::map<Value* ,std::map<int, double>> caller_arg_bonus;
    std::map<Value* ,std::map<int, double>> callee_arg_bonus;
    std::map<Value* ,double> call_bonus;
    std::map<Value* ,double> ret_bonus;
};


#endif