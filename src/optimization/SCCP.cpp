#include "optimization/SCCP.hpp"

bool SCCP::runOnFunction(Function *f) {
    worklist.clear();
    execFlag.clear();
    LattValue.clear();

    worklist.push_back(Edge::makeFlowEdge(nullptr, f->getEntryBlock()));
    while(!worklist.empty()){
        Edge edge = worklist.back();
        worklist.pop_back();

        if(edge.isFlowEdge() && !getExecFlag(edge)){
            execFlag[edge]++;      // -> execFlag[edge] = true if execFlag[edge]==0
            BasicBlock *dest = (BasicBlock*) edge.edge.second;
            
            for(auto inst : dest->getInstructions()){
                if(PhiInst *phi = dynamic_cast<PhiInst*>(inst))
                    visitPhi(phi);
                else if(getExecFlag(edge) == 1)    // 只有新增的边才会触发遍历基本块全部指令
                    visitInst(inst);   
            }
            
            if(dest->getSuccBasicBlocks().size() == 1)
                addFlowEdge(dest->getSuccBasicBlocks().front());

        }else if(edge.isSSAEdge()){
            Value* use = edge.edge.second;
            if(PhiInst *phi = dynamic_cast<PhiInst*>(use))
                visitPhi(phi);   
            else if(use->getUseList().size() > 0)
                visitInst( (Instruction*) use);
        }
    }

    // replace

}

void SCCP::run() {
    // bool isChanged = false;
    for(auto func : moudle_->getFunctions()){
        runOnFunction(func);
    }
}

int SCCP::getExecFlag(Edge e){
    if(execFlag.find(e) == execFlag.end())
        execFlag[e] = 0;
    return execFlag[e];
}

InstVal &SCCP::getInstVal(Value *v){
    // can find the value's lattice
    if(LattValue.find(v) != LattValue.end())
        return LattValue[v];
    
    // can't find, init the value's lattice
    if(Constant *c = dynamic_cast<Constant*>(v)) {
        LattValue[v].markConst(c);
    }else if(Argument *arg = dynamic_cast<Argument*>(v)) {
        LattValue[v].markNac();
    // 全局常量呢？这样无法处理进程间的传播
    }else if(GlobalVariable *gv = dynamic_cast<GlobalVariable*>(v)) {
        LattValue[v].markNac();
    }

    return LattValue[v];
}

void SCCP::addFlowEdge(BasicBlock *from){
    BasicBlock *to = from->getSuccBasicBlocks().front();
    worklist.push_back(Edge::makeFlowEdge(from, to));
}

void SCCP::addSSAEdge(Value *def){
    for(auto use : def->getUseList()){
        Value *u = use.val_;
        worklist.push_back(Edge::makeSSAEdge(def, u));
    }
}

void SCCP::visitInst(Instruction *i) {

}

void SCCP::visitPhi(PhiInst *phi) {

}

bool SCCP::replaceAllConst() {
    
}