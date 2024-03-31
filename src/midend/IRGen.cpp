#include "midend/IRGen.hpp"
#include "frontend/type.hpp"
#include "midend/Constant.hpp"
#include "midend/Instruction.hpp"
#include "midend/Module.hpp"
#include "midend/Type.hpp"
Module* global_m_ptr;

#define LOG(msg) assert(0 && msg);


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

            // array_bounds = {1, array_dimensions_list, 1}
            array_bounds.push_back(1);
            for(auto &bound_expr : node.func_f_params[i]->index_num) {
                if(bound_expr == nullptr) {
                    array_bounds.push_back(1);
                } else {
                    bound_expr->accept(*this);
                    auto bound = dynamic_cast<ConstantInt*>(tmp_val);
                    if(bound == nullptr) {
                        LOG( "Array bounds must be const int var or literal" )
                    }
                    array_bounds.push_back(bound->getValue());
                    total_size *= bound->getValue();
                }
            }
            array_bounds.push_back(1);

            std::list<int> array_sizes_l = {1};
            for(auto iter_r = array_bounds.rbegin()+1; iter_r != array_bounds.rend()-1; iter_r++){
                array_sizes_l.push_front(array_sizes_l.front() * *iter_r);
            }
            std::vector<int> array_sizes = std::vector<int>(array_sizes_l.begin(), array_sizes_l.end());
            scope.pushSize(node.func_f_params[i]->name, array_sizes);
        }
    }

    // alloc return value
    if(ret_type == FLOAT_T) 
        ret_addr = AllocaInst::createAlloca(FLOAT_T, cur_block_of_cur_fun);
    else if(ret_type == INT32_T)
        ret_addr = AllocaInst::createAlloca(INT32_T, cur_block_of_cur_fun);

   

    // build return BB    
    ret_BB = BasicBlock::create(module.get(), "ret", fun);

     // build func block
    node.body->accept(*this);

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
        def->accept(*this);
    }
        
}

void IRGen::visit(ast::ValDefStmt &node) {
    // is_init_array = false;
    
    // process init_val
    if(node.init_expr != nullptr) {
        node.init_expr->accept(*this);
        auto tmp_int32_val = dynamic_cast<ConstantInt*>(tmp_val);
        auto tmp_float_val = dynamic_cast<ConstantFP*>(tmp_val);
        // init_val isn't a const
        if(tmp_float_val==nullptr&&tmp_int32_val==nullptr){
            // int <- var float
            if( cur_type == INT32_T && tmp_val->getType()->isFloatType() ){
                tmp_val = FpToSiInst::createFpToSi(tmp_val, INT32_T, cur_block_of_cur_fun);
            // float <- var int
            }else if( cur_type == FLOAT_T && tmp_val->getType() ->isIntegerType() ) {
                tmp_val = SiToFpInst::createSiToFp(tmp_val, FLOAT_T, cur_block_of_cur_fun);
            }
            // int <- var int or float <- var float
        
        // int <- const float
        }else if( cur_type == INT32_T && tmp_float_val != nullptr ){
            tmp_val = CONST_INT(int(tmp_float_val->getValue()));
        // float <- const int
        }else if(cur_type == FLOAT_T && tmp_int32_val != nullptr){
            tmp_val = CONST_FP(float(tmp_int32_val->getValue()));
        
        }
        // int <- const int or float <- const float
    }else{
        if(scope.inGlobal())
            tmp_val = ConstantZero::get(cur_type, module.get());
    }

    // alloc var
    if(scope.inGlobal()) {
        // GlobalVar's init_val must be const ?? fix after!!
        auto var = GlobalVariable::create(node.name, module.get(), cur_type, false, dynamic_cast<Constant *>(tmp_val));
        scope.push(node.name, var);
    } else {
        auto var = AllocaInst::createAlloca(cur_type, cur_block_of_cur_fun);
        if(node.init_expr)
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
        def->accept(*this);
    }
}

void IRGen::visit(ast::ConstDefStmt &node){
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
        }else if( tmp_int32_val == nullptr && tmp_float_val == nullptr ){
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
    arr_total_size = 1;
    ArrayType * array_type;

    array_bounds.clear();
    // array_bounds = {1, array_dimensions_list, 1}
    array_bounds.push_back(1);      
    for(auto &bound_expr : node.array_length) {
        require_lvalue=false;
        bound_expr->accept(*this);
        auto bound = dynamic_cast<ConstantInt*>(tmp_val);
        if(bound == nullptr) {
            LOG( "Array bounds must be const int var or literal" )
        }else if(bound->getValue() < 0){
            LOG( "Array bounds must be greater than 0" )
        }

        array_bounds.push_back(bound->getValue());
        arr_total_size *= bound->getValue();
    }
    array_bounds.push_back(1);
    array_type = ArrayType::get(cur_type, arr_total_size);
    
    std::list<int> array_sizes_l = {1};
    for(auto iter_r = array_bounds.rbegin()+1; iter_r != array_bounds.rend()-1; iter_r++){
        array_sizes_l.push_front(array_sizes_l.front() * *iter_r);
    }
    std::vector<int> array_sizes = std::vector<int>(array_sizes_l.begin(), array_sizes_l.end());

    cur_depth = 0;
    cur_pos = 0;
    array_pos.clear();
    init_val.clear();
    init_val_map.clear();

    for(int i=0; i<arr_total_size; i++){
        if(cur_type == INT32_T){
            init_val.push_back( CONST_INT(0) );
        }else{
            init_val.push_back( CONST_FP(0.0) );
        }
    }

    // set all element in array to 0
    // add memset(array, array_len)

    if(node.initializers) {
        node.initializers->accept(*this);
    }

    if(scope.inGlobal()) {
        if(init_val_map.size() == 0){
            // only print zeroinitializer
            auto initializer = ConstantZero::get(array_type, module.get());
            auto var = GlobalVariable::create(node.name, module.get(), array_type, false, initializer);
            scope.push(node.name, var);
            scope.pushSize(node.name, array_sizes);
        }else{
            // print all elements
            auto initializer = ConstantArray::get(array_type, init_val);
            auto var = GlobalVariable::create(node.name, module.get(), array_type, false, initializer);
            scope.push(node.name, var);
            scope.pushSize(node.name, array_sizes);
        }
    } else {
        auto var = AllocaInst::createAlloca(array_type, cur_block_of_cur_fun);
        
        for(auto [offset, value] : init_val_map){
            auto elem_addr = GetElementPtrInst::createGep(var, {CONST_INT(0), CONST_INT(offset)}, cur_block_of_cur_fun);
            StoreInst::createStore(value, elem_addr, cur_block_of_cur_fun);
        }

        // if(init_val_map.size() != 0) {
        //     for(int i = 0; i < arr_total_size; i++) {
        //         auto elem_addr = GetElementPtrInst::createGep(var, {CONST_INT(0), CONST_INT(i)}, cur_block_of_cur_fun);
        //         if(init_val_map[i]) {
        //             StoreInst::createStore(init_val_map[i], elem_addr, cur_block_of_cur_fun);
        //         } else {
        //             if (cur_type == INT32_T) {
        //                 StoreInst::createStore(CONST_INT(0), elem_addr, cur_block_of_cur_fun);
        //             } else {
        //                 StoreInst::createStore(CONST_FP(0), elem_addr, cur_block_of_cur_fun);
        //             }
        //         }
        //     } 
        // }
        scope.push(node.name, var);
        scope.pushSize(node.name, array_sizes);
    }
}

void IRGen::visit(ast::ConstArrDefStmt &node) {
    arr_total_size = 1;
    ArrayType * array_type;

    array_bounds.clear();
    // array_bounds = {1, array_dimensions_list, 1}
    array_bounds.push_back(1);      
    for(auto &bound_expr : node.array_length) {
        bound_expr->accept(*this);
        auto bound = dynamic_cast<ConstantInt*>(tmp_val);
        if(bound == nullptr) {
            LOG( "Array bounds must be const int var or literal" )
        }else if(bound->getValue() < 0){
            LOG( "Array bounds must be greater than 0" )
        }

        array_bounds.push_back(bound->getValue());
        arr_total_size *= bound->getValue();
    }
    array_bounds.push_back(1);
    array_type = ArrayType::get(cur_type, arr_total_size);
    
    std::list<int> array_sizes_l = {1};
    for(auto iter_r = array_bounds.rbegin()+1; iter_r != array_bounds.rend()-1; iter_r++){
        array_sizes_l.push_front(array_sizes_l.front() * *iter_r);
    }
    std::vector<int> array_sizes = std::vector<int>(array_sizes_l.begin(), array_sizes_l.end());

    cur_depth = 0;
    cur_pos = 0;
    array_pos.clear();
    init_val.clear();
    init_val_map.clear();

    for(int i=0; i<arr_total_size; i++){
        if(cur_type == INT32_T){
            init_val.push_back( CONST_INT(0) );
        }else{
            init_val.push_back( CONST_FP(0.0) );
        }
    }

    // set all element in array to 0
    // add memset(array, array_len)

    if(node.initializers) {
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
            scope.pushSize(node.name, array_sizes);
            scope.pushConst(node.name, ConstantArray::get(array_type, init_val));
        }else{
            auto initializer = ConstantArray::get(array_type, init_val);
            auto var = GlobalVariable::create(node.name, module.get(), array_type, false, initializer);
            scope.push(node.name, var);
            scope.pushSize(node.name, array_sizes);
            scope.pushConst(node.name, initializer);
        }
    } else {
        auto var = AllocaInst::createAlloca(array_type, cur_block_of_cur_fun);
        
        for(auto [offset, value] : init_val_map){
            auto elem_addr = GetElementPtrInst::createGep(var, {CONST_INT(0), CONST_INT(offset)}, cur_block_of_cur_fun);
            StoreInst::createStore(value, elem_addr, cur_block_of_cur_fun);
        }

        // if(init_val_map.size() != 0) {
        //     for(int i = 0; i < arr_total_size; i++) {
        //         auto elem_addr = GetElementPtrInst::createGep(var, {CONST_INT(0), CONST_INT(i)}, cur_block_of_cur_fun);
        //         if(init_val_map[i]) {
        //             StoreInst::createStore(init_val_map[i], elem_addr, cur_block_of_cur_fun);
        //         } else {
        //             if (cur_type == INT32_T) {
        //                 StoreInst::createStore(CONST_INT(0), elem_addr, cur_block_of_cur_fun);
        //             } else {
        //                 StoreInst::createStore(CONST_FP(0), elem_addr, cur_block_of_cur_fun);
        //             }
        //         }
        //     } 
        // }
        scope.push(node.name, var);
        scope.pushSize(node.name, array_sizes);
        scope.pushConst(node.name, ConstantArray::get(array_type, init_val));
    }
}

void IRGen::visit(ast::IntConst &node) {
    tmp_val = CONST_INT(node.Value.i);
}

void IRGen::visit(ast::FloatConst &node){
    tmp_val = CONST_FP(node.Value.f);
}

void IRGen::visit(ast::InitializerExpr &node) {
    cur_depth++;

    // max_depth = array_bounds.size() = 1 + array_dimensions + 1
    // cur_depth > max_depth -> only get the first element in last {}
    if(array_bounds.size()-1 < cur_depth){
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

        if(dynamic_cast<ast::InitializerExpr*>(initializer.get()) != nullptr)
            continue;

        if(cur_pos >= arr_total_size)
            LOG( "element num in array greater than array bound!" );

        // tmp_val is const
        auto tmp_int32_val = dynamic_cast<ConstantInt*>(tmp_val);
        auto tmp_float_val = dynamic_cast<ConstantFP*>(tmp_val);

        if(cur_type == INT32_T && tmp_float_val != nullptr){
            tmp_val = CONST_INT( int(tmp_float_val->getValue()) );
        }else if(cur_type == FLOAT_T && tmp_int32_val != nullptr){
            tmp_val = CONST_FP( float(tmp_int32_val->getValue()) );
        }

        if(scope.inGlobal())
            init_val[cur_pos] = dynamic_cast<Constant *>(tmp_val);

        // tmp_val is const and var
        init_val_map[cur_pos] = tmp_val;
        cur_pos++;
    }
    cur_pos = array_pos.back().first + array_pos.back().second;
    cur_depth--;
}

void IRGen::visit(ast::ExprStmt &node) {
    node.expr->accept(*this);
}

void IRGen::visit(ast::AssignStmt &node) {
    require_lvalue = false;
    node.expr->accept(*this);
    auto result = tmp_val;
    require_lvalue = true;
    node.l_val->accept(*this);
    auto addr = tmp_val;
    if (addr->getType()->getPointerElementType()->isIntegerType() && result->getType()->isFloatType()) {
        auto const_result = dynamic_cast<ConstantFP*>(result);
        if(const_result) {
            result = CONST_INT(int(const_result->getValue()));
        } else {
            FpToSiInst::createFpToSi(result, INT32_T,cur_block_of_cur_fun);
        }
    } else if (addr->getType()->getPointerElementType()->isFloatType() && result->getType()->isIntegerType()) {
        auto const_result = dynamic_cast<ConstantInt*>(result);
        if(const_result) {
            result = CONST_FP(float(const_result->getValue()));
        } else {
            SiToFpInst::createSiToFp(result, FLOAT_T,cur_block_of_cur_fun);
        }
    }
    StoreInst::createStore(result, addr,cur_block_of_cur_fun);
    tmp_val = result;
}
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
                        FCmpInst::createFCmp(CmpOp::EQ,lhs, rhs,cur_block_of_cur_fun,module.get());
                    } else {
                        Value* lhs = tmp_val;
                        Value* rhs = CONST_INT(0);
                        CmpInst::createCmp(CmpOp::EQ,lhs, rhs,cur_block_of_cur_fun,module.get());
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
void IRGen::visit(ast::AndExp &node) {
    require_lvalue = false;
    auto true_BB = BasicBlock::create(module.get(), "", cur_fun);
    node.lhs->accept(*this);
        
        Value *cond_val;
        if(tmp_val->getType() == INT1_T) {
            cond_val = tmp_val;
        } else if(tmp_val->getType() == INT32_T) {
            auto const_tmp_val = dynamic_cast<ConstantInt*>(tmp_val);
            if(const_tmp_val) {
                cond_val = CONST_INT(const_tmp_val->getValue() != 0);
            } else {
                cond_val=CmpInst::createCmp(CmpOp::NE,tmp_val, CONST_INT(0),cur_block_of_cur_fun,module.get());
            }
        } else if(tmp_val->getType() == FLOAT_T) {
            auto const_tmp_val = dynamic_cast<ConstantFP*>(tmp_val);
            if(const_tmp_val) {
                cond_val = CONST_INT(const_tmp_val->getValue() != 0);
            } else {
                cond_val =FCmpInst::createFCmp(CmpOp::NE,tmp_val, CONST_FP(0),cur_block_of_cur_fun,module.get());
            }
        }

    BranchInst::createCondBr(cond_val, true_BB, IF_WHILE_Cond_Stack.back().falseBB, cur_block_of_cur_fun);
    cur_block_of_cur_fun=true_BB;
    node.rhs->accept(*this);
}
void IRGen::visit(ast::ORExp &node){
    require_lvalue = false;
    auto false_BB = BasicBlock::create(module.get(), "", cur_fun);
    IF_WHILE_Cond_Stack.push_back({IF_WHILE_Cond_Stack.back().trueBB, false_BB});
    node.lhs->accept(*this);
    IF_WHILE_Cond_Stack.pop_back();
        
        Value *cond_val;
        if(tmp_val->getType() == INT1_T) {
            cond_val = tmp_val;
        } else if(tmp_val->getType() == INT32_T) {
            auto const_tmp_val = dynamic_cast<ConstantInt*>(tmp_val);
            if(const_tmp_val) {
                cond_val = CONST_INT(const_tmp_val->getValue() != 0);
            } else {
                CmpInst::createCmp(CmpOp::NE,tmp_val, CONST_INT(0),cur_block_of_cur_fun,module.get());
            }
        } else if(tmp_val->getType() == FLOAT_T) {
            auto const_tmp_val = dynamic_cast<ConstantFP*>(tmp_val);
            if(const_tmp_val) {
                cond_val = CONST_INT(const_tmp_val->getValue() != 0);
            } else {
                cond_val =FCmpInst::createFCmp(CmpOp::NE,tmp_val, CONST_FP(0),cur_block_of_cur_fun,module.get());
            }
        }

    BranchInst::createCondBr(cond_val, IF_WHILE_Cond_Stack.back().trueBB, false_BB,cur_block_of_cur_fun);
    cur_block_of_cur_fun=false_BB;
    node.rhs->accept(*this);
}
void IRGen::visit(ast::BinopExpr &node) {
    require_lvalue = false;
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
            tmp_val=ConstantInt::get(const_l->getValue()+const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MINUS:
            tmp_val=ConstantInt::get(const_l->getValue()-const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MULTI:
            tmp_val=ConstantInt::get(const_l->getValue()*const_r->getValue(),module.get());
            break;
        case::ast::BinOp::SLASH:
            tmp_val=ConstantInt::get(const_l->getValue()/const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MOD:
            tmp_val=ConstantInt::get(const_l->getValue()%const_r->getValue(),module.get());
        case::ast::BinOp::LT:
            tmp_val=ConstantInt::get(const_l->getValue()<const_r->getValue(),module.get());
            break;
        case::ast::BinOp::LE:
            tmp_val=ConstantInt::get(const_l->getValue()<=const_r->getValue(),module.get());
            break;
        case::ast::BinOp::GT:
            tmp_val=ConstantInt::get(const_l->getValue()>const_r->getValue(),module.get());
            break;
        case::ast::BinOp::GE:
            tmp_val=ConstantInt::get(const_l->getValue()>=const_r->getValue(),module.get());
            break;
        case::ast::BinOp::EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),module.get());
            break;
        case::ast::BinOp::NOT_EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),module.get());
            break;
        default:
            exit(151);
        }
        const_l=nullptr;const_r=nullptr;
    }else if(ConstantFP* const_l=dynamic_cast<ConstantFP*>(lhs),*const_r =dynamic_cast<ConstantFP*>(rhs);
    const_l!=nullptr&&const_r!=nullptr){
        switch(node_op){
        case::ast::BinOp::PlUS:
            tmp_val=ConstantFP::get(const_l->getValue()+const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MINUS:
            tmp_val=ConstantFP::get(const_l->getValue()-const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MULTI:
            tmp_val=ConstantFP::get(const_l->getValue()*const_r->getValue(),module.get());
            break;
        case::ast::BinOp::SLASH:
            tmp_val=ConstantFP::get(const_l->getValue()/const_r->getValue(),module.get());
            break;
        case::ast::BinOp::LT:
            tmp_val=ConstantInt::get(const_l->getValue()<const_r->getValue(),module.get());
            break;
        case::ast::BinOp::LE:
            tmp_val=ConstantInt::get(const_l->getValue()<=const_r->getValue(),module.get());
            break;
        case::ast::BinOp::GT:
            tmp_val=ConstantInt::get(const_l->getValue()>const_r->getValue(),module.get());
            break;
        case::ast::BinOp::GE:
            tmp_val=ConstantInt::get(const_l->getValue()>=const_r->getValue(),module.get());
            break;
        case::ast::BinOp::EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),module.get());
            break;
        case::ast::BinOp::NOT_EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),module.get());
        default:
            exit(151);
        }
        const_l=nullptr;const_r=nullptr;
    }else if(ConstantFP* const_l=dynamic_cast<ConstantFP*>(lhs);
    const_l!=nullptr&&dynamic_cast<ConstantInt*>(rhs)){
        auto const_r=dynamic_cast<ConstantInt*>(rhs);
        switch(node_op){
        case::ast::BinOp::PlUS:
            tmp_val=ConstantFP::get(const_l->getValue()+const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MINUS:
            tmp_val=ConstantFP::get(const_l->getValue()-const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MULTI:
            tmp_val=ConstantFP::get(const_l->getValue()*const_r->getValue(),module.get());
            break;
        case::ast::BinOp::SLASH:
            tmp_val=ConstantFP::get(const_l->getValue()/const_r->getValue(),module.get());
            break;
        case::ast::BinOp::LT:
            tmp_val=ConstantInt::get(const_l->getValue()<const_r->getValue(),module.get());
            break;
        case::ast::BinOp::LE:
            tmp_val=ConstantInt::get(const_l->getValue()<=const_r->getValue(),module.get());
            break;
        case::ast::BinOp::GT:
            tmp_val=ConstantInt::get(const_l->getValue()>const_r->getValue(),module.get());
            break;
        case::ast::BinOp::GE:
            tmp_val=ConstantInt::get(const_l->getValue()>=const_r->getValue(),module.get());
            break;
        case::ast::BinOp::EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),module.get());
            break;
        case::ast::BinOp::NOT_EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),module.get());
        default:
            exit(151);
        }
        const_l=nullptr;const_r=nullptr;
    }else if(ConstantInt* const_l=dynamic_cast<ConstantInt*>(lhs);
    const_l!=nullptr&&dynamic_cast<ConstantFP*>(rhs)){
        auto const_r=dynamic_cast<ConstantFP*>(rhs);
        switch(node_op){
        case::ast::BinOp::PlUS:
            tmp_val=ConstantFP::get(const_l->getValue()+const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MINUS:
            tmp_val=ConstantFP::get(const_l->getValue()-const_r->getValue(),module.get());
            break;
        case::ast::BinOp::MULTI:
            tmp_val=ConstantFP::get(const_l->getValue()*const_r->getValue(),module.get());
            break;
        case::ast::BinOp::SLASH:
            tmp_val=ConstantFP::get(const_l->getValue()/const_r->getValue(),module.get());
            break;
        case::ast::BinOp::LT:
            tmp_val=ConstantInt::get(const_l->getValue()<const_r->getValue(),module.get());
            break;
        case::ast::BinOp::LE:
            tmp_val=ConstantInt::get(const_l->getValue()<=const_r->getValue(),module.get());
            break;
        case::ast::BinOp::GT:
            tmp_val=ConstantInt::get(const_l->getValue()>const_r->getValue(),module.get());
            break;
        case::ast::BinOp::GE:
            tmp_val=ConstantInt::get(const_l->getValue()>=const_r->getValue(),module.get());
            break;
        case::ast::BinOp::EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),module.get());
            break;
        case::ast::BinOp::NOT_EQ:
            tmp_val=ConstantInt::get(const_l->getValue()==const_r->getValue(),module.get());
        default:
            exit(151);
        }
        const_l=nullptr;const_r=nullptr;
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
            exit(153);
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
        case::ast::BinOp::MOD:
            if(is_float)
                exit(154);
            else
                tmp_val=BinaryInst::createSRem(l_instr,r_instr,cur_block_of_cur_fun);
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
                    exit(154);
                    }
                };
            if(is_float)
                tmp_val =  FCmpInst::createFCmp(binop_to_cmpop(),lhs,rhs,cur_block_of_cur_fun,module.get());
            else    
                tmp_val  =  CmpInst::createCmp(binop_to_cmpop(),lhs,rhs,cur_block_of_cur_fun,module.get());
        }
    }
}
void IRGen::visit(ast::LvalExpr &node){
    auto var = scope.find(node.name);
    Type *type;
    if(!var->getType()->getPointerElementType())
        if(var->getType()->isFloatType()){
            type=FLOAT_T;
        }else{
            type=INT32_T;
        }
    else{
        if(var->getType()->getPointerElementType()->isPointerType()){
            if(var->getType()->getPointerElementType()->getPointerElementType()->isFloatType()){
                type=FLOATPTR_T;
            }else{
                type=INT32PTR_T;
            }
        }else{
            if(var->getType()->getPointerElementType()->isFloatType()){
                type=FLOAT_T;
            }else{
                type=INT32_T;
            }
        }
    }
    bool should_return_lvalue = require_lvalue;
    require_lvalue = false;
    if(node.index_num.empty()) {
        if(should_return_lvalue) {
            if(var->getType()->getPointerElementType()->isArrayType()) {
                tmp_val=GetElementPtrInst::createGep(var, {CONST_INT(0), CONST_INT(0)},cur_block_of_cur_fun);
            } else if(var->getType()->getPointerElementType()->isPointerType()) {
                tmp_val = LoadInst::createLoad(type,var,cur_block_of_cur_fun);
            } else {
                tmp_val = var;
            }
        } else {
            if(var->getType() == FLOAT_T) 
                tmp_val = dynamic_cast<ConstantFP*>(var);
            else 
                {tmp_val = dynamic_cast<ConstantInt*>(var);

            }if(tmp_val==nullptr){
                tmp_val = LoadInst::createLoad(type,var,cur_block_of_cur_fun);  
            }
        }
    } else {
        auto size = scope.findSize(node.name);
        std::vector<Value*> var_indexs;
        Value *var_index = nullptr;
        int index_const = 0;
        auto const_array = scope.findConst(node.name);
        {
            /*
            int x[30][20][10] ;
            {6000,200,10,1}
            */
            std::vector<Value*> indexs;
            for(auto & index:node.index_num){
                index->accept(*this);
                indexs.push_back(tmp_val);
            }
            for(int i=1;i<size.size();i++){
                Value* one_index;
                if(auto const_val=dynamic_cast<ConstantInt*>(indexs[i-1])){
                    one_index=CONST_INT(const_val->getValue()*size[i]);    
                }else{
                    one_index=BinaryInst::createMul(indexs[i-1],CONST_INT(size[i]),cur_block_of_cur_fun);
                }
                if(var_index==nullptr)
                    var_index=one_index;
                else {
                    if(dynamic_cast<ConstantInt*>(one_index)&&dynamic_cast<ConstantInt*>(var_index))
                        var_index=CONST_INT(((ConstantInt*)one_index)->getValue()+((ConstantInt*)var_index)->getValue());
                    else
                        var_index=BinaryInst::createAdd(var_index, one_index,cur_block_of_cur_fun,module.get());
                }
            }
            if(const_array!=nullptr&&dynamic_cast<ConstantInt*>(var_index)){
                tmp_val=const_array->getElementValue(dynamic_cast<ConstantInt*>(var_index)->getValue());
                return ;
            }
            if(var->getType()->getPointerElementType()->isPointerType()) {
                auto tmp_load = LoadInst::createLoad(var->getType()->getPointerElementType(),var,cur_block_of_cur_fun);
                tmp_val = GetElementPtrInst::createGep(tmp_load, {var_index},cur_block_of_cur_fun);
            } else {
                tmp_val =  GetElementPtrInst::createGep(var, {CONST_INT(0),var_index},cur_block_of_cur_fun);
            }
            if(!should_return_lvalue)
                tmp_val = LoadInst::createLoad(static_cast<PointerType *>(tmp_val->getType())->getElementType(),tmp_val,cur_block_of_cur_fun);
        }
    }
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
    // tmp_val=CmpInst::createCmp(CmpOp::NE,CONST_INT(0),tmp_val,cur_block_of_cur_fun);
    IF_WHILE_Cond_Stack.pop_back();

    //生成比较指令
    Value* inst_cmp;
    if(tmp_val->getType()==INT1_T)  inst_cmp = tmp_val;
    else if(tmp_val->getType()==INT32_T){
        auto tmp_val_const = dynamic_cast<ConstantInt*>(tmp_val);
        if(tmp_val_const){
            inst_cmp = ConstantInt::get(static_cast<bool>(tmp_val_const->getValue()!=0));
        }
        else{
            inst_cmp = CmpInst::createCmp(NE, tmp_val, ConstantInt::get(0), cur_block_of_cur_fun);
        }
    }
    else if(tmp_val->getType()==FLOAT_T){
        auto tmp_val_const = dynamic_cast<ConstantFP*>(tmp_val);
        if(tmp_val_const){
            inst_cmp = ConstantInt::get(static_cast<bool>(tmp_val_const->getValue()!=0));
        }
        else{
            inst_cmp = FCmpInst::createFCmp(NE, tmp_val, ConstantFP::get(0), cur_block_of_cur_fun);
        }
    }

    if(node.else_stmt==nullptr) BranchInst::createCondBr(inst_cmp, true_bb, next_bb, cur_block_of_cur_fun);
    else    BranchInst::createCondBr(inst_cmp, true_bb, false_bb, cur_block_of_cur_fun);

    cur_basic_block_list.pop_back();
    cur_block_of_cur_fun = true_bb;
    cur_basic_block_list.push_back(true_bb);
  
    if(dynamic_cast<ast::BlockStmt*>(node.then_stmt.get()))  
        node.then_stmt->accept(*this);
    else{
        scope.enter();
        node.then_stmt->accept(*this);
        scope.exit();
    }
   
    if(cur_block_of_cur_fun->getTerminator()==nullptr)  BranchInst::createBr(next_bb, cur_block_of_cur_fun);
    cur_basic_block_list.pop_back();
    if(node.else_stmt==nullptr) false_bb->eraseFromParent();
    else{
         cur_block_of_cur_fun=false_bb;
        cur_basic_block_list.push_back(false_bb);
        if(dynamic_cast<ast::BlockStmt*>(node.else_stmt.get())) 
            node.else_stmt->accept(*this);
        else{
            scope.enter();
            node.else_stmt->accept(*this);
            scope.exit();
        }
        if(cur_block_of_cur_fun->getTerminator()==nullptr)  BranchInst::createBr(next_bb, cur_block_of_cur_fun);
        cur_basic_block_list.pop_back();
    }
   cur_block_of_cur_fun=next_bb;
    cur_basic_block_list.push_back(next_bb);
    if(next_bb->getPreBasicBlocks().size()==0){
        cur_block_of_cur_fun = true_bb;
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
    cur_block_of_cur_fun = pred_bb;
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
    cur_block_of_cur_fun = iter_bb;
    cur_basic_block_list.push_back(iter_bb);
    if(dynamic_cast<ast::BlockStmt*>(node.loop_stmt.get())) node.loop_stmt->accept(*this);
    else{
        scope.enter();
        node.loop_stmt->accept(*this);
        scope.exit();
    }

    if(cur_block_of_cur_fun->getTerminator()==nullptr)  BranchInst::createBr(pred_bb, cur_block_of_cur_fun);
    cur_basic_block_list.pop_back();
    cur_block_of_cur_fun = next_bb;
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
                    tmp_val = ConstantInt::get( int(tmp_val_const_float->getValue()));
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
