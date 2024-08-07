//该文件里写寄存器分配
#ifndef LSRA_HPP
#define LSRA_HPP


#include <queue>

#include "midend/Module.hpp"
#include "midend/Instruction.hpp"
#include "midend/BasicBlock.hpp"
#include "analysis/Dataflow.hpp"
#include "optimization/PassManager.hpp"
#include "analysis/CIDBB.hpp"
#include "analysis/CLND.hpp"
#include "Interval.hpp"
#include "AsmGen.hpp"


class AsmGen;

//可以用于分配的寄存器
extern::std::vector<int> all_alloca_gprs;

extern::std::vector<int> all_alloca_fprs;

extern::std::map<int, int> gpr_priority;

extern::std::map<int, int> fpr_priority;


class cmp_short_interval_low{
    public:
        bool operator()(::std::pair<int, int>* si_1, ::std::pair<int, int>* si_2 )const{
            return si_1->first>si_2->first;
        }
};
class cmp_gpr_priority{
    public:
        bool  operator()(int greg1, int greg2)const{
            return gpr_priority[greg1]>gpr_priority[greg2];
        }
};

class cmp_fpr_priority{
    public:
        bool operator()(int freg1, int freg2)const{
            return fpr_priority[freg1]>fpr_priority[freg2];
        }
};

class cmp_bb_depth{
    public:
        bool operator()(BasicBlock* block1, BasicBlock* block2)const{
            return block1->getLoopDepth()<block2->getLoopDepth();
        }
};

class cmp_interval{
    public:
        bool operator()(Interval* i1, Interval* i2)const{
            int i1_low = (*(i1->small_intervals.begin()))->first;
            int i2_low = (*(i2->small_intervals.begin()))->first;
            if(i1_low==i2_low){
                return i1->value->getName()<i2->value->getName();
            }
            else{
                return i1_low<i2_low;
            }
        }
};


class LSRA{
    public:
        LSRA(Module *module):module(module){}
        void run(LiveVar* lv, CIDBB* cidbb, AsmGen* asm_gen);

    private:
        void regAlloca(Function* func, LiveVar* lv, CIDBB* cidbb);
        void initialize();
        void numberBBAndInst(Function* func, CIDBB* cidbb);
        void calculateCost(Function* func);
        void calculateCalleeArgCost(Function* func);
        void calculatePhiCost(Function* func, BasicBlock* bb, Instruction* inst);
        void calculateCallCost(Function* func, BasicBlock* bb, Instruction* inst);
        void calculateRetCost(Function* func, BasicBlock* bb, Instruction* inst);
        void calculateGepCost(Function* func, BasicBlock* bb, Instruction* inst);
        void calculateOtherInstCost(Function* func, BasicBlock* bb, Instruction* inst);
        void genIntervals(Function* func, LiveVar* lv);
        void genIValIntervalMap(BasicBlock* bb, LiveVar* lv, int bb_low, int bb_high);
        void genFValIntervalMap(BasicBlock* bb, LiveVar* lv, int bb_low, int bb_high);
        void genFInstIntervalMap(Instruction* inst, int bb_low, int bb_high);
        void genIInstIntervalMap(Instruction* inst, int bb_low, int bb_high);
        void genFInterMap(Value* op, Instruction* inst, int bb_low);
        void genIInterMap(Value* op, Instruction* inst, int bb_low);
        void traverseIntervals();
        void findSpareFpr(Interval* cur_interval);
        void findSpareGpr(Interval* cur_interval);
        void replaceFReg(Interval* cur_interval);
        void replaceGReg(Interval* cur_interval);
        int fSpill(Interval* cur_interval, bool* is_spill);
        int gSpill(Interval* cur_interval, bool* is_spill);
        void allFSpill(Interval* cur_interval, int spill_reg);
        void allGSpill(Interval* cur_interval, int spill_reg);
        void replaceTFReg(Interval* cur_interval);
        void replaceTGReg(Interval* cur_interval);

    private:

        ::std::map<Function*, std::map<Value*, Interval*>> ivalue_interval_map;
        ::std::map<Function*, std::map<Value*, Interval*>> fvalue_interval_map;
        ::std::map<Function*, std::vector<BasicBlock*>> number_bbs;
        Module *module;
       //Function* func;

        //空闲的寄存器
        ::std::set<int, cmp_gpr_priority> free_gprs;
        ::std::set<int, cmp_fpr_priority> free_fprs;


        ::std::set<int> unused_gpr;
        ::std::set<int> unused_fpr;

        ::std::set<int, cmp_gpr_priority> spare_gprs;
        ::std::set<int, cmp_fpr_priority> spare_fprs;

        //分配的寄存器与其对应的活跃区间的映射
        ::std::map<int, ::std::set<Interval*>> gpr_interval_map;
        ::std::map<int, ::std::set<Interval*>> fpr_interval_map;

        double mov_cost = 1.0;
        double load_cost = 5.0;
        double store_cost = 3.0;
        double loop_scale = 100.0;

        ::std::map<Value*, double> spill_cost;

        ::std::map<Value* ,std::map<Value*, double>> phi_bonus;
        ::std::map<Value* ,std::map<int, double>> caller_argument_bonus;
        ::std::map<Value* ,std::map<int, double>> callee_argument_bonus;
        ::std::map<Value* ,double> call_bonus;
        ::std::map<Value* ,double> ret_bonus; 

        //变量与活跃区间的映射
        ::std::map<Value*, Interval*> ival_inter_map;
        ::std::map<Value*, Interval*> fval_inter_map;

        //活跃区间列表
        ::std::set<Interval*, cmp_interval> intervals;
    
        //当前正在占有寄存器的活跃区间集合
        ::std::set<Interval*> occupied_intervals;

        //还未处理过的活跃区间集合
        ::std::vector<Interval*> free_intervals;

        
};

#endif