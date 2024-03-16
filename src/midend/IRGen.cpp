#include "../../include/midend/IRGen.hpp"

std::unique_ptr<Module> module_sole= std::unique_ptr<Module>(new Module("sysY code"));

void IRGen::visit(ast::CompunitNode &node) {}
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