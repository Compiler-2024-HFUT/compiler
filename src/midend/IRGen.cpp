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