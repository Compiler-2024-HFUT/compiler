#include<iostream>
#include<cmath>

#include "backend/LSRA.hpp"


//可以用于分配的寄存器
::std::vector<int> all_alloca_gprs = {5, 6, 7, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

::std::vector<int> all_alloca_fprs = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

::std::map<int, int> gpr_priority = {
    {0, 0},
    {1, 0},
    {2, 0},
    {3, 0},
    {4, 0},
    {5, 25},
    {6, 24},
    {7, 23},
    {8, 0},
    {9, 0},
    {10, 8},
    {11, 7},
    {12, 6},
    {13, 5},
    {14, 4},
    {15, 3},
    {16, 2},
    {17, 1},
    {18, 18},
    {19, 17},
    {20, 16},
    {21, 15},
    {22, 14},
    {23, 13},
    {24, 12},
    {25, 11},
    {26, 10},
    {27, 9},
    {28, 22},
    {29, 21},
    {30, 20},
    {31, 19}
};

::std::map<int, int> fpr_priority = {
    {0, 30},
    {1, 29},
    {2, 28},
    {3, 27},
    {4, 26},
    {5, 25},
    {6, 24},
    {7, 23},
    {8, 0},
    {9, 0},
    {10, 8},
    {11, 7},
    {12, 6},
    {13, 5},
    {14, 4},
    {15, 3},
    {16, 2},
    {17, 1},
    {18, 18},
    {19, 17},
    {20, 16},
    {21, 15},
    {22, 14},
    {23, 13},
    {24, 12},
    {25, 11},
    {26, 10},
    {27, 9},
    {28, 22},
    {29, 21},
    {30, 20},
    {31, 19}
};



void LSRA::run(LiveVar* lv, CIDBB* cidbb, AsmGen* asm_gen){
    for(auto func:module->getFunctions()){
        if(!func->getBasicBlocks().empty()){ 
            regAlloca(func, lv, cidbb);
            asm_gen->ivalue_interval_map[func] = ival_inter_map;
            asm_gen->fvalue_interval_map[func] = fval_inter_map;
        }
}
}

void LSRA::regAlloca(Function* func, LiveVar* lv, CIDBB* cidbb){
    initialize();
    numberBBAndInst(func, cidbb);
    calculateCost(func);
    genIntervals(func, lv);
    traverseIntervals();
    //ivalue_interval_map[func] = ival_inter_map;
    //fvalue_interval_map[func] = fval_inter_map;
    //number_bbs[func]在numberBBAndInst(func, cidbb);里面已经做了
}

void LSRA::initialize(){
    spill_cost.clear();
    phi_bonus.clear();
    callee_argument_bonus.clear();
    caller_argument_bonus.clear();
    call_bonus.clear();
    ret_bonus.clear();

    free_intervals = {};
    //current = nullptr;
    ival_inter_map = {};
    fval_inter_map = {};

    intervals = {};

    free_gprs = {all_alloca_gprs.begin(), all_alloca_gprs.end()};
    free_fprs = {all_alloca_fprs.begin(), all_alloca_fprs.end()};

    unused_gpr = {all_alloca_gprs.begin(), all_alloca_gprs.end()};
    unused_fpr = {all_alloca_gprs.begin(), all_alloca_gprs.end()};

    occupied_intervals = {};


    for(auto greg: all_alloca_gprs)
        gpr_interval_map[greg] = ::std::set<Interval*>();

    for(auto freg: all_alloca_fprs)
        fpr_interval_map[freg] = ::std::set<Interval*>();
}

void LSRA::numberBBAndInst(Function* func, CIDBB* cidbb){
    ::std::priority_queue<BasicBlock*, ::std::vector<BasicBlock*>, cmp_bb_depth > bbs;
    bbs.push(func->getEntryBlock());
    while(!bbs.empty()){
        auto bb = bbs.top();
        bbs.pop();
        number_bbs[func].push_back(bb);
        for(auto succ_bb: bb->getSuccBasicBlocks()){
            cidbb->inDegreeUpdate(succ_bb, -1);
            if(cidbb->getInDegree(succ_bb)==0){
                bbs.push(succ_bb);
            }
        }
    }

    int id = 0;
    for(auto bb: number_bbs[func]){
        for(auto inst: bb->getInstructions()){
            inst->setId(id);
            id+=2;
        }
    }

}



void LSRA::calculateCost(Function* func){
    calculateCalleeArgCost(func);
    for(auto bb: number_bbs[func]){
        for(auto inst: bb->getInstructions()){
            if(inst->isPhi())   calculatePhiCost(func, bb, inst);
            else if(inst->isCall())  calculateCallCost(func, bb, inst);
            else if(inst->isRet())   calculateRetCost(func, bb, inst);
            else if(inst->isGep())   calculateGepCost(func, bb, inst);
            else calculateOtherInstCost(func, bb, inst);
        }
    }
    return;

}



void LSRA::calculateCalleeArgCost(Function* func){
    int callee_argument_num = 0;
    for(auto iarg: func->getIArgs()){
        if(callee_argument_num<8){
            spill_cost[iarg] +=store_cost;
            callee_argument_bonus[iarg][callee_argument_num]+=mov_cost;
        }
        callee_argument_num++;
    }

    for(auto farg: func->getFArgs()){
        if(callee_argument_num<8){
            spill_cost[farg] +=store_cost;
            callee_argument_bonus[farg][callee_argument_num]+=mov_cost;
        }
        callee_argument_num++;
    }

    return;
}

void LSRA::calculatePhiCost(Function* func, BasicBlock* bb, Instruction* inst){
    double amplification_factor = pow(loop_scale, bb->getLoopDepth());
    for(auto op: inst->getOperands()){
        if(!dynamic_cast<BasicBlock*>(op) && !dynamic_cast<Constant*>(op)){
            phi_bonus[inst][op] += mov_cost * amplification_factor;
            phi_bonus[op][inst] += mov_cost * amplification_factor;
            if(!inst->isAlloca() && !inst->isGep() && !dynamic_cast<GlobalVariable*>(op)){
                spill_cost[op] += load_cost * amplification_factor; 
            }
        }
    }
    
    if(!inst->isVoid() && !inst->isAlloca())
        spill_cost[inst] += store_cost * amplification_factor;
    
}

void LSRA::calculateCallCost(Function* func, BasicBlock* bb, Instruction* inst){
    int caller_iargument_number = -1;
    int caller_fargument_number = -1;
    double amplification_factor = pow(loop_scale, bb->getLoopDepth());
    for(auto op: inst->getOperands()){
        if(op->getType()->isFloatType()){
            if(caller_fargument_number>=0 && caller_fargument_number<8){
                caller_argument_bonus[op][caller_fargument_number] += mov_cost * amplification_factor;
            }
            caller_fargument_number++;
        }
        else{
            if(caller_iargument_number>=0 && caller_iargument_number<8){
                caller_argument_bonus[op][caller_iargument_number] += mov_cost * amplification_factor;
            }
            caller_iargument_number++;
        }
        if(!inst->isAlloca() && !inst->isGep() && !dynamic_cast<BasicBlock*>(op) && !dynamic_cast<Constant*>(op) && !dynamic_cast<GlobalVariable*>(op)){
            spill_cost[op] += load_cost * amplification_factor;
        }
    }

    if(!inst->isVoid()) call_bonus[inst] += mov_cost * amplification_factor;
    if(!inst->isVoid() && !inst->isAlloca())
        spill_cost[inst] += store_cost * amplification_factor;


}

void LSRA::calculateRetCost(Function* func, BasicBlock* bb, Instruction* inst){
    double amplification_factor = pow(loop_scale, bb->getLoopDepth());
    int ret_ops_num = inst->getNumOperands();
    if(ret_ops_num){
        auto src = inst->getOperand(0);
        ret_bonus[src] += mov_cost;
    }
    for(auto op: inst->getOperands()){
        if(!inst->isAlloca() && !inst->isGep() && !dynamic_cast<BasicBlock*>(op) && !dynamic_cast<Constant*>(op) && !dynamic_cast<GlobalVariable*>(op))
            spill_cost[op] += load_cost * amplification_factor;
    }
    
    if(!inst->isVoid() && !inst->isAlloca())
        spill_cost[inst] += store_cost * amplification_factor;
}

void LSRA::calculateGepCost(Function* func, BasicBlock* bb, Instruction* inst){
    double amplification_factor = pow(loop_scale, bb->getLoopDepth());
    for(auto op: inst->getOperands()){
        if(!dynamic_cast<GlobalVariable*>(op) && !dynamic_cast<AllocaInst*>(op) && !dynamic_cast<Constant*>(op)){
            spill_cost[op] += load_cost * amplification_factor;
        }
    }
    if(!inst->isVoid() && !inst->isAlloca())
        spill_cost[inst] += store_cost * amplification_factor;
}

void LSRA::calculateOtherInstCost(Function* func, BasicBlock* bb, Instruction* inst){
    double amplification_factor = pow(loop_scale, bb->getLoopDepth());
    for(auto op: inst->getOperands()){
        if(!inst->isAlloca() && !inst->isGep() && !dynamic_cast<BasicBlock*>(op) && !dynamic_cast<Constant*>(op) && !dynamic_cast<GlobalVariable*>(op))
            spill_cost[op] += load_cost * amplification_factor;
    }

    if(!inst->isVoid() && !inst->isAlloca())
        spill_cost[inst] += store_cost * amplification_factor;
}

void LSRA::genIntervals(Function* func, LiveVar* lv){
    for(auto bb_ = number_bbs[func].rbegin(); bb_!=number_bbs[func].rend(); bb_++){
        auto &inst_list = (*bb_)->getInstructions();
        int bb_low = (*(inst_list.begin()))->getId();
        int bb_high = (*(inst_list.rbegin()))->getId()+2;

        genIValIntervalMap((*bb_), lv, bb_low, bb_high);
        genFValIntervalMap((*bb_), lv, bb_low, bb_high);

        for(auto inst_ = inst_list.rbegin(); inst_!=inst_list.rend(); inst_++){
            auto inst = *inst_;
            if(!inst->isVoid()){
                if(!dynamic_cast<AllocaInst*>(inst)){
                    if(inst->getType()->isFloatType()){
                        genFInstIntervalMap(inst, bb_low, bb_high);
                    }
                    else{
                        genIInstIntervalMap(inst, bb_low, bb_high);
                    }
                }
                else continue;
            }
            if(!inst->isPhi()){
                for(auto op: inst->getOperands()){
                    if((dynamic_cast<Instruction*>(op) || dynamic_cast<Argument*>(op)) && (!dynamic_cast<AllocaInst *>(op))){
                        if(op->getType()->isFloatType()){
                            genFInterMap(op, inst, bb_low);
                        }
                        else{
                            genIInterMap(op, inst, bb_low);
                        }
                    }
                }
            }
            
        }
    }
    for(auto i: ival_inter_map)
        intervals.insert(i.second);
    
    for(auto f: fval_inter_map)
        intervals.insert(f.second);

}



void LSRA::genIValIntervalMap(BasicBlock* bb, LiveVar* lv, int bb_low, int bb_high){
    for(auto op: lv->getIntLiveVarOut(bb)){
        if((dynamic_cast<Instruction*>(op) || dynamic_cast<Argument*>(op)) && (!dynamic_cast<AllocaInst *>(op))){
            if(ival_inter_map.find(op)==ival_inter_map.end())
                ival_inter_map[op] = new Interval(op);

            ival_inter_map[op]->addShortInterval(bb_low, bb_high);
        }
    }
}

void LSRA::genFValIntervalMap(BasicBlock* bb, LiveVar* lv, int bb_low, int bb_high){
    for(auto op: lv->getFloatLiveVarOut(bb)){
        if((dynamic_cast<Instruction*>(op) || dynamic_cast<Argument*>(op)) && (!dynamic_cast<AllocaInst *>(op))){
            if(fval_inter_map.find(op)==fval_inter_map.end())
                fval_inter_map[op] = new Interval(op);

            fval_inter_map[op]->addShortInterval(bb_low, bb_high);
        }
    }
}

void LSRA::genFInstIntervalMap(Instruction* inst, int bb_low, int bb_high){
    if(fval_inter_map.find(inst)==fval_inter_map.end()){
        fval_inter_map[inst] = new Interval(inst);
        fval_inter_map[inst]->addShortInterval(bb_low, bb_high);
    }
    auto short_interval = *(fval_inter_map[inst]->small_intervals.begin());
    short_interval->first = inst->getId();
    fval_inter_map[inst]->addUP(inst->getId());
}

void LSRA::genIInstIntervalMap(Instruction* inst, int bb_low, int bb_high){
    if(ival_inter_map.find(inst)==ival_inter_map.end()){
        ival_inter_map[inst] = new Interval(inst);
        ival_inter_map[inst]->addShortInterval(bb_low, bb_high);
    }
    auto short_interval = *(ival_inter_map[inst]->small_intervals.begin());
    short_interval->first = inst->getId();
    ival_inter_map[inst]->addUP(inst->getId());
}

void LSRA::genFInterMap(Value* op, Instruction* inst,  int bb_low){
    if(fval_inter_map.find(op)==fval_inter_map.end()){
        fval_inter_map[op] = new Interval(op);
        fval_inter_map[op]->addShortInterval(bb_low, inst->getId()+2);
        fval_inter_map[op]->addUP(inst->getId());
    }
    else{
        fval_inter_map[op]->addShortInterval(bb_low, inst->getId()+2);
        fval_inter_map[op]->addUP(inst->getId());
    }
}

void LSRA::genIInterMap(Value* op, Instruction* inst, int bb_low){
    if(ival_inter_map.find(op)==ival_inter_map.end()){
        ival_inter_map[op] = new Interval(op);
        ival_inter_map[op]->addShortInterval(bb_low, inst->getId()+2);
        ival_inter_map[op]->addUP(inst->getId());
    }
    else{
        ival_inter_map[op]->addShortInterval(bb_low, inst->getId()+2);
        ival_inter_map[op]->addUP(inst->getId());
    }
}


void LSRA::traverseIntervals(){
    for(auto i=intervals.begin(); i!=intervals.end(); i++){
         auto cur_interval = *i;
        for(auto i_: occupied_intervals){
            if((*(i_->small_intervals.rbegin()))->second<(*cur_interval->small_intervals.begin())->first){
                if(i_->isFloat()){
                    assert(i_->reg>=0 && i_->reg<=31);//continue;
                    if(fpr_interval_map[i_->reg].size()<=1) free_fprs.insert(i_->reg);
                    fpr_interval_map[i_->reg].erase(i_);
                }
                else{
                    assert(i_->reg>=5 && i_->reg<=31);//continue;
                    if(gpr_interval_map[i_->reg].size()<=1) free_gprs.insert(i_->reg);
                    gpr_interval_map[i_->reg].erase(i_);
                }
                free_intervals.push_back(i_);
            }

        }

        for(auto i_: free_intervals){
            occupied_intervals.erase(i_);
            if(i_->isFloat())   fpr_interval_map[i_->reg].erase(i_);
            else gpr_interval_map[i_->reg].erase(i_);
        }

        //试分配FPR
        if(cur_interval->isFloat()){
            //没有空闲寄存器
            if(free_fprs.empty()){
                spare_fprs = {};
                //尝试能不能进行寄存器替换

                findSpareFpr(cur_interval);
                //有寄存器可以替换，选择较优解的寄存器予以替换
                if(!spare_fprs.empty()){


                    replaceFReg(cur_interval);
                   continue;
                }

                bool is_spill;
                int spill_reg =  fSpill(cur_interval, &is_spill);
                if(!is_spill){
                    cur_interval->reg = -1;
                   continue;
                }else{
                    //溢出的完备性，凡是使用溢出寄存器的区间统统溢出
                    assert(spill_reg>=0);
                    allFSpill(cur_interval, spill_reg);
                    continue;
                }
            }
            else{

          replaceTFReg(cur_interval);
                continue;
            }
        }
        else{




                    if(!free_gprs.empty()){
                 spare_gprs = {};

                findSpareFpr(cur_interval);
                if(!spare_gprs.empty()){
                    replaceGReg(cur_interval);
                    continue;
                }

                bool is_spill;
                int spill_reg = gSpill(cur_interval, &is_spill);
                if(!is_spill){
                    cur_interval->reg = -1;
                   continue;
                }else{
                    assert(spill_reg>=0);
                    allGSpill(cur_interval, spill_reg);
                    continue;
                }
            }
            else{

         replaceTGReg(cur_interval);
               continue;
            }
        }

        }
}



void LSRA::findSpareFpr(Interval* cur_interval){
    bool in_mem;
    for(const auto& p:fpr_interval_map){
        in_mem = true;
        for(auto interval:p.second){
            in_mem = in_mem && (!interval->isOverLap(cur_interval));
        }
        if(in_mem){
            spare_fprs.insert(p.first);
        }
    }
}

void LSRA::findSpareGpr(Interval* cur_interval){
    bool in_mem;
    for(const auto& p:gpr_interval_map){
        in_mem = true;
        for(auto interval:p.second){
            in_mem = in_mem && (!interval->isOverLap(cur_interval));
        }
        if(in_mem){
            spare_gprs.insert(p.first);
        }
    }
}

void LSRA::replaceFReg(Interval* cur_interval){
    int assign_reg = *spare_fprs.begin();
    double assign_reward = -1;
    double new_reward;
    for(auto new_reg:spare_fprs){
        new_reward = 0;
        for(auto p:phi_bonus[cur_interval->value]){
            auto phi_val = p.first;
            if(!fval_inter_map.count(phi_val))continue;
            if(!fval_inter_map[phi_val]->isOverLap(cur_interval)){
                if(fval_inter_map[phi_val]->reg == new_reg){
                    new_reward += p.second;
                }
            }
        }
        new_reward += caller_argument_bonus[cur_interval->value][new_reg]+
                      callee_argument_bonus[cur_interval->value][new_reg];
        if(new_reg == 0){
            new_reward += ret_bonus[cur_interval->value]+
                          call_bonus[cur_interval->value];
        }
        if(new_reward > assign_reward){
            assign_reward = new_reward;
            assign_reg = new_reg;
        }
    }
    cur_interval->reg = assign_reg;
    unused_fpr.erase(assign_reg);
    occupied_intervals.insert(cur_interval);
    fpr_interval_map[assign_reg].insert(cur_interval);
}

void LSRA::replaceGReg(Interval* cur_interval){
    int assign_reg = *spare_gprs.begin();
    double assign_reward = -1;
    double new_reward;
    for(auto new_reg:spare_gprs){
        new_reward = 0;
        for(auto p:phi_bonus[cur_interval->value]){
            auto phi_val = p.first;
            if(!ival_inter_map.count(phi_val))continue;
            if(!ival_inter_map[phi_val]->isOverLap(cur_interval)){
                if(ival_inter_map[phi_val]->reg == new_reg){
                    new_reward += p.second;
                }
            }
        }
        new_reward += caller_argument_bonus[cur_interval->value][new_reg]+
                      callee_argument_bonus[cur_interval->value][new_reg];
        if(new_reg == 0){
            new_reward += ret_bonus[cur_interval->value]+
                          call_bonus[cur_interval->value];
        }
        if(new_reward > assign_reward){
            assign_reward = new_reward;
            assign_reg = new_reg;
        }
    }
    cur_interval->reg = assign_reg;
    unused_gpr.erase(assign_reg);
    occupied_intervals.insert(cur_interval);
    gpr_interval_map[assign_reg].insert(cur_interval);
}

int LSRA::fSpill(Interval* cur_interval, bool* is_spill){
    auto spill_interval = cur_interval;
    auto min_cost = spill_cost[cur_interval->value];
    int spill_reg = -1;
    for(const auto& pair:fpr_interval_map){
        double cur_cost = 0.0;
        for(auto inter:pair.second){
            if(inter->isOverLap(cur_interval)){
                cur_cost += spill_cost[inter->value];
            }
        }
        if(cur_cost < min_cost){
            spill_reg = pair.first;  
            min_cost = cur_cost;
            spill_interval = nullptr;
        }
    }
    *is_spill = spill_interval==cur_interval?false:true;
    return spill_reg;
}

int LSRA::gSpill(Interval* cur_interval, bool* is_spill){
    auto spill_interval = cur_interval;
    auto min_cost = spill_cost[cur_interval->value];
    int spill_reg = -1;
    for(const auto& pair:gpr_interval_map){
        double cur_cost = 0.0;
        for(auto inter:pair.second){
            if(inter->isOverLap(cur_interval)){
                cur_cost += spill_cost[inter->value];
            }
        }
        if(cur_cost < min_cost){
            spill_reg = pair.first;  
            min_cost = cur_cost;
            spill_interval = nullptr;
        }
    }
    *is_spill = spill_interval==cur_interval?false:true;
    return spill_reg;
}

void LSRA::allFSpill(Interval* cur_interval, int spill_reg){
    ::std::set<Interval* > spill_intervals;
    cur_interval->reg = spill_reg;
    unused_fpr.erase(spill_reg);
    for(auto interval:fpr_interval_map.at(spill_reg)){
        if(interval->isOverLap(cur_interval)){
            spill_intervals.insert(interval);
        }
    }
    for(auto spill_interval:spill_intervals){
        spill_interval->reg = -1;
        occupied_intervals.erase(spill_interval);
        fpr_interval_map.at(spill_reg).erase(spill_interval);
    }
    fpr_interval_map.at(spill_reg).insert(cur_interval);
    occupied_intervals.insert(cur_interval);
}

void LSRA::allGSpill(Interval* cur_interval, int spill_reg){
    ::std::set<Interval* > spill_intervals;
    cur_interval->reg = spill_reg;
    unused_gpr.erase(spill_reg);
    for(auto interval:gpr_interval_map.at(spill_reg)){
        if(interval->isOverLap(cur_interval)){
            spill_intervals.insert(interval);
        }
    }
    for(auto spill_interval:spill_intervals){
        spill_interval->reg = -1;
        occupied_intervals.erase(spill_interval);
        gpr_interval_map.at(spill_reg).erase(spill_interval);
    }
    gpr_interval_map.at(spill_reg).insert(cur_interval);
    occupied_intervals.insert(cur_interval);
}

void LSRA::replaceTFReg(Interval* cur_interval){
    int assign_reg = *free_fprs.begin();
    double assign_reward = -1;
    double new_reward;
    for(auto new_reg:free_fprs){
        new_reward = 0;
        for(auto p:phi_bonus[cur_interval->value]){
            auto phi_val = p.first;
            if(!fval_inter_map.count(phi_val))continue;
            if(!fval_inter_map[phi_val]->isOverLap(cur_interval)){
                if(fval_inter_map[phi_val]->reg == new_reg){
                    new_reward += p.second;
                }
            }
        }
        new_reward += caller_argument_bonus[cur_interval->value][new_reg]+
                      callee_argument_bonus[cur_interval->value][new_reg];
        if(new_reg == 0){
            new_reward += ret_bonus[cur_interval->value]+
                          call_bonus[cur_interval->value];
        }
        if(new_reward > assign_reward){
            assign_reward = new_reward;
            assign_reg = new_reg;
        }
    }
    cur_interval->reg = assign_reg;
    unused_fpr.erase(assign_reg);
    occupied_intervals.insert(cur_interval);
    fpr_interval_map[assign_reg].insert(cur_interval);
    free_fprs.erase(assign_reg);
}

void LSRA::replaceTGReg(Interval* cur_interval){
    int assign_reg = *free_gprs.begin();
    double assign_reward = -1;
    double new_reward;
    for(auto new_reg:free_gprs){
        new_reward = 0;
        for(auto p:phi_bonus[cur_interval->value]){
            auto phi_val = p.first;
            if(!ival_inter_map.count(phi_val))continue;
            if(!ival_inter_map[phi_val]->isOverLap(cur_interval)){
                if(ival_inter_map[phi_val]->reg == new_reg){
                    new_reward += p.second;
                }
            }
        }
        new_reward += caller_argument_bonus[cur_interval->value][new_reg]+
                      callee_argument_bonus[cur_interval->value][new_reg];
        if(new_reg == 0){
            new_reward += ret_bonus[cur_interval->value]+
                          call_bonus[cur_interval->value];
        }
        if(new_reward > assign_reward){
            assign_reward = new_reward;
            assign_reg = new_reg;
        }
    }
    cur_interval->reg = assign_reg;
    unused_gpr.erase(assign_reg);
    occupied_intervals.insert(cur_interval);
    gpr_interval_map[assign_reg].insert(cur_interval);
    free_gprs.erase(assign_reg);
}