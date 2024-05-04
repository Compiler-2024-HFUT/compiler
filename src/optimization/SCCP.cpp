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

    // replace all of var's with const
    bool isChanged = false;
    for (auto bb : f->getBasicBlocks()) {
        for (auto ii = bb->getInstructions().begin(); ii != bb->getInstructions().end(); ) {
            InstVal &latt = LattValue[*ii];
            if (latt.isConst()) {
                (*ii)->replaceAllUseWith(latt.getConst());
                ii = bb->getInstructions().erase(ii);           // remove the define inst
                isChanged = true;
            }else{
                ++ii;
            }
        }
    }
    return isChanged;
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

Constant *SCCP::foldConst(Instruction *inst) {
    Constant *result;
    Instruction::OpID id = inst->getInstrType();

    Value *a = inst->getOperand(0), *b = inst->getOperand(1);
    ConstantInt *ia, *ib;
    int iav = 0, ibv = 0;
    ConstantFP *fa, *fb;
    float fav = 0.0f, fbv = 0.0f;

    // 确认类型并取值
    if(inst->isIntBinary() || inst->isCmp()){
        ia = dynamic_cast<ConstantInt*>(a);
        ib = dynamic_cast<ConstantInt*>(b);
        assert( (ia && ib) && "a, b can't be int!" );
        iav = ia->getValue();
        ibv = ib->getValue();
    }else if(inst->isFloatBinary() || inst->isFCmp()){
        fa = dynamic_cast<ConstantFP*>(a);
        fb = dynamic_cast<ConstantFP*>(b);
        assert( (fa && fb) && "a, b can't be float!" );
        fav = fa->getValue();
        fbv = fb->getValue();
    }else{
        assert( 0 && "type error!" );
    }

    switch(id){
        // int
        case Instruction::OpID::add: result = (Constant*)ConstantInt::get(iav + ibv); break;
        case Instruction::OpID::sub: result = (Constant*)ConstantInt::get(iav - ibv); break;
        case Instruction::OpID::mul: result = (Constant*)ConstantInt::get(iav * ibv); break;
        case Instruction::OpID::sdiv: result = (Constant*)ConstantInt::get(iav / ibv); break;
        case Instruction::OpID::srem: result = (Constant*)ConstantInt::get(iav % ibv); break;

        // logic
        case Instruction::OpID::land: result = (Constant*)ConstantInt::get(iav & ibv); break;
        case Instruction::OpID::lor: result = (Constant*)ConstantInt::get(iav | ibv); break;
        case Instruction::OpID::lxor: result = (Constant*)ConstantInt::get(iav ^ ibv); break;

        // shift
        case Instruction::OpID::asr: result = (Constant*)ConstantInt::get(iav >> ibv); break;
        case Instruction::OpID::shl: result = (Constant*)ConstantInt::get(iav << ibv); break;
        case Instruction::OpID::lsr: result = (Constant*)ConstantInt::get((int)((unsigned int)iav >> ibv)); break;    // 逻辑右移

        // float 
        case Instruction::OpID::fadd: result = (Constant*)ConstantFP::get(fav + fbv); break;
        case Instruction::OpID::fsub: result = (Constant*)ConstantFP::get(fav - fbv); break;
        case Instruction::OpID::fmul: result = (Constant*)ConstantFP::get(fav * fbv); break;
        case Instruction::OpID::fdiv: result = (Constant*)ConstantFP::get(fav / fbv); break;

        // cmp or fcmp
        case Instruction::OpID::cmp:
        case Instruction::OpID::fcmp:
            CmpOp cmpType = (dynamic_cast<CmpInst*>(inst)) ? dynamic_cast<CmpInst*>(inst)->getCmpOp() : dynamic_cast<FCmpInst*>(inst)->getCmpOp();
            float ca = iav + fav, cb = ibv + fbv;   // maybe bug??
            switch (cmpType) {
                case CmpOp::EQ: result = (Constant*)ConstantInt::get(ca == cb); break;
                case CmpOp::NE: result = (Constant*)ConstantInt::get(ca != cb); break;
                case CmpOp::GT: result = (Constant*)ConstantInt::get(ca >  cb); break;
                case CmpOp::GE: result = (Constant*)ConstantInt::get(ca >= cb); break;
                case CmpOp::LT: result = (Constant*)ConstantInt::get(ca <  cb); break;
                case CmpOp::LE: result = (Constant*)ConstantInt::get(ca <= cb); break;
            }
            break;

        default: assert(0 && "inst opid error!");
    }

    return result;
}

void SCCP::visitInst(Instruction *i) {
    
}

void SCCP::visitPhi(PhiInst *phi) {

}