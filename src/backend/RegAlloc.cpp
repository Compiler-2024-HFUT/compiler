#include<iostream>
#include<cmath>

#include "backend/RegAlloc.hpp"
//#include "logging.hpp"

void Interval::add_range(int from, int to){
    if(range_list.empty()){
        range_list.push_front(new Range(from, to));
        return;
    }
    auto top_range = *range_list.begin();
    if(from>=top_range->from && from<=top_range->to){
        top_range->to = to > top_range->to?to:top_range->to;
    }else if(from < top_range->from){
        if(to <= top_range->to && to>=top_range->from){
            top_range->from = from;
        }else{
            auto new_range = new Range(from,to);
            range_list.push_front(new_range);
        }
    }else{
        auto new_range = new Range(from,to);
        range_list.push_front(new_range);
    }
}

void Interval::add_use_pos(int pos){
    use_position_list.push_front(pos);
}

bool Interval::covers(int id){
    for(auto range:range_list){
        if(range->from<=id&&range->to>id){
            return true;
        }
    }
    return false;
}

bool Interval::covers(Instruction* inst){
    return covers(inst->getId());
}

bool Interval::intersects(Interval* interval){
    auto local_it = range_list.begin();
    auto with_it = interval->range_list.begin();
    while(local_it!=range_list.end() && with_it!=interval->range_list.end()){
        auto local_range = *local_it;
        auto with_range = *with_it;
        if(local_range->to<=with_range->from){
            local_it++;
            continue;
        }else if(with_range->to<=local_range->from){
            with_it++;
            continue;
        }else{
            return true;
        }
    }
    return false;
}

void Interval::union_interval(Interval* interval){
    std::priority_queue<Range*, std::vector<Range*>, cmp_range> all_range;
    for(auto range:range_list){
        all_range.push(range);
    }
    for(auto range:interval->range_list){
        all_range.push(range);
    }
    if(all_range.empty()){
        return;
    }
    range_list.clear();
    auto cur_range = all_range.top();
    all_range.pop();
    while(!all_range.empty()){
        auto merge_range = all_range.top();
        all_range.pop();
        if(merge_range->from > cur_range->to){
            range_list.push_back(cur_range);
            cur_range = merge_range;
        }else{
            cur_range->to = cur_range->to >= merge_range->to?cur_range->to:merge_range->to;
        }
    }
    range_list.push_back(cur_range);
}


//& RegAllocDriver
void RegAllocDriver::compute_reg_alloc(){
    for(auto func:m->getFunctions()){
        if(func->getBasicBlocks().empty()){
            continue;
        }else{
            auto allocator = new RegAlloc(func);
            allocator->execute();
            ireg_alloc[func] = allocator->get_ireg_alloc();
            freg_alloc[func] = allocator->get_freg_alloc();
            bb_order[func] = allocator->get_block_order();
        }
    }   
}


//& RegAlloc
void RegAlloc::execute(){
    init();
    compute_block_order();
    number_operations();
    compute_bonus_and_cost();
    build_intervals();
    walk_intervals();
}


void RegAlloc::init(){
    for(auto ireg_id : remained_all_ireg_ids){
        ireg2ActInter[ireg_id] = std::set<Interval *>();
    }
    for(auto freg_id : remained_all_freg_ids){
        freg2ActInter[freg_id] = std::set<Interval *>();
    }
}

void RegAlloc::compute_block_order(){
    std::priority_queue<BasicBlock*, std::vector<BasicBlock*>, cmp_block_depth> work_list;
    block_order.clear();
    work_list.push(this->func->getEntryBlock());
    while(!work_list.empty()){
        auto cur_bb = work_list.top();
        work_list.pop();
        block_order.push_back(cur_bb);
        for(auto succ_bb : cur_bb->getSuccBasicBlocks()){
            succ_bb->incomingDecrement();
            if(succ_bb->isIncomingZero()){
                work_list.push(succ_bb);
            }
        }
    }
}

void RegAlloc::number_operations(){
    int id = 0;
    for(auto bb : block_order){
        for(auto instr : bb->getInstructions()){
            instr->setId(id);
            id += 2;
        }
    }
}


void RegAlloc::build_intervals(){
    for (auto bb_riter = block_order.rbegin(); bb_riter != block_order.rend();bb_riter++){
        auto bb = *bb_riter;
        auto &instrs = bb->getInstructions();
        int block_from = (*(instrs.begin()))->getId();
        int block_to = (*(instrs.rbegin()))->getId()+2;
        //& for int, not alloc reg for pointer type
        for(auto iopr : bb->getLiveOutInt()){
            if((!dynamic_cast<Instruction*>(iopr) && !dynamic_cast<Argument*>(iopr)) || dynamic_cast<AllocaInst *>(iopr)){
                continue;
            }
            if(ival2Inter.find(iopr)==ival2Inter.end()){
                ival2Inter[iopr] = new Interval(iopr);
            }
            ival2Inter[iopr]->add_range(block_from,block_to);
        }
        //& for float
        for(auto fopr : bb->getLiveOutFloat()){
            if((!dynamic_cast<Instruction*>(fopr) && !dynamic_cast<Argument*>(fopr))||dynamic_cast<AllocaInst *>(fopr)){
                continue;
            }
            if(fval2Inter.find(fopr)==fval2Inter.end()){
                fval2Inter[fopr] = new Interval(fopr);
            }
            fval2Inter[fopr]->add_range(block_from,block_to);
        }
        
        for (auto instr_riter = instrs.rbegin(); instr_riter != instrs.rend();instr_riter++){
            auto instr = *instr_riter;
            if(!instr->isVoid()){
                if(dynamic_cast<AllocaInst *>(instr))continue;
                //for float
                if(instr->getType()->isFloatType()){
                    if(fval2Inter.find(instr) == fval2Inter.end()){
                        fval2Inter[instr] = new Interval(instr);
                        fval2Inter[instr]->add_range(block_from,block_to);
                    }
                    auto top_range = *(fval2Inter[instr]->range_list.begin());
                    top_range->from = instr->getId();
                    fval2Inter[instr]->add_use_pos(instr->getId());
                }
                //for int
                else{
                    if(ival2Inter.find(instr) == ival2Inter.end()){
                        ival2Inter[instr] = new Interval(instr);
                        ival2Inter[instr]->add_range(block_from,block_to);
                    }
                    auto top_range = *(ival2Inter[instr]->range_list.begin());
                    top_range->from = instr->getId();
                    ival2Inter[instr]->add_use_pos(instr->getId());
                }
            }
            if(instr->isPhi()) continue;
            for(auto opr : instr->getOperands()){
                if((!dynamic_cast<Instruction*>(opr) && !dynamic_cast<Argument*>(opr))||dynamic_cast<AllocaInst *>(opr)){
                    continue;
                }
                //for float
                if(opr->getType()->isFloatType()){
                    if(fval2Inter.find(opr)==fval2Inter.end()){
                        fval2Inter[opr] = new Interval(opr);
                        fval2Inter[opr]->add_range(block_from,instr->getId()+2);
                        fval2Inter[opr]->add_use_pos(instr->getId());
                    }
                    else{
                        auto fcur_interval = fval2Inter[opr];
                        fcur_interval->add_range(block_from,instr->getId()+2);
                        fcur_interval->add_use_pos(instr->getId());
                    }
                }
                //for int
                else{
                    if(ival2Inter.find(opr)==ival2Inter.end()){
                        ival2Inter[opr] = new Interval(opr);
                        ival2Inter[opr]->add_range(block_from,instr->getId()+2);
                        ival2Inter[opr]->add_use_pos(instr->getId());
                    }
                    else{
                        auto icur_interval = ival2Inter[opr];
                        icur_interval->add_range(block_from,instr->getId()+2);
                        icur_interval->add_use_pos(instr->getId());
                }
                }
            }
        }
    }
    for(auto pair:ival2Inter){
        add_interval(pair.second);
    }
    for(auto pair:fval2Inter){
        add_interval(pair.second);
    }
}

void RegAlloc::compute_bonus_and_cost() {
    int callee_arg_cnt = 0;
    for(auto arg:func->getIArgs()){
        if(callee_arg_cnt < 8){
            callee_arg_bonus[arg][callee_arg_cnt] += mov_cost;
            spill_cost[arg] += store_cost;
        }//TODO:FOR ARG > 4?
        callee_arg_cnt ++;
    }
    for(auto arg:func->getFArgs()){
        if(callee_arg_cnt < 8){
            callee_arg_bonus[arg][callee_arg_cnt] += mov_cost;
            spill_cost[arg] += store_cost;
        }//TODO:FOR ARG > 4?
        callee_arg_cnt ++;
    }
    for(auto bb:block_order){
        auto loop_depth = bb->getLoopDepth();
        double scale_factor = pow(loop_scale, loop_depth);
        for(auto inst:bb->getInstructions()){
            auto ops = inst->getOperands();
            
            if(inst->isPhi()){
                for(auto op:ops){
                    if(dynamic_cast<BasicBlock*>(op)||dynamic_cast<Constant*>(op)) continue;
                    phi_bonus[inst][op] += mov_cost * scale_factor;
                    phi_bonus[op][inst] += mov_cost * scale_factor;
                }
            }
            
            if(inst->isCall()){
                int caller_iarg_cnt = -1;
                int caller_farg_cnt = -1;
                for(auto op:ops){
                    
                   if(op->getType()->isFloatType()){
                        if(caller_farg_cnt >= 0){
                            if(caller_farg_cnt < 8){
                                caller_arg_bonus[op][caller_farg_cnt] += mov_cost * scale_factor;
                            }
                            caller_farg_cnt ++;
                            continue;
                        }
                        else{
                            caller_farg_cnt ++;
                            continue;
                        }
                   }
                   else{
                        if(caller_iarg_cnt >= 0){
                            if(caller_iarg_cnt < 8){
                                caller_arg_bonus[op][caller_iarg_cnt] += mov_cost * scale_factor;
                            }
                            caller_iarg_cnt ++;
                            continue;
                        }
                        else{
                            caller_iarg_cnt ++;
                            continue;
                        }
                   }
                }
                
                if(!inst->isVoid()){
                    call_bonus[inst] += mov_cost * scale_factor;
                }
            }
            
            if(inst->isRet()){
                if(inst->getNumOperands()){
                    ret_bonus[inst->getOperand(0)] += mov_cost;//TODO:whether to add scale_factor?
                }
            }
            
            if(!inst->isAlloca()&&!inst->isGep()){
                for(auto op:ops){
                    if(dynamic_cast<BasicBlock*>(op)||dynamic_cast<Constant*>(op)||dynamic_cast<GlobalVariable*>(op))
                        continue;
                    spill_cost[op] += load_cost * scale_factor;
                }
            }
            if(inst->isGep()){
                for(auto op:ops){
                    if(dynamic_cast<GlobalVariable*>(op)||dynamic_cast<AllocaInst*>(op)||dynamic_cast<Constant*>(op))
                        continue;
                    spill_cost[op] += load_cost * scale_factor;
                }
            }
            if(!inst->isVoid()){
                if(!inst->isAlloca()){
                    spill_cost[inst] += store_cost * scale_factor;
                }
            }
        }
    }
}


void RegAlloc::walk_intervals(){
    active.clear();
    ireg2ActInter.clear();
    freg2ActInter.clear();
    for (auto cur_it = interval_list.begin(); cur_it != interval_list.end(); cur_it++){
        current = *cur_it;
        auto pos = (*current->range_list.begin())->from;
        std::vector<Interval*> delete_list = {};
        for(auto it : active){
           
            if((*(it->range_list.rbegin()))->to < pos){
                add_reg_to_pool(it);
                delete_list.push_back(it);
            }
        }
        for(auto it: delete_list){
            active.erase(it);
            if(it->is_float_type())
                freg2ActInter[it->reg_id].erase(it);
            else
                ireg2ActInter[it->reg_id].erase(it);
        }

        try_alloc_free_reg();
    }
}


/*
// void RegAlloc::walk_intervals(){
//     active.clear();
//     for (auto cur_it = interval_list.begin(); cur_it != interval_list.end();cur_it++){
//         current = *cur_it;
//         auto pos = (*current->range_list.begin())->from;
        
//         for(auto it : active){
//             if((*(it->range_list.rbegin()))->to < pos){
//                 active.erase(it);
//                 add_reg_to_pool(it);
//                 reg2ActInter[it->reg_id].erase(it);
//             }

//         }
//     }
// }
*/

bool RegAlloc::try_alloc_free_reg(){
    if(!current->is_float_type()){
        if(!remained_all_ireg_ids.empty()){
            double bonus = -1;
            int assigned_id = *remained_all_ireg_ids.begin();
            for(auto new_reg_id:remained_all_ireg_ids){
                double new_bonus = 0;
                for(auto pair:phi_bonus[current->value]){
                    auto phi_val = pair.first;
                    if(!ival2Inter.count(phi_val))continue;
                    if(!ival2Inter[phi_val]->intersects(current)){//TODO:CHECK must be true?;
                        if(ival2Inter[phi_val]->reg_id == new_reg_id){
                            new_bonus += pair.second;
                        }
                    }
                }
                new_bonus += caller_arg_bonus[current->value][new_reg_id];
                new_bonus += callee_arg_bonus[current->value][new_reg_id];
                if(new_reg_id == 0){
                    new_bonus += ret_bonus[current->value];
                    new_bonus += call_bonus[current->value];
                }
                if(new_bonus > bonus){
                    bonus = new_bonus;
                    assigned_id = new_reg_id;
                }
            }
            remained_all_ireg_ids.erase(assigned_id);
            current->reg_id = assigned_id;
            unused_ireg_id.erase(assigned_id);
            active.insert(current);
            ireg2ActInter[assigned_id].insert(current);
            return true;
        }
        else{
            std::set<int, cmp_ireg> spare_reg_id = {};
            for(const auto& pair:ireg2ActInter){
                bool insert_in_hole = true;
                for(auto inter:pair.second){
                    insert_in_hole = insert_in_hole && (!inter->intersects(current));
                }
                if(insert_in_hole){
                    spare_reg_id.insert(pair.first);
                }
            }
            if(!spare_reg_id.empty()){
                int assigned_id = *spare_reg_id.begin();
                double bonus = -1;
                for(auto new_reg_id:spare_reg_id){
                    double new_bonus = 0;
                    for(auto pair:phi_bonus[current->value]){
                        auto phi_val = pair.first;
                        if(!ival2Inter.count(phi_val))continue;
                        if(!ival2Inter[phi_val]->intersects(current)){
                            if(ival2Inter[phi_val]->reg_id == new_reg_id){
                                new_bonus += pair.second;
                            }
                        }
                    }
                    new_bonus += caller_arg_bonus[current->value][new_reg_id];
                    new_bonus += callee_arg_bonus[current->value][new_reg_id];
                    if(new_reg_id == 0){
                        new_bonus += ret_bonus[current->value];
                        new_bonus += call_bonus[current->value];
                    }
                    if(new_bonus > bonus){
                        bonus = new_bonus;
                        assigned_id = new_reg_id;
                    }
                }
                current->reg_id = assigned_id;
                unused_ireg_id.erase(assigned_id);
                active.insert(current);
                ireg2ActInter[assigned_id].insert(current);
                return true;
            }

            
            auto spill_val = current;
            auto min_expire_val = spill_cost[current->value];
            int spilled_reg_id = -1;
            for(const auto& pair:ireg2ActInter){
                double cur_expire_val = 0.0;
                for(auto inter:pair.second){
                    if(inter->intersects(current)){
                        cur_expire_val += spill_cost[inter->value];
                    }
                }
                if(cur_expire_val < min_expire_val){
                    spilled_reg_id = pair.first;
                    min_expire_val = cur_expire_val;
                    spill_val = nullptr;
                }
            }
            if(spill_val==current){
                current->reg_id = -1;
                return false;
            }else{
                if(spilled_reg_id < 0){
                   // LOG(ERROR) << "spilled reg id is -1,something was wrong while register allocation";
                }
                std::set<Interval *> to_spill_set;
                current->reg_id = spilled_reg_id;
                unused_ireg_id.erase(spilled_reg_id);
                for(auto inter:ireg2ActInter.at(spilled_reg_id)){
                    if(inter->intersects(current)){
                        to_spill_set.insert(inter);
                    }
                }
                for(auto spill_inter:to_spill_set){
                    spill_inter->reg_id = -1;
                    active.erase(spill_inter);
                    ireg2ActInter.at(spilled_reg_id).erase(spill_inter);
                }
                ireg2ActInter.at(spilled_reg_id).insert(current);
                active.insert(current);
                return true;
            }
        }
    }
    else{
        if(!remained_all_freg_ids.empty()){
            double bonus = -1;
            int assigned_id = *remained_all_freg_ids.begin();
            for(auto new_reg_id:remained_all_freg_ids){
                double new_bonus = 0;
                for(auto pair:phi_bonus[current->value]){
                    auto phi_val = pair.first;
                    if(!fval2Inter.count(phi_val))continue;
                    if(!fval2Inter[phi_val]->intersects(current)){//TODO:CHECK must be true?;
                        if(fval2Inter[phi_val]->reg_id == new_reg_id){
                            new_bonus += pair.second;
                        }
                    }
                }
                new_bonus += caller_arg_bonus[current->value][new_reg_id];
                new_bonus += callee_arg_bonus[current->value][new_reg_id];
                if(new_reg_id == 0){
                    new_bonus += ret_bonus[current->value];
                    new_bonus += call_bonus[current->value];
                }
                if(new_bonus > bonus){
                    bonus = new_bonus;
                    assigned_id = new_reg_id;
                }
            }
            remained_all_freg_ids.erase(assigned_id);
            current->reg_id = assigned_id;
            unused_freg_id.erase(assigned_id);
            active.insert(current);
            freg2ActInter[assigned_id].insert(current);
            return true;
        }
        else{
            std::set<int, cmp_freg> spare_reg_id = {};
            for(const auto& pair:freg2ActInter){
                bool insert_in_hole = true;
                for(auto inter:pair.second){
                    insert_in_hole = insert_in_hole && (!inter->intersects(current));
                }
                if(insert_in_hole){
                    spare_reg_id.insert(pair.first);
                }
            }
            if(!spare_reg_id.empty()){
                int assigned_id = *spare_reg_id.begin();
                double bonus = -1;
                for(auto new_reg_id:spare_reg_id){
                    double new_bonus = 0;
                    for(auto pair:phi_bonus[current->value]){
                        auto phi_val = pair.first;
                        if(!fval2Inter.count(phi_val))continue;
                        if(!fval2Inter[phi_val]->intersects(current)){
                            if(fval2Inter[phi_val]->reg_id == new_reg_id){
                                new_bonus += pair.second;
                            }
                        }
                    }
                    new_bonus += caller_arg_bonus[current->value][new_reg_id];
                    new_bonus += callee_arg_bonus[current->value][new_reg_id];
                    if(new_reg_id == 0){
                        new_bonus += ret_bonus[current->value];
                        new_bonus += call_bonus[current->value];
                    }
                    if(new_bonus > bonus){
                        bonus = new_bonus;
                        assigned_id = new_reg_id;
                    }
                }
                current->reg_id = assigned_id;
                unused_freg_id.erase(assigned_id);
                active.insert(current);
                freg2ActInter[assigned_id].insert(current);
                return true;
            }
            
            auto spill_val = current;
            auto min_expire_val = spill_cost[current->value];
            int spilled_reg_id = -1;
            for(const auto& pair:freg2ActInter){
                double cur_expire_val = 0.0;
                for(auto inter:pair.second){
                    if(inter->intersects(current)){
                        cur_expire_val += spill_cost[inter->value];
                    }
                }
                if(cur_expire_val < min_expire_val){
                    spilled_reg_id = pair.first;
                    min_expire_val = cur_expire_val;
                    spill_val = nullptr;
                }
            }
            if(spill_val==current){
                current->reg_id = -1;
                return false;
            }else{
                if(spilled_reg_id < 0){
                    //LOG(ERROR) << "spilled reg id is -1,something was wrong while register allocation";
                }
                std::set<Interval *> to_spill_set;
                current->reg_id = spilled_reg_id;
                unused_freg_id.erase(spilled_reg_id);
                for(auto inter:freg2ActInter.at(spilled_reg_id)){
                    if(inter->intersects(current)){
                        to_spill_set.insert(inter);
                    }
                }
                for(auto spill_inter:to_spill_set){
                    spill_inter->reg_id = -1;
                    active.erase(spill_inter);
                    freg2ActInter.at(spilled_reg_id).erase(spill_inter);
                }
                freg2ActInter.at(spilled_reg_id).insert(current);
                active.insert(current);
                return true;
            }
        }
    }
}

void RegAlloc::add_reg_to_pool(Interval* interval){
    int reg_id = current->reg_id;
    if(interval->is_float_type()){
        if(reg_id < 0||reg_id > 31){
            return;
        }
        if(freg2ActInter[reg_id].size()<=1){
            remained_all_freg_ids.insert(reg_id);
        }
        freg2ActInter[reg_id].erase(interval);
    }
    else{
        if(reg_id < 5||reg_id > 31){
            return;
        }
        if(ireg2ActInter[reg_id].size()<=1){
            remained_all_ireg_ids.insert(reg_id);
        }
        ireg2ActInter[reg_id].erase(interval);
    }
}