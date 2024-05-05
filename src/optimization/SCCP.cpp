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
        // 仅在有定义的函数上执行
        if(func->getBasicBlocks().size() != 0)
            runOnFunction(func);
    }
}

int SCCP::getExecFlag(Edge e) {
    if(execFlag.find(e) == execFlag.end())
        execFlag[e] = 0;
    return execFlag[e];
}

int SCCP::getExecFlag(PhiInst *phi, BasicBlock *bb) {
    Edge e = Edge::makeFlowEdge(bb, phi->getParent());
    return getExecFlag(e);
}

InstVal &SCCP::getInstVal(Value *v){
    // can find the value's lattice
    if(LattValue.find(v) != LattValue.end())
        return LattValue[v];
    
    // can't find, init the value's lattice
    if(Constant *c = dynamic_cast<Constant*>(v)) {
        LattValue[v].markConst(c);
    }else if(Argument *arg = dynamic_cast<Argument*>(v)) {
        LattValue[v].markNaC();
    // 全局常量呢？这样无法处理进程间的传播
    }else if(GlobalVariable *gv = dynamic_cast<GlobalVariable*>(v)) {
        LattValue[v].markNaC();
    }

    return LattValue[v];
}

void SCCP::addFlowEdge(BasicBlock *from){
    for(auto to : from->getSuccBasicBlocks()) {
        worklist.push_back(Edge::makeFlowEdge(from, to));
    }
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

    Value *a = getInstVal(inst->getOperand(0)).getConst() , *b = getInstVal(inst->getOperand(1)).getConst();
    ConstantInt *ia, *ib;
    int iav = 0, ibv = 0;
    ConstantFP *fa, *fb;
    float fav = 0.0f, fbv = 0.0f;

    // 确认类型并取值
    if(inst->isIntBinary() || inst->isCmp()){
        ia = dynamic_cast<ConstantInt*>(a);
        ib = dynamic_cast<ConstantInt*>(b);
        assert( (ia && ib) && "a, b should be int!" );
        iav = ia->getValue();
        ibv = ib->getValue();
    }else if(inst->isFloatBinary() || inst->isFCmp()){
        fa = dynamic_cast<ConstantFP*>(a);
        fb = dynamic_cast<ConstantFP*>(b);
        assert( (fa && fb) && "a, b should be float!" );
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
        case Instruction::OpID::fcmp: {
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
        }
        default: assert(0 && "inst opid error!");
    }
    return result;
}

/* 暂不考虑以下情况：
 * 函数返回值是固定常数，call指令都是NaC
 * 常量数组元素 ？
 */
void SCCP::visitInst(Instruction *i) {
    
    if(i->isBinary() || i->isCmp() || i->isFCmp()) {
        InstVal &v1 = getInstVal(i->getOperand(0));
        InstVal &v2 = getInstVal(i->getOperand(1));
        if(v1.isNaC() || v2.isNaC()) {
            getInstVal(i).markNaC();
        }else if(v1.isConst() && v2.isConst()){
            Constant *r = foldConst(i);
            getInstVal(i).markConst(r);
        }
    // addon: 如果br是条件跳转，且条件是已知常数，那么这里就不应该添加全部FlowEdge
    }else if(i->isBr()) {
        addFlowEdge(i->getParent());
    }else {
        getInstVal(i).markNaC();
    }

}

void SCCP::visitPhi(PhiInst *phi) {
    Constant *c = getInstVal(phi).getConst();
    if( getInstVal((Instruction*)phi).isNaC() ) return;     // phi's InstVal won't be changed
    
    int size = phi->getOperands().size() / 2;
    for(int i = 0; i < size; ++i){
        // phi的第i个值是undef，无论其是否可达，都不会影响phi的instVal
        if( getInstVal( phi->getOperand(2*i) ).isUndef() )    continue;

        // phi的第i条入边可达
        if( getExecFlag( phi, (BasicBlock*)phi->getOperand(2*i + 1) ) ) {
            // NaC
            if( getInstVal( phi->getOperand(2*i) ).isNaC() ) {
                getInstVal(phi).markNaC();
                // phi's instVal has changed
                addSSAEdge(phi);
                return;
            // constant
            }else {
                // phi is undef
                if(c == nullptr) {
                    c = getInstVal( phi->getOperand(2*i) ).getConst();
                }else {
                    ConstantInt *ic = dynamic_cast<ConstantInt*>(c);
                    ConstantInt *iv = dynamic_cast<ConstantInt*>( getInstVal( phi->getOperand(2*i) ).getConst() );
                    ConstantFP  *fc = dynamic_cast<ConstantFP*>(c);
                    ConstantFP  *fv = dynamic_cast<ConstantFP*>( getInstVal( phi->getOperand(2*i) ).getConst() );
                    if(ic->getValue() == iv->getValue() || fc->getValue() == fv->getValue())
                        continue;
                    else {
                        getInstVal(phi).markNaC();
                        addSSAEdge(phi);
                        return;
                    }
                }
            }
        }
    }

    if(c) {
        getInstVal(phi).markConst(c);
        addSSAEdge(phi);
    }
}