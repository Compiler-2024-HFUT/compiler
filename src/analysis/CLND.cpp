#include "analysis/CLND.hpp"
#include "midend/BasicBlock.hpp"

void CLND::runOnFunc(Function*func){
    auto initialfunc = initialFunction(func);
    if(initialfunc!=nullptr){
        marker.clear();
        dfn_.clear();
        low_.clear();
        dfn = 0;
        auto bb_entry = initialfunc->getEntryBlock();
        calLoopNestingDepth(bb_entry);
        while(!Loops.empty() ){
            auto loop = Loops.back();
            Loops.pop_back();
            auto bb_loop_entry = *(*loop).rbegin();
            marker[bb_loop_entry] = visited;
            if(!bb_[bb_loop_entry])
               outer_[loop] = bb_[bb_loop_entry]; 
            for(auto bb : *loop){
                bb->loopDepthAdd(1);        //0代表没循环，1代表一层循环
                bb_[bb] = loop;
            }
            
         
            //再来，现在已经分析完了一个函数的一层循环，接下来进入嵌套的一层
            dfn_.clear();
            low_.clear();
            dfn = 0;
        
            for(auto bb_succ : bb_loop_entry->getSuccBasicBlocks()){
                //如果bb_succ没被访问过，并且bb_succ属于循环一部分（如果bb_succ不是循环的一部分，就别谈它里面还有循环嵌套了）
                if(marker[bb_succ]!=visited && bb_[bb_succ]==loop)
                    calLoopNestingDepth(bb_succ);
            }
        }
    

    }
    
}



Function* CLND::initialFunction(Function *func){
    if(func->getNumBasicBlocks()!=0){
        for(auto bb: func->getBasicBlocks())
            bb->loopDepthReset();
        return func;
    }
    return nullptr;


}

//tarjan算法
void CLND::calLoopNestingDepth(BasicBlock* bb){
    dfn++;
    dfn_[bb] = dfn;
    low_[bb] = dfn;
    BBs.push_back(bb);
    marker[bb] = unvisited;
    for(auto bb_succ : bb->getSuccBasicBlocks()){
        if(marker[bb_succ] == visited)
            continue;
        if(dfn_[bb_succ] == 0){
            calLoopNestingDepth(bb_succ);
            low_[bb] = low_[bb]>low_[bb_succ]? low_[bb_succ]:low_[bb];
        }
        else if(marker[bb_succ]==unvisited){
            low_[bb] = low_[bb]>low_[bb_succ]?low_[bb_succ]:low_[bb];
        }
    }

    if(low_[bb] == dfn_[bb]){
        auto bb_vector = new ::std::vector<BasicBlock*>;
        auto bb_element = BBs.back();
        while(bb_element!=bb){
            BBs.pop_back();
            marker[bb_element] = visiting;
            bb_vector->push_back(bb_element);
            bb_element = BBs.back();
        }
        BBs.pop_back();
        marker[bb_element] = visiting;
        bb_vector->push_back(bb_element);
        if(bb_vector->size()>=2){
            Loops.push_back(bb_vector);
            func_loops.push_back(bb_vector);
        }


    }

}