#include "midend/IRGen.hpp"
#include "midend/Module.hpp"
Module* global_m_ptr;
namespace IRBuilder{

void IRGen::visit(ast::CompunitNode &node) {
    global_m_ptr=m_.get();
}
void IRGen::visit(ast::FuncFParam &node) {}
void IRGen::visit(ast::FuncDef &node) {}
void IRGen::visit(ast::ValDeclStmt &node) {}
void IRGen::visit(ast::ValDefStmt &node) {}
void IRGen::visit(ast::ArrDefStmt &node) {}
void IRGen::visit(ast::ConstDeclStmt &node){}
void IRGen::visit(ast::ConstDefStmt &node){}
void IRGen::visit(ast::ConstArrDefStmt &node) {}
void IRGen::visit(ast::ExprStmt &node) {}
void IRGen::visit(ast::AssignStmt &node) {}
void IRGen::visit(ast::UnaryExpr &node) {
    node.rhs->accept(*this);
    if(auto tmp=dynamic_cast<ConstantInt*>(tmp_val)){
        switch (node.operat) {
            case::ast::UnOp::PLUS:
                tmp_val=ConstantInt::get(tmp->getValue());
                break;
            case::ast::UnOp::MINUS:
                tmp_val=ConstantInt::get(-tmp->getValue());
                break;
            case::ast::UnOp::NOT:
                tmp_val=ConstantInt::get(!tmp->getValue());
                break;
        }
    }else if(auto tmp=dynamic_cast<ConstantInt*>(tmp_val)){
        switch (node.operat) {
            case::ast::UnOp::PLUS:
                tmp_val=ConstantFP::get(tmp->getValue());
                break;
            case::ast::UnOp::MINUS:
                tmp_val=ConstantFP::get(-tmp->getValue());
                break;
            case::ast::UnOp::NOT:
                tmp_val=ConstantFP::get(!tmp->getValue());
                break;
        }
    }else{
        switch (node.operat) {
        case::ast::UnOp::MINUS:
            if(tmp_val->getType()->isFloatType()){
                Value* lhs = CONST_FP(0);
                Value* rhs = tmp_val;
                BinaryInst::createFSub(lhs, rhs, cur_block_of_cur_fun);
            }else{
                Value* lhs = CONST_INT(0);
                Value* rhs = tmp_val;
                BinaryInst::createSub(lhs, rhs, cur_block_of_cur_fun);    
            }
            break;
        case::ast::UnOp::NOT:{
                auto fcmp_inst = dynamic_cast<FCmpInst*>(tmp_val);
                auto icmp_inst = dynamic_cast<CmpInst*>(tmp_val);
                if(fcmp_inst || icmp_inst) {
                    if(fcmp_inst)
                        fcmp_inst->negation();
                    else
                        icmp_inst->negation();
                } else {
                    if(tmp_val->getType()->isFloatType()) {
                        Value* lhs = tmp_val;
                        Value* rhs = CONST_FP(0);
                        FCmpInst::createFCmp(CmpOp::EQ,lhs, rhs,cur_block_of_cur_fun,m_.get());
                    } else {
                        Value* lhs = tmp_val;
                        Value* rhs = CONST_INT(0);
                        CmpInst::createCmp(CmpOp::EQ,lhs, rhs,cur_block_of_cur_fun,m_.get());
                    }
                }
            }
            break;
        case::ast::UnOp::PLUS:
            break;
        }
    }

}
void IRGen::visit(ast::AssignExpr &node) {
    exit(152);
}
void IRGen::visit(ast::RelopExpr &node) {}
void IRGen::visit(ast::EqExpr &node) {}
void IRGen::visit(ast::AndExp &node) {}
void IRGen::visit(ast::ORExp &node){}
void IRGen::visit(ast::BinopExpr &node) {
    node.lhs->accept(*this);
    Value* lhs=tmp_val,*rhs;
    node.rhs->accept(*this);
    rhs=tmp_val;
    ast::BinOp node_op=node.operat;
    //需要判断浮点数与整数再生成指令
    if(ConstantInt* const_l=dynamic_cast<ConstantInt*>(lhs),*const_r =dynamic_cast<ConstantInt*>(rhs);
    const_l!=nullptr&&const_r!=nullptr){
        switch(node_op){
        case::ast::BinOp::PlUS:
            tmp_val=ConstantInt::get(const_l->getValue()+const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MINUS:
            tmp_val=ConstantInt::get(const_l->getValue()-const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MULTI:
            tmp_val=ConstantInt::get(const_l->getValue()*const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::SLASH:
            tmp_val=ConstantInt::get(const_l->getValue()/const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MOD:
            tmp_val=ConstantInt::get(const_l->getValue()%const_r->getValue(),m_.get());
        case::ast::BinOp::LT:
            tmp_val=ConstantInt::get(const_l->getValue()<const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::LE:
            tmp_val=ConstantInt::get(const_l->getValue()<=const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::GT:
            tmp_val=ConstantInt::get(const_l->getValue()>const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::GE:
            tmp_val=ConstantInt::get(const_l->getValue()>=const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::NOT_EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),m_.get());
            break;
        default:
            exit(151);
        }
    }else if(ConstantFP* const_l=dynamic_cast<ConstantFP*>(lhs),*const_r =dynamic_cast<ConstantFP*>(rhs);
    const_l!=nullptr&&const_r!=nullptr){
        switch(node_op){
        case::ast::BinOp::PlUS:
            tmp_val=ConstantFP::get(const_l->getValue()+const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MINUS:
            tmp_val=ConstantFP::get(const_l->getValue()-const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MULTI:
            tmp_val=ConstantFP::get(const_l->getValue()*const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::SLASH:
            tmp_val=ConstantFP::get(const_l->getValue()/const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::LT:
            tmp_val=ConstantInt::get(const_l->getValue()<const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::LE:
            tmp_val=ConstantInt::get(const_l->getValue()<=const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::GT:
            tmp_val=ConstantInt::get(const_l->getValue()>const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::GE:
            tmp_val=ConstantInt::get(const_l->getValue()>=const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::NOT_EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),m_.get());
        default:
            exit(151);
        }
    }else if(ConstantFP* const_l=dynamic_cast<ConstantFP*>(lhs);
    const_l!=nullptr&&dynamic_cast<ConstantInt*>(lhs)){
        auto const_r=dynamic_cast<ConstantInt*>(lhs);
        switch(node_op){
        case::ast::BinOp::PlUS:
            tmp_val=ConstantFP::get(const_l->getValue()+const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MINUS:
            tmp_val=ConstantFP::get(const_l->getValue()-const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MULTI:
            tmp_val=ConstantFP::get(const_l->getValue()*const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::SLASH:
            tmp_val=ConstantFP::get(const_l->getValue()/const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::LT:
            tmp_val=ConstantInt::get(const_l->getValue()<const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::LE:
            tmp_val=ConstantInt::get(const_l->getValue()<=const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::GT:
            tmp_val=ConstantInt::get(const_l->getValue()>const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::GE:
            tmp_val=ConstantInt::get(const_l->getValue()>=const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::NOT_EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),m_.get());
        default:
            exit(151);
        }
    }else if(ConstantInt* const_l=dynamic_cast<ConstantInt*>(lhs);
    const_l!=nullptr&&dynamic_cast<ConstantFP*>(lhs)){
        auto const_r=dynamic_cast<ConstantFP*>(lhs);
        switch(node_op){
        case::ast::BinOp::PlUS:
            tmp_val=ConstantFP::get(const_l->getValue()+const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MINUS:
            tmp_val=ConstantFP::get(const_l->getValue()-const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::MULTI:
            tmp_val=ConstantFP::get(const_l->getValue()*const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::SLASH:
            tmp_val=ConstantFP::get(const_l->getValue()/const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::LT:
            tmp_val=ConstantInt::get(const_l->getValue()<const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::LE:
            tmp_val=ConstantInt::get(const_l->getValue()<=const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::GT:
            tmp_val=ConstantInt::get(const_l->getValue()>const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::GE:
            tmp_val=ConstantInt::get(const_l->getValue()>=const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),m_.get());
            break;
        case::ast::BinOp::NOT_EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),m_.get());
        default:
            exit(151);
        }
    }else{
        Value* l_instr,* r_instr;
        bool is_float=false;
        if(lhs->getType() == INT1_T && rhs->getType() == INT1_T) {
                l_instr= ZextInst::createZext(lhs, INT32_T,cur_block_of_cur_fun);
                r_instr= ZextInst::createZext(lhs, INT32_T,cur_block_of_cur_fun);
        }else if(lhs->getType() == INT32_T && rhs->getType() == INT32_T) {
                l_instr=lhs;
                r_instr=rhs;
        }else if(lhs->getType() == FLOAT_T && rhs->getType() == FLOAT_T) {
                l_instr=lhs;
                r_instr=rhs;
                is_float=true;
        }else if(lhs->getType() == INT1_T && rhs->getType() == INT32_T) {
                l_instr= ZextInst::createZext(lhs, INT32_T,cur_block_of_cur_fun);
                r_instr=rhs;
        }else if(lhs->getType() == INT1_T && rhs->getType() == FLOAT_T) {
                l_instr= ZextInst::createZext(lhs, INT32_T,cur_block_of_cur_fun); 
                l_instr = SiToFpInst::createSiToFp(l_instr, FLOAT_T,cur_block_of_cur_fun);
                r_instr=rhs;
                is_float=true;       
        }else if(lhs->getType() == INT32_T && rhs->getType() == INT1_T) {
                l_instr=lhs;
                r_instr= ZextInst::createZext(lhs, INT32_T,cur_block_of_cur_fun);   
        }else if(lhs->getType() == INT32_T && rhs->getType() == FLOAT_T) {
                l_instr = SiToFpInst::createSiToFp(l_instr, FLOAT_T,cur_block_of_cur_fun);
                r_instr=rhs;
                is_float=true;       
        }else if(lhs->getType() == FLOAT_T && rhs->getType() == INT1_T) {
                l_instr=lhs;
                r_instr= ZextInst::createZext(lhs, INT32_T,cur_block_of_cur_fun);   
                r_instr = SiToFpInst::createSiToFp(r_instr, FLOAT_T,cur_block_of_cur_fun);
                is_float=true;       
        }else if(lhs->getType() == FLOAT_T && rhs->getType() == INT32_T) {
                l_instr=lhs;
                r_instr = SiToFpInst::createSiToFp(rhs, FLOAT_T,cur_block_of_cur_fun);
                is_float=true;       
        }else{
            exit(152);
        }
        switch(node_op){
        case::ast::BinOp::PlUS:
            if(is_float)
                tmp_val=BinaryInst::createFAdd(l_instr,r_instr,cur_block_of_cur_fun);
            else
                tmp_val=BinaryInst::createAdd(l_instr,r_instr,cur_block_of_cur_fun);
            break;
        case::ast::BinOp::MINUS:
            if(is_float)
                tmp_val=BinaryInst::createFSub(l_instr,r_instr,cur_block_of_cur_fun);
            else
                tmp_val=BinaryInst::createSub(l_instr,r_instr,cur_block_of_cur_fun);
            break;
        case::ast::BinOp::MULTI:
            if(is_float)
                tmp_val=BinaryInst::createFMul(l_instr,r_instr,cur_block_of_cur_fun);
            else
                tmp_val=BinaryInst::createMul(l_instr,r_instr,cur_block_of_cur_fun);
            break;
        case::ast::BinOp::SLASH:
            if(is_float)
                tmp_val=BinaryInst::createFDiv(l_instr,r_instr,cur_block_of_cur_fun);
            else
                tmp_val=BinaryInst::createSDiv(l_instr,r_instr,cur_block_of_cur_fun);
            break;
        default:
            auto binop_to_cmpop=[node_op](){
                switch (node_op) {
                case::ast::BinOp::LT:
                    return CmpOp::LT;
                    break;
                case::ast::BinOp::LE:
                    return CmpOp::LE;
                    break;
                case::ast::BinOp::GT:
                    return CmpOp::GT;
                    break;
                case::ast::BinOp::GE:
                    return CmpOp::GE;
                    break;
                case::ast::BinOp::EQ:
                    return CmpOp::EQ;
                    break;
                case::ast::BinOp::NOT_EQ:
                    return CmpOp::EQ;
                    break;
                default:
                    exit(152);
                    }
                };
            if(is_float)
                FCmpInst::createFCmp(binop_to_cmpop(),lhs,rhs,cur_block_of_cur_fun,m_.get());
            else    
                CmpInst::createCmp(binop_to_cmpop(),lhs,rhs,cur_block_of_cur_fun,m_.get());
        }
    }
}
void IRGen::visit(ast::LvalExpr &node){
    auto var = scope.find(node.name);
    bool should_return_lvalue = require_lvalue;
    require_lvalue = false;
    if(node.index_num.empty()) {
        if(should_return_lvalue) {
            if(var->getType()->getPointerElementType()->isArrayType()) {
                GetElementPtrInst::createGep(var, {CONST_INT(0), CONST_INT(0)},cur_block_of_cur_fun);
            } else if(var->getType()->getPointerElementType()->isPointerType()) {
                tmp_val = LoadInst::createLoad(var->getType(),var,cur_block_of_cur_fun);
            } else {
                tmp_val = var;
            }
        } else {
            if(var->getType() == FLOAT_T) 
                tmp_val = dynamic_cast<ConstantFP*>(var);
            else 
                tmp_val = dynamic_cast<ConstantInt*>(var);
            if(tmp_val==nullptr){
                tmp_val = LoadInst::createLoad(var->getType(),var,cur_block_of_cur_fun);  
            }
        }
    } else {
        auto size = scope.findSize(node.name);
        std::vector<Value*> var_indexs;
        Value *var_index = nullptr;
        int index_const = 0;
        bool const_check = true;
        auto const_array = scope.findConst(node.name);
        if(const_array == nullptr) 
            const_check = false;
        for(int i = 0; i < node.index_num.size(); ++i) {
            node.index_num[i]->accept(*this);
            var_indexs.push_back(tmp_val);
            if(const_check == true) {
                auto tmp_const = dynamic_cast<ConstantInt*>(tmp_val);
                if(tmp_const == nullptr) {
                    const_check = false;
                } else {
                    index_const = size[i+1] * tmp_const->getValue() + index_const;
                }
            }
        }
        if(should_return_lvalue == false && const_check == true) {
            ConstantInt *tmp_const = dynamic_cast<ConstantInt*>(const_array->getElementValue(index_const));
            tmp_val = CONST_INT(tmp_const->getValue());
        } else {
            for(int i = 0; i < var_indexs.size(); i++) {
                auto index_val = var_indexs[i];
                Value* one_index;
                if(size[i+1] > 1) {
                    one_index = BinaryInst::createMul(CONST_INT(size[i+1]), index_val,cur_block_of_cur_fun,m_.get());
                } else {
                    one_index = index_val;
                }
                if(var_index == nullptr) {
                    var_index = one_index;
                } else {
                    var_index=BinaryInst::createAdd(var_index, one_index,cur_block_of_cur_fun,m_.get());
                }
            }
            if(var->getType()->getPointerElementType()->isPointerType()) {
                auto tmp_load = LoadInst::createLoad(var->getType(),var,cur_block_of_cur_fun);
                tmp_val = GetElementPtrInst::createGep(tmp_load, {var_index},cur_block_of_cur_fun);
            } else {
                tmp_val =  GetElementPtrInst::createGep(var, {CONST_INT(0), var_index},cur_block_of_cur_fun);
            }
            if(!should_return_lvalue)
                tmp_val = LoadInst::createLoad(tmp_val->getType(),tmp_val,cur_block_of_cur_fun);
        }
    }
}
void IRGen::visit(ast::IntConst &node) {
    tmp_val=ConstantInt::get(node.Value.i);
}
void IRGen::visit(ast::InitializerExpr &node) {}
void IRGen::visit(ast::FloatConst &node){
    tmp_val=ConstantFP::get(node.Value.f);
}
void IRGen::visit(ast::BlockStmt &node) {}
void IRGen::visit(ast::IfStmt &node) {}
void IRGen::visit(ast::WhileStmt &node){}
void IRGen::visit(ast::CallExpr &node) {}
void IRGen::visit(ast::RetStmt &node) {}
void IRGen::visit(ast::ContinueStmt &node){}
void IRGen::visit(ast::BreakStmt &node) {}
void IRGen::visit(ast::EmptyStmt &node) {}


IRGen::IRGen() {
    

    m_ = std::make_unique<Module>("Sysy2024 code");
    global_m_ptr=m_.get();

    VOID_T = Type::getVoidType(m_.get());
    INT1_T = Type::getInt1Type(m_.get());
    INT32_T = Type::getInt32Type(m_.get());
    INT32PTR_T = Type::getInt32PtrType(m_.get());
    FLOAT_T = Type::getFloatType(m_.get());
    FLOATPTR_T = Type::getFloatPtrType(m_.get());

    auto input_type = FunctionType::get(INT32_T, {});
    auto get_int =
        Function::create(
                input_type,
                "getint",
                m_.get());

    input_type = FunctionType::get(FLOAT_T, {});
    auto get_float =
        Function::create(
                input_type,
                "getfloat",
                m_.get());

    input_type = FunctionType::get(INT32_T, {});
    auto get_char =
        Function::create(
                input_type,
                "getch",
                m_.get());

    std::vector<Type *> input_params;
    std::vector<Type *>().swap(input_params);
    input_params.push_back(INT32PTR_T);
    input_type = FunctionType::get(INT32_T, input_params);
    auto get_array =
        Function::create(
                input_type,
                "getarray",
                m_.get());

    std::vector<Type *>().swap(input_params);
    input_params.push_back(FLOATPTR_T);
    input_type = FunctionType::get(INT32_T, input_params);
    auto get_farray =
        Function::create(
                input_type,
                "getfarray",
                m_.get());

    std::vector<Type *> output_params;
    std::vector<Type *>().swap(output_params);
    output_params.push_back(INT32_T);
    auto output_type = FunctionType::get(VOID_T, output_params);
    auto put_int =
        Function::create(
                output_type,
                "putint",
                m_.get());

    std::vector<Type *>().swap(output_params);
    output_params.push_back(FLOAT_T);
    output_type = FunctionType::get(VOID_T, output_params);
    auto put_float =
        Function::create(
                output_type,
                "putfloat",
                m_.get());

    std::vector<Type *>().swap(output_params);
    output_params.push_back(INT32_T);
    output_type = FunctionType::get(VOID_T, output_params);
    auto put_char =
        Function::create(
                output_type,
                "putch",
                m_.get());

    std::vector<Type *>().swap(output_params);
    output_params.push_back(INT32_T);
    output_params.push_back(INT32PTR_T);
    output_type = FunctionType::get(VOID_T, output_params);
    auto put_array =
        Function::create(
                output_type,
                "putarray",
                m_.get());

    std::vector<Type *>().swap(output_params);
    output_params.push_back(INT32_T);
    output_params.push_back(FLOATPTR_T);
    output_type = FunctionType::get(VOID_T, output_params);
    auto put_farray =
        Function::create(
                output_type,
                "putfarray",
                m_.get());

    std::vector<Type *>().swap(input_params);
    input_params.push_back(INT32_T);
    auto time_type = FunctionType::get(VOID_T, input_params);
    auto start_time =
        Function::create(
                time_type,
                "_sysy_starttime",
                m_.get());

    std::vector<Type *>().swap(input_params);
    input_params.push_back(INT32_T);
    time_type = FunctionType::get(VOID_T, input_params);
    auto stop_time =
        Function::create(
                time_type,
                "_sysy_stoptime",
                m_.get());

    scope.enter();
    scope.pushFunc("getint", get_int);
    scope.pushFunc("getfloat", get_float);
    scope.pushFunc("getch", get_char);
    scope.pushFunc("getarray", get_array);
    scope.pushFunc("getfarray", get_farray);
    scope.pushFunc("putint", put_int);
    scope.pushFunc("putfloat", put_float);
    scope.pushFunc("putch", put_char);
    scope.pushFunc("putarray", put_array);
    scope.pushFunc("putfarray", put_farray);
    scope.pushFunc("starttime", start_time);
    scope.pushFunc("stoptime", stop_time);
    
}

}