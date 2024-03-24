#include "midend/IRGen.hpp"
#include "midend/Module.hpp"
Module* global_m_ptr;

#define LOG(msg) assert(0 && msg);

#define CONST_INT(num)  ConstantInt::get(num, module.get())
#define CONST_FP(num)   ConstantFP::get(num, module.get())

using namespace IRBuilder;

void IRGen::visit(ast::CompunitNode &node) {
    for (auto &decl : node.global_defs) {
        decl->accept(*this);
    }
}

void IRGen::visit(ast::FuncDef &node) {
    Type *ret_type;
    FunctionType *fun_type;
    Function *fun;

    // process return 
    if(IS_FLOAT(node.type.t)) 
        ret_type = FLOAT_T;
    else if(IS_INT(node.type.t))
        ret_type = INT32_T;
    else
        ret_type = VOID_T;

    // process params
    // get type of param and return value to build func type 
    std::vector<Type*> param_types;
    for(auto &func_param : node.func_f_params) {
        func_param->accept(*this);
        param_types.push_back( cur_type );
    }
    fun_type = FunctionType::get(ret_type, param_types);

    // create current func
    fun = Function::create(fun_type, node.name, module.get());
    scope.pushFunc(node.name, fun);
    cur_fun = fun;
    
    // create entry block, which alloc params
    auto entryBB = BasicBlock::create(module.get(), "entry", fun);
    cur_block_of_cur_fun = entryBB;
    cur_basic_block_list.push_back(entryBB);
    
    // alloc params, it should be in vist funcParams!!!!!
    vector<Value *> args;
    for(auto iter = fun->argBegin(); iter != fun->argEnd(); ++iter)
        args.push_back( *iter );

    // need alloc var for func
    if(args.size() != 0 && ret_type != VOID_T){
        scope.enter();
        from_func = true;
    }
    
    for(int i=0; i<node.func_f_params.size(); i++) {
        if(node.func_f_params[i]->index_num.size() == 0) {
            Value* alloc = AllocaInst::createAlloca(args[i]->getType(), cur_block_of_cur_fun);
            StoreInst::createStore(args[i], alloc, cur_block_of_cur_fun);
            scope.push(node.func_f_params[i]->name, alloc);
        } else {
            Value* alloc_array;
            int total_size = 1;

            alloc_array = AllocaInst::createAlloca(param_types[i], cur_block_of_cur_fun);
            StoreInst::createStore(args[i], alloc_array, cur_block_of_cur_fun);
            scope.push(node.func_f_params[i]->name, alloc_array);
            array_bounds.clear();

            for(auto &bound_expr : node.func_f_params[i]->index_num) {
                if(bound_expr == nullptr) {
                    array_bounds.push_back(1);
                } else {
                    is_init_val = false;
                    bound_expr->accept(*this);
                    auto bound = dynamic_cast<ConstantInt*>(tmp_val);
                    if(bound == nullptr) {
                        LOG( "Array bounds must be const int var or literal" )
                    }
                    array_bounds.push_back(bound->getValue());
                    total_size *= bound->getValue();
                }
            }

            scope.pushSize(node.func_f_params[i]->name, array_bounds);
        }
    }

    // alloc return value
    if(ret_type == FLOAT_T) 
        ret_addr = AllocaInst::createAlloca(FLOAT_T, cur_block_of_cur_fun);
    else if(ret_type == INT32_T)
        ret_addr = AllocaInst::createAlloca(INT32_T, cur_block_of_cur_fun);

    // build func block
    node.body->accept(*this);

    // build return BB    
    ret_BB = BasicBlock::create(module.get(), "ret", fun);

    // cur block don't have terminator, br to ret_bb
    if(cur_block_of_cur_fun->getTerminator() == nullptr) {
        if(cur_fun->getReturnType() == FLOAT_T) {
            StoreInst::createStore(CONST_FP(0), ret_addr, cur_block_of_cur_fun);
        } else if(cur_fun->getReturnType() == INT32_T) {
            StoreInst::createStore(CONST_INT(0), ret_addr, cur_block_of_cur_fun);
        }
        BranchInst::createBr(ret_BB, cur_block_of_cur_fun);
    }

    cur_basic_block_list.pop_back();
    cur_block_of_cur_fun = ret_BB;
    if(fun->getReturnType() == VOID_T) {
        ReturnInst::createVoidRet(ret_BB);
    } else {
      auto ret_val = LoadInst::createLoad(ret_type, ret_addr, ret_BB);
      ReturnInst::createRet(ret_val, ret_BB);
    }
}

void IRGen::visit(ast::FuncFParam &node) {
    type::ValType frontType = node.type;

    if( IS_INT(frontType.t) )
        cur_type = INT32_T;
    else
        cur_type = FLOAT_T;
    
    if( node.index_num.size() == 0)
        return;
    else{
        if(cur_type == INT32_T) 
            cur_type = INT32PTR_T;
        else
            cur_type = FLOATPTR_T;
        return;
    }
}

void IRGen::visit(ast::BlockStmt &node) {
    bool need_enter_scope = !from_func;
    
    from_func = false;
    if(need_enter_scope) {
        scope.enter();
    }

    for(auto &inst : node.block_items){
        inst->accept(*this);
    }

    if(need_enter_scope) {
      scope.exit();
    }
}

void IRGen::visit(ast::ValDeclStmt &node) {
    type::ValType frontType = node.all_type;
    
    if ( IS_INT(frontType.t) )
        cur_type = INT32_T;
    else
        cur_type = FLOAT_T;

    for (auto &def : node.var_def_list){
        is_init_val = true;
        def->accept(*this);
    }
        
}

void IRGen::visit(ast::ValDefStmt &node) {
    is_init_array = false;
    
    // process init_val
    if(node.init_expr != nullptr) {
        node.init_expr->accept(*this);
        auto tmp_int32_val = dynamic_cast<ConstantInt*>(tmp_val);
        auto tmp_float_val = dynamic_cast<ConstantFP*>(tmp_val);

        // int <- const float
        if( cur_type == INT32_T && tmp_float_val != nullptr ){
            tmp_val = CONST_INT(int(tmp_float_val->getValue()));
        // float <- const int
        }else if(cur_type == FLOAT_T && tmp_int32_val != nullptr){
            tmp_val = CONST_FP(float(tmp_int32_val->getValue()));
        // init_val isn't a const
        }else if( tmp_int32_val != nullptr && tmp_float_val != nullptr ){
            // int <- var float
            if( cur_type == INT32_T && tmp_val->getType() == FLOAT_T ){
                tmp_val = FpToSiInst::createFpToSi(tmp_val, INT32_T, cur_block_of_cur_fun);
            // float <- var int
            }else if( cur_type == FLOAT_T && tmp_val->getType() == INT32_T ) {
                tmp_val = SiToFpInst::createSiToFp(tmp_val, FLOAT_T, cur_block_of_cur_fun);
            }
            // tmp_val can be a array ??
            // int <- var int or float <- var float
        }
        // int <- const int or float <- const float
    }else{
        tmp_val = ConstantZero::get(cur_type, module.get());
    }

    // alloc var
    if(scope.inGlobal()) {
        // GlobalVar's init_val must be const ?? fix after!!
        auto var = GlobalVariable::create(node.name, module.get(), cur_type, false, dynamic_cast<Constant *>(tmp_val));
        scope.push(node.name, var);
    } else {
        auto var = AllocaInst::createAlloca(cur_type, cur_block_of_cur_fun);
        StoreInst::createStore(tmp_val, var, cur_block_of_cur_fun);
        scope.push(node.name, var);
    }
}

void IRGen::visit(ast::ConstDeclStmt &node){
    type::ValType frontType = node.all_type;
    
    if ( IS_INT(frontType.t) )
        cur_type = INT32_T;
    else
        cur_type = FLOAT_T;

    for (auto &def : node.var_def_list){
        is_init_val = true;
        def->accept(*this);
    }
}

void IRGen::visit(ast::ConstDefStmt &node){
    is_init_array = false;

    if(node.init_expr != nullptr) {
        node.init_expr->accept(*this);
        auto tmp_int32_val = dynamic_cast<ConstantInt*>(tmp_val);
        auto tmp_float_val = dynamic_cast<ConstantFP*>(tmp_val);

        // int <- const float
        if( cur_type == INT32_T && tmp_float_val != nullptr ){
            tmp_val = CONST_INT(int(tmp_float_val->getValue()));
        // float <- const int
        }else if(cur_type == FLOAT_T && tmp_int32_val != nullptr){
            tmp_val = CONST_FP(float(tmp_int32_val->getValue()));
        // init_val isn't a const
        }else if( tmp_int32_val != nullptr && tmp_float_val != nullptr ){
            LOG( "init value must be const!" )
        }
        // int <- const int or float <- const float
    }else{
        LOG( "const value must be init!" )
    }

    scope.push(node.name, tmp_val);
}

// for scope, any array can have more than one dimensions
// for ir   , any array only has one dimension
void IRGen::visit(ast::ArrDefStmt &node) {
    is_init_array = true;

    arr_total_size = 1;
    ArrayType * array_type;

    array_bounds.clear();
    // array_bounds = {1, array_dimensions_list, 1}
    array_bounds.push_back(1);      
    for(auto &bound_expr : node.array_length) {
        is_init_val = false;
        bound_expr->accept(*this);
        auto bound = dynamic_cast<ConstantInt*>(tmp_val);
        if(bound == nullptr) {
            LOG( "Array bounds must be const int var or literal" )
        }
        array_bounds.push_back(bound->getValue());
        arr_total_size *= bound->getValue();
    }
    array_bounds.push_back(1);
    array_type = ArrayType::get(cur_type, arr_total_size);
    
    cur_depth = 0;
    cur_pos = 0;
    array_pos.clear();
    init_val.clear();
    init_val_map.clear();

    for(int i=0; i<arr_total_size; i++)
        init_val.push_back( ConstantZero::get(cur_type, module.get()) );

    // set all element in array to 0
    // add memset(array, array_len)

    if(node.initializers) {
        is_init_val = true;
        node.initializers->accept(*this);
    }

    if(scope.inGlobal()) {
        if(init_val_map.size() == 0){
            // only print zeroinitializer
            auto initializer = ConstantZero::get(array_type, module.get());
            auto var = GlobalVariable::create(node.name, module.get(), array_type, false, initializer);
            scope.push(node.name, var);
            scope.pushSize(node.name, array_bounds);
        }else{
            // print all elements
            auto initializer = ConstantArray::get(array_type, init_val);
            auto var = GlobalVariable::create(node.name, module.get(), array_type, false, initializer);
            scope.push(node.name, var);
            scope.pushSize(node.name, array_bounds);
        }
    } else {
        auto var = AllocaInst::createAlloca(array_type, cur_block_of_cur_fun);
        if(init_val_map.size() != 0) {
            for(int i = 0; i < arr_total_size; i++) {
                auto elem_addr = GetElementPtrInst::createGep(var, {CONST_INT(0), CONST_INT(i)}, cur_block_of_cur_fun);
                if(init_val_map[i]) {
                    StoreInst::createStore(init_val_map[i], elem_addr, cur_block_of_cur_fun);
                } else {
                    if (cur_type == INT32_T) {
                        StoreInst::createStore(CONST_INT(0), elem_addr, cur_block_of_cur_fun);
                    } else {
                        StoreInst::createStore(CONST_FP(0), elem_addr, cur_block_of_cur_fun);
                    }
                }
            } 
        }
        scope.push(node.name, var);
        scope.pushSize(node.name, array_bounds);
    }
}

void IRGen::visit(ast::ConstArrDefStmt &node) {
    is_init_array = true;

    arr_total_size = 1;
    ArrayType * array_type;

    array_bounds.clear();
    // array_bounds = {1, array_dimensions_list, 1}
    array_bounds.push_back(1);      
    for(auto &bound_expr : node.array_length) {
        is_init_val = false;
        bound_expr->accept(*this);
        auto bound = dynamic_cast<ConstantInt*>(tmp_val);
        if(bound == nullptr) {
            LOG( "Array bounds must be const int var or literal" )
        }
        array_bounds.push_back(bound->getValue());
        arr_total_size *= bound->getValue();
    }
    array_bounds.push_back(1);
    array_type = ArrayType::get(cur_type, arr_total_size);
    
    cur_depth = 0;
    cur_pos = 0;
    array_pos.clear();
    init_val.clear();
    init_val_map.clear();

    for(int i=0; i<arr_total_size; i++)
        init_val.push_back( ConstantZero::get(cur_type, module.get()) );

    // set all element in array to 0
    // add memset(array, array_len)

    if(node.initializers) {
        is_init_val = true;
        node.initializers->accept(*this);
    }

    // how to check const, method using below is so bad!!
    for(auto [index, val]: init_val_map){
        if(dynamic_cast<Constant *>(val) == nullptr){
            LOG( "const array using a no const to init!" );
        }
    }

    if(scope.inGlobal()) {
        if(init_val_map.size() == 0){
            auto initializer = ConstantZero::get(array_type, module.get());
            auto var = GlobalVariable::create(node.name, module.get(), array_type, false, initializer);
            scope.push(node.name, var);
            scope.pushSize(node.name, array_bounds);
            scope.pushConst(node.name, ConstantArray::get(array_type, init_val));
        }else{
            auto initializer = ConstantArray::get(array_type, init_val);
            auto var = GlobalVariable::create(node.name, module.get(), array_type, false, initializer);
            scope.push(node.name, var);
            scope.pushSize(node.name, array_bounds);
            scope.pushConst(node.name, initializer);
        }
    } else {
        auto var = AllocaInst::createAlloca(array_type, cur_block_of_cur_fun);
        if(init_val_map.size() != 0) {
            for(int i = 0; i < arr_total_size; i++) {
                auto elem_addr = GetElementPtrInst::createGep(var, {CONST_INT(0), CONST_INT(i)}, cur_block_of_cur_fun);
                if(init_val_map[i]) {
                    StoreInst::createStore(init_val_map[i], elem_addr, cur_block_of_cur_fun);
                } else {
                    if (cur_type == INT32_T) {
                        StoreInst::createStore(CONST_INT(0), elem_addr, cur_block_of_cur_fun);
                    } else {
                        StoreInst::createStore(CONST_FP(0), elem_addr, cur_block_of_cur_fun);
                    }
                }
            } 
        }
        scope.push(node.name, var);
        scope.pushSize(node.name, array_bounds);
        scope.pushConst(node.name, ConstantArray::get(array_type, init_val));
    }
}

void IRGen::visit(ast::IntConst &node) {
    if(!is_init_val){
        tmp_val = CONST_INT(node.Value.i);
        return;
    }
    
    if(cur_pos >= arr_total_size)
        LOG( "element num in array greater than array bound!" );

    if(cur_type == INT32_T)
        tmp_val = CONST_INT(node.Value.i);
    else
        tmp_val = CONST_FP( float(node.Value.i) );

    // come from visit_arrdef_int
    if(is_init_array){
        init_val[cur_pos] = dynamic_cast<Constant *>(tmp_val);
        init_val_map[cur_pos] = tmp_val;
        cur_pos++;
    }
}

void IRGen::visit(ast::FloatConst &node){
    if(!is_init_val){
        tmp_val = CONST_FP(node.Value.f);
        return;
    }
    
    if(cur_pos >= arr_total_size)
        LOG( "element num in array greater than array bound!" );

    if(cur_type == INT32_T)
        tmp_val = CONST_INT( int(node.Value.f) );
    else
        tmp_val = CONST_FP(node.Value.f);

    // come from visit_arrdef_int
    if(is_init_array){
        init_val[cur_pos] = dynamic_cast<Constant *>(tmp_val);
        init_val_map[cur_pos] = tmp_val;
        cur_pos++;
    }
}

void IRGen::visit(ast::InitializerExpr &node) {
    cur_depth++;

    // max_depth = array_bounds.size() = 1 + array_dimensions + 1
    // cur_depth > max_depth -> only get the first element in last {}
    if(node.initializers.size() < cur_depth){
        node.initializers[0]->accept(*this);
        cur_depth--;
        return;
    }

    // bug, fix after
    if(cur_pos >= arr_total_size)
        LOG( "element num in array greater than array bound!" );

    // cur_depth <= max_depth
    array_pos.push_back( {cur_pos, array_bounds[cur_depth]} );
    for(auto &initializer : node.initializers){
        initializer->accept(*this);
    }
    cur_pos = array_pos.back().first + array_pos.back().second;
    cur_depth--;
}

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
    module = std::make_unique<Module>("Sysy 2024");
    global_m_ptr = module.get();

    VOID_T = Type::getVoidType(module.get());
    INT1_T = Type::getInt1Type(module.get());
    INT32_T = Type::getInt32Type(module.get());
    INT32PTR_T = Type::getInt32PtrType(module.get());
    FLOAT_T = Type::getFloatType(module.get());
    FLOATPTR_T = Type::getFloatPtrType(module.get());

    auto input_type = FunctionType::get(INT32_T, {});
    auto get_int =
        Function::create(
                input_type,
                "getint",
                module.get());

    input_type = FunctionType::get(FLOAT_T, {});
    auto get_float =
        Function::create(
                input_type,
                "getfloat",
                module.get());

    input_type = FunctionType::get(INT32_T, {});
    auto get_char =
        Function::create(
                input_type,
                "getch",
                module.get());

    std::vector<Type *> input_params;
    std::vector<Type *>().swap(input_params);
    input_params.push_back(INT32PTR_T);
    input_type = FunctionType::get(INT32_T, input_params);
    auto get_array =
        Function::create(
                input_type,
                "getarray",
                module.get());

    std::vector<Type *>().swap(input_params);
    input_params.push_back(FLOATPTR_T);
    input_type = FunctionType::get(INT32_T, input_params);
    auto get_farray =
        Function::create(
                input_type,
                "getfarray",
                module.get());

    std::vector<Type *> output_params;
    std::vector<Type *>().swap(output_params);
    output_params.push_back(INT32_T);
    auto output_type = FunctionType::get(VOID_T, output_params);
    auto put_int =
        Function::create(
                output_type,
                "putint",
                module.get());

    std::vector<Type *>().swap(output_params);
    output_params.push_back(FLOAT_T);
    output_type = FunctionType::get(VOID_T, output_params);
    auto put_float =
        Function::create(
                output_type,
                "putfloat",
                module.get());

    std::vector<Type *>().swap(output_params);
    output_params.push_back(INT32_T);
    output_type = FunctionType::get(VOID_T, output_params);
    auto put_char =
        Function::create(
                output_type,
                "putch",
                module.get());

    std::vector<Type *>().swap(output_params);
    output_params.push_back(INT32_T);
    output_params.push_back(INT32PTR_T);
    output_type = FunctionType::get(VOID_T, output_params);
    auto put_array =
        Function::create(
                output_type,
                "putarray",
                module.get());

    std::vector<Type *>().swap(output_params);
    output_params.push_back(INT32_T);
    output_params.push_back(FLOATPTR_T);
    output_type = FunctionType::get(VOID_T, output_params);
    auto put_farray =
        Function::create(
                output_type,
                "putfarray",
                module.get());

    std::vector<Type *>().swap(input_params);
    input_params.push_back(INT32_T);
    auto time_type = FunctionType::get(VOID_T, input_params);
    auto start_time =
        Function::create(
                time_type,
                "_sysy_starttime",
                module.get());

    std::vector<Type *>().swap(input_params);
    input_params.push_back(INT32_T);
    time_type = FunctionType::get(VOID_T, input_params);
    auto stop_time =
        Function::create(
                time_type,
                "_sysy_stoptime",
                module.get());

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
