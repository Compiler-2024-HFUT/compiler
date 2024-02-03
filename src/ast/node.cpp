#include "node.hpp"
#include <string>
using namespace ast;
SyntaxNode::SyntaxNode(Pos pos):pos(pos){}
Statement::Statement(Pos pos):SyntaxNode(pos){}
funcDef::funcDef(string name ,Pos pos,ValType type):funcStmt(name,pos,type){}
funcStmt::funcStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
DefStmt::DefStmt(string name ,Pos pos,ValType type):Statement(pos),name(name),val_type(type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
RetStmt::RetStmt(Pos pos):Statement(pos){}
ifStmt::ifStmt(Pos Pos):Statement(pos){}
int ValDefStmt::getType(){
    return (int)ast::StmtType::VAL_DEF_STMT;
}
int funcStmt::getType(){
    return (int)ast::StmtType::FUNSTMT;
}
int funcDef::getType(){
    return (int)ast::StmtType::FUNSTMT;
}
int RetStmt::getType(){
    return (int)ast::StmtType::RETURNSTMT;
}
int ifStmt::getType(){
    return (int)ast::StmtType::IFSTMT;
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
bool funcDef::isReDef(string tok_name){
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