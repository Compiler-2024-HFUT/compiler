#include "midend/IRGen.hpp"
#include "midend/Module.hpp"





Module* global_m_ptr;
namespace IRBuilder{

void IRGen::visit(ast::CompunitNode &node) {
   

    for (auto  &def: node.global_defs) {
        def->accept(*this);
    }
}
void IRGen::visit(ast::FuncFParam &node) {

}
void IRGen::visit(ast::FuncDef &node) {
 
}
void IRGen::visit(ast::ValDeclStmt &node) {}
void IRGen::visit(ast::ValDefStmt &node) {}
void IRGen::visit(ast::ArrDefStmt &node) {}
void IRGen::visit(ast::ConstDeclStmt &node){}
void IRGen::visit(ast::ConstDefStmt &node){}
void IRGen::visit(ast::ConstArrDefStmt &node) {}
void IRGen::visit(ast::ExprStmt &node) {}

void IRGen::visit(ast::AssignStmt &node) {}
void IRGen::visit(ast::UnaryExpr &node) {}
void IRGen::visit(ast::AssignExpr &node) {}
void IRGen::visit(ast::RelopExpr &node) {}
void IRGen::visit(ast::EqExpr &node) {}
void IRGen::visit(ast::AndExp &node) {}
void IRGen::visit(ast::ORExp &node){}
void IRGen::visit(ast::BinopExpr &node) {}
void IRGen::visit(ast::LvalExpr &node){}
void IRGen::visit(ast::IntConst &node) {}
void IRGen::visit(ast::InitializerExpr &node) {}
void IRGen::visit(ast::FloatConst &node){}

void IRGen::visit(ast::BlockStmt &node) {
    for(auto &item: node.block_items )
        item->accept(*this);
}

void IRGen::visit(ast::IfStmt &node) {
    auto true_bb = BasicBlock::create(global_m_ptr, "", cur_fun);
    auto false_bb = BasicBlock::create(global_m_ptr, "", cur_fun);
    auto next_bb = BasicBlock::create(global_m_ptr, "", cur_fun);

    IF_WHILE_Cond_Stack.push_back({nullptr, nullptr});
    IF_WHILE_Cond_Stack.back().trueBB = true_bb;
   
    if(node.else_stmt == nullptr){
        IF_WHILE_Cond_Stack.back().falseBB = next_bb;
    }
    else{
        IF_WHILE_Cond_Stack.back().falseBB = false_bb;
    }

   // is_init_val = false;
    node.pred->accept(*this);
    IF_WHILE_Cond_Stack.pop_back();

    //生成比较指令
    Value* inst_cmp;
    if(tmp_val->getType()==INT1_T)  inst_cmp = tmp_val;
    else if(tmp_val->getType()==INT32_T){
        auto tmp_val_const = dynamic_cast<ConstantInt*>(tmp_val);
        if(tmp_val_const){
            inst_cmp = ConstantInt::get(tmp_val_const->getValue()!=0);
        }
        else{
            inst_cmp = CmpInst::createCmp(NE, tmp_val, ConstantInt::get(0), cur_block_of_cur_fun);
        }
    }
    else if(tmp_val->getType()==FLOAT_T){
        auto tmp_val_const = dynamic_cast<ConstantFP*>(tmp_val);
        if(tmp_val_const){
            inst_cmp = ConstantInt::get(tmp_val_const->getValue()!=0);
        }
        else{
            inst_cmp = FCmpInst::createFCmp(NE, tmp_val, ConstantFP::get(0), cur_block_of_cur_fun);
        }
    }

    if(node.else_stmt==nullptr) BranchInst::createCondBr(inst_cmp, true_bb, next_bb, cur_block_of_cur_fun);
    else    BranchInst::createCondBr(inst_cmp, true_bb, false_bb, cur_block_of_cur_fun);

    cur_basic_block_list.pop_back();
    cur_basic_block_list.push_back(true_bb);

    if(dynamic_cast<ast::BlockStmt*>(node.pred.get()))  node.pred->accept(*this);
    else{
        scope.enter();
        node.pred->accept(*this);
        scope.exit();
    }

    if(cur_block_of_cur_fun->getTerminator()==nullptr)  BranchInst::createBr(next_bb, cur_block_of_cur_fun);
    cur_basic_block_list.pop_back();
    if(node.else_stmt==nullptr) false_bb->eraseFromParent();
    else{
        cur_basic_block_list.push_back(false_bb);
        if(dynamic_cast<ast::BlockStmt*>(node.else_stmt.get())) node.else_stmt->accept(*this);
        else{
            scope.enter();
            node.else_stmt->accept(*this);
            scope.exit();
        }
        if(cur_block_of_cur_fun->getTerminator()==nullptr)  BranchInst::createBr(next_bb, cur_block_of_cur_fun);
        cur_basic_block_list.pop_back();
    }
    cur_basic_block_list.push_back(next_bb);
    if(next_bb->getPreBasicBlocks().size()==0){
        next_bb->eraseFromParent();
    }
}
void IRGen::visit(ast::WhileStmt &node){
    auto pred_bb = BasicBlock::create(global_m_ptr, "", cur_fun);
    auto iter_bb = BasicBlock::create(global_m_ptr, "", cur_fun);
    auto next_bb = BasicBlock::create(global_m_ptr, "", cur_fun);
    While_Stack.push_back({pred_bb, next_bb});
    if(cur_block_of_cur_fun->getTerminator()==nullptr)  BranchInst::createBr(pred_bb, cur_block_of_cur_fun);
    cur_basic_block_list.pop_back();
    IF_WHILE_Cond_Stack.push_back({iter_bb, next_bb});
    node.pred->accept(*this);
    IF_WHILE_Cond_Stack.pop_back();
    Value * inst_cmp;
    if(tmp_val->getType()==INT1_T)  inst_cmp = tmp_val;
    else if(tmp_val->getType()==INT32_T){
        auto tmp_val_const = dynamic_cast<ConstantInt*>(tmp_val);
        if(tmp_val_const)   inst_cmp = ConstantInt::get(tmp_val_const->getValue()!=0);
        else inst_cmp = CmpInst::createCmp(NE, tmp_val, ConstantInt::get(0), cur_block_of_cur_fun); 
    }
    else if(tmp_val->getType()==FLOAT_T){
        auto tmp_val_const = dynamic_cast<ConstantFP*>(tmp_val);
        if(tmp_val_const)   inst_cmp = ConstantInt::get(tmp_val_const->getValue()!=0);
        else    inst_cmp = FCmpInst::createFCmp(NE, tmp_val, ConstantFP::get(0), cur_block_of_cur_fun);
    }

    BranchInst::createCondBr(inst_cmp, iter_bb, next_bb, cur_block_of_cur_fun);
    cur_basic_block_list.push_back(iter_bb);
    if(dynamic_cast<ast::BlockStmt*>(node.loop_stmt.get())) node.loop_stmt->accept(*this);
    else{
        scope.enter();
        node.loop_stmt->accept(*this);
        scope.exit();
    }

    if(cur_block_of_cur_fun->getTerminator()==nullptr)  BranchInst::createBr(pred_bb, cur_block_of_cur_fun);
    cur_basic_block_list.pop_back();
    cur_basic_block_list.push_back(next_bb);
    While_Stack.pop_back();
    


}
void IRGen::visit(ast::CallExpr &node) {
    auto called_func = static_cast<Function*>(scope.findFunc(node.call_name));
    std::vector<Value*>params_list;
    int index = 0;
    if(node.call_name == "starttime" || node.call_name == "stoptime"){
        params_list.push_back(ConstantInt::get(node.pos.line));
    }
    else{
        for(auto &param : node.func_r_params){
            auto param_type = called_func->getFunctionType()->getParamType(index++);
            if(param_type->isIntegerType() || param_type->isFloatType()){
                require_lvalue = false;
            }
            else{
                require_lvalue = true;
            }
            param->accept(*this);
            require_lvalue = false;
            //指针类型
            if(param_type->isFloatType() && tmp_val->getType()->isIntegerType()){
                auto tmp_val_const_int = dynamic_cast<ConstantInt*>(tmp_val);
                if(tmp_val_const_int != nullptr){
                    tmp_val = ConstantFP::get( float(tmp_val_const_int->getValue()));
                }
                else{
                    SiToFpInst::createSiToFp(tmp_val, FLOAT_T, cur_block_of_cur_fun);
                }
            }
            else if(param_type->isIntegerType() && tmp_val->getType()->isFloatType()){
                auto tmp_val_const_float = dynamic_cast<ConstantFP*>(tmp_val);
                if(tmp_val_const_float != nullptr){
                    tmp_val = ConstantInt::get( float(tmp_val_const_float->getValue()));
                }
                else{
                    FpToSiInst::createFpToSi(tmp_val, FLOAT_T, cur_block_of_cur_fun);
                }              
            }
            params_list.push_back(tmp_val);
            
        }
    }
    tmp_val = CallInst::createCall(static_cast<Function*>(called_func), params_list, cur_block_of_cur_fun);

}
void IRGen::visit(ast::RetStmt &node) {
    if(node.expr != nullptr){   //有返回值
        node.expr->accept(*this);
        //int
        if(cur_fun->getReturnType()->isIntegerType()){
            auto value = dynamic_cast<ConstantFP*>(tmp_val); 
            if(value != nullptr){
                tmp_val = ConstantInt::get(int(value->getValue()));

            }
            else if(tmp_val->getType()==FLOAT_T){
                tmp_val = FpToSiInst::createFpToSi(tmp_val, INT32_T, cur_block_of_cur_fun);

            }
            StoreInst::createStore(tmp_val, ret_addr, cur_block_of_cur_fun);
        }
        //float
        else{
            auto value = dynamic_cast<ConstantInt*>(tmp_val);
            if(value != nullptr){
                tmp_val = ConstantFP::get(float(value->getValue()));

            }
            else if(tmp_val->getType()==INT32_T){
                tmp_val = SiToFpInst::createSiToFp(tmp_val, FLOAT_T, cur_block_of_cur_fun);


            }
            StoreInst::createStore(tmp_val, ret_addr, cur_block_of_cur_fun);

        }
    }
    BranchInst::createBr(ret_BB, cur_block_of_cur_fun);
}
void IRGen::visit(ast::ContinueStmt &node){
    BranchInst::createBr(While_Stack.back().trueBB,cur_block_of_cur_fun);
}
void IRGen::visit(ast::BreakStmt &node) {
    BranchInst::createBr(While_Stack.back().falseBB, cur_block_of_cur_fun);
}
void IRGen::visit(ast::EmptyStmt &node) {}


IRGen::IRGen() {
    

    m_ = std::make_unique<Module>("Sysy2024 code");

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