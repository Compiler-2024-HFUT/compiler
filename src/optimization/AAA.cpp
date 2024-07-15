#include"optimization/AAA.hpp"

void ActiveVar::runOnFunc(Function *func){
   
        if(!func->getBasicBlocks().empty()){
            this->func = func;
            ilive_in.clear();
            ilive_out.clear();
            flive_in.clear();
            flive_out.clear();
            map_def.clear();
            map_use.clear();
            get_def_use();
            get_in_out_var();
            for(auto bb : this->func->getBasicBlocks()){
                bb->setLiveInInt(ilive_in[bb]);
                bb->setLiveOutInt(ilive_out[bb]);
                bb->setLiveInFloat(flive_in[bb]);
                bb->setLiveOutFloat(flive_out[bb]);
            }
            /* 
            // std::cout<<"**************************************************"<<std::endl;
            // std::cout << func->print() << std::endl;
            // for (auto bb : func->get_basic_blocks()) {
            //     bb->set_live_in(live_in[bb]);
            //     bb->set_live_out(live_out[bb]);
            //     std::cout<<"======================================="<<std::endl;
            //     std::cout<<"BasicBlock:"<<std::endl;
            //     std::cout << bb->print() << std::endl;
            //     std::cout<<"live_in:"<<std::endl;
            //     for (auto var : live_in[bb]){
            //         std::cout<<var->get_name()<<std::endl;
            //     }
            //     std::cout<<"live_out:"<<std::endl;
            //     for (auto var : live_out[bb]){
            //         std::cout<<var->get_name()<<std::endl;
            //     }
            //     std::cout<<"======================================="<<std::endl;
            // }
            // std::cout<<"**************************************************"<<std::endl;
            */
        }
    
    return;
}


void ActiveVar::get_def_use(){
    for(auto bb : func->getBasicBlocks()){
        std::set<Value*> tmp_def;
        std::set<Value*> tmp_use;
        std::set<Value*> tmp_phi;
        map_def.insert(make_pair(bb, tmp_def));
        map_use.insert(make_pair(bb, tmp_use));
        map_phi.insert(make_pair(bb, tmp_phi));
        for(auto instr : bb->getInstructions()){
            if(!instr->isVoid()) {
                map_def[bb].insert(instr);
            }
            if(instr->getInstrType()==Instruction::OpID::phi){
                map_phi[bb].insert(instr);
            }
            else{
                for (auto op: instr->getOperands()) {
                    //! 跳过全局变量
                    if (dynamic_cast<GlobalVariable *>(op)) continue;
                    if (dynamic_cast<ConstantInt*>(op)) continue;
                    if (dynamic_cast<BasicBlock*>(op)) continue;
                    if (dynamic_cast<Function*>(op)) continue;
                    if (!map_def[bb].count(op))
                            map_use[bb].insert(op); 
                }
            }
        }
    }
}

void ActiveVar::get_in_out_var(){
    for(auto bb : this->func->getBasicBlocks()){
        for(auto val : map_use[bb]){
            if(val->getType()->isFloatType()){
                flive_in[bb].insert(val);
            }
            else{
                ilive_in[bb].insert(val);
            }
        }
        ilive_out.insert({bb, {}});
        flive_out.insert({bb, {}});
    }
    bool changed = true;
    while(changed) {
        changed = false;
        //& each block
        for(auto bb : this->func->getBasicBlocks()){
            auto &iout = ilive_out[bb];
            auto &fout = flive_out[bb];
            //& each succ_block
            for(auto succ_bb : bb->getSuccBasicBlocks()){
                //for int
                for(auto ilvin : ilive_in[succ_bb]){
                    if(map_phi[succ_bb].count(ilvin)){
                        continue;
                    }
                    if(!iout.count(ilvin)){
                        changed = true;
                        iout.insert(ilvin);
                        if (!map_def[bb].count(ilvin)) {
                            ilive_in[bb].insert(ilvin);
                        }

                    }
                }
                //for float
                for(auto flvin : flive_in[succ_bb]){
                    if(map_phi[succ_bb].count(flvin)){
                        continue;
                    }
                    if(!fout.count(flvin)){
                        changed = true;
                        fout.insert(flvin);
                        if (!map_def[bb].count(flvin)) {
                            flive_in[bb].insert(flvin);
                        }

                    }
                }

                std::set<Value *> succ_phi = map_phi[succ_bb];
                //& each phi_instr
                for (auto phi : succ_phi)
                {
                    auto phi_instr = dynamic_cast<PhiInst *>(phi);
                    //& each operand
                    for (int i = 0; i < phi_instr->getOperands().size(); i += 2)
                    {
                        auto vused = phi_instr->getOperand(i);
                        auto pre_bb = dynamic_cast<BasicBlock *>(phi_instr->getOperand(i+1));
                        if(vused->getType()->isFloatType()){
                            if(!fout.count(vused) && pre_bb == bb){
                            changed = true;
                            fout.insert(vused);
                            if (!map_def[bb].count(vused)) {
                                flive_in[bb].insert(vused);
                            }
                        }
                        }
                        else{
                            if(!iout.count(vused) && pre_bb == bb){
                            changed = true;
                            iout.insert(vused);
                            if (!map_def[bb].count(vused)) {
                                ilive_in[bb].insert(vused);
                            }
                        }
                        }

                    }
                }
            }
        }
    }
    for(auto bb : this->func->getBasicBlocks()){
        for (auto phi : map_phi[bb]) {
            auto phi_instr = dynamic_cast<PhiInst *>(phi);
            for (int i = 0; i < phi_instr->getOperands().size(); i += 2) {
                if(phi_instr->getOperand(i)->getType()->isFloatType()){
                    flive_in[bb].erase(phi_instr->getOperand(i));
                }
                else{
                    ilive_in[bb].erase(phi_instr->getOperand(i));
                }
            }
        }
    }
}