#include "node.hpp"
#include <string>
using namespace ast;
SyntaxNode::SyntaxNode(Pos pos):pos(pos){}
Statement::Statement(Pos pos):SyntaxNode(pos){}
ValDeclStmt::ValDeclStmt(Pos pos):Statement(pos){}
FuncDef::FuncDef(string name ,Pos pos,ValType type):FuncStmt(name,pos,type){}
FuncStmt::FuncStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
DefStmt::DefStmt(string name ,Pos pos,ValType type):Statement(pos),name(name),val_type(type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):DefStmt(name,pos,type),init_expr(std::move(expr)){}
LvalStmt::LvalStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
LvalStmt::LvalStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):DefStmt(name,pos,type),expr(std::move(expr)){}
RetStmt::RetStmt(Pos pos):Statement(pos){}
WhileStmt::WhileStmt(Pos pos):Statement(pos){}
IfStmt::IfStmt(Pos Pos):Statement(pos){}
int ValDefStmt::getType(){
    return (int)ast::StmtType::VAL_DEF_STMT;
}
int ValDeclStmt::getType(){
    return (int)ast::StmtType::VAL_DECL_STMT;
}
int FuncStmt::getType(){
    return (int)ast::StmtType::FUNSTMT;
}
int FuncDef::getType(){
    return (int)ast::StmtType::FUNSTMT;
}
int RetStmt::getType(){
    return (int)ast::StmtType::RETURNSTMT;
}
int IfStmt::getType(){
    return (int)ast::StmtType::IFSTMT;
}
int WhileStmt::getType(){
    return (int)ast::StmtType::WHILE_STMT;
}
bool CompunitNode::isReDef(string tok_name){
    bool re_def=false;
    for(auto &i:global_defs){
        if(i->name==tok_name){
            re_def=true;
        }   
    }
    return re_def;
}
bool FuncDef::isReDef(string tok_name){
    bool re_def=false;
    for(auto &i:body){
        if(i->getType()==StmtType::VAL_DEF_STMT) {
            auto *p=(DefStmt*)(i.get());
            if(tok_name==p->name)
                re_def=true;
        }
    }
    return re_def;
}