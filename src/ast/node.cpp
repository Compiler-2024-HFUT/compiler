#include "node.hpp"
#include <string>
using namespace ast;
SyntaxNode::SyntaxNode(Pos pos):pos(pos){}
Statement::Statement(Pos pos):SyntaxNode(pos){}
ContinueStmt::ContinueStmt(Pos pos):Statement(pos){}
BreakStmt::BreakStmt(Pos pos):Statement(pos){}
ExprStmt::ExprStmt(Pos pos):Statement(pos){}
EmptyStmt::EmptyStmt(Pos pos):Statement(pos){}
ValDeclStmt::ValDeclStmt(Pos pos):Statement(pos){}
FuncDef::FuncDef(string name ,Pos pos,ValType type):FuncStmt(name,pos,type){}
FuncStmt::FuncStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
DefStmt::DefStmt(string name ,Pos pos,ValType type):Statement(pos),name(name),val_type(type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):DefStmt(name,pos,type),init_expr(std::move(expr)){}
LvalExpr::LvalExpr(Pos pos,string name ):ExprNode(pos),name(name){}
CallExpr::CallExpr(Pos pos,string name ):ExprNode(pos),name(name){}
//LvalStmt::LvalStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):DefStmt(name,pos,type),expr(std::move(expr)){}
RetStmt::RetStmt(Pos pos):Statement(pos){}
WhileStmt::WhileStmt(Pos pos):Statement(pos){}
BlockStmt::BlockStmt(Pos pos):Statement(pos){}
IfStmt::IfStmt(Pos Pos):Statement(pos){}
ExprNode::ExprNode(Pos pos):SyntaxNode(pos){}
IntLiteral::IntLiteral(Pos pos):ExprNode(pos){};
PrefixExpr::PrefixExpr(Pos pos):ExprNode(pos){};
SuffixExpr::SuffixExpr(Pos pos):ExprNode(pos){};
InfixExpr::InfixExpr(Pos pos,unique_ptr<ExprNode> lhs):ExprNode(pos),lhs(std::move(lhs)){}
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
    return (int)ast::StmtType::IF_STMT;
}
int WhileStmt::getType(){
    return (int)ast::StmtType::WHILE_STMT;
}
int BlockStmt::getType(){
    return (int)ast::StmtType::BLOCK_STMT;
}
int IntLiteral::getType(){
    return (int)ast::StmtType::INT_LITERAL;
}
int CallExpr::getType(){
    exit(114);
    //return (int)ast::StmtType::INT_LITERAL;
}
int LvalExpr::getType(){
    exit(114);
    //return (int)ast::StmtType::INT_LITERAL;
}
int ExprStmt::getType(){
    exit(114);
    //return (int)ast::StmtType::INT_LITERAL;
}
int PrefixExpr::getType(){
    return (int)ast::StmtType::PREFIX;
}
int InfixExpr::getType(){
    return (int)ast::StmtType::INFIX;
}
int SuffixExpr::getType(){
    return (int)ast::StmtType::SUFFIX;
}
int EmptyStmt::getType(){
    return (int)ast::StmtType::NULL_STMT;
}
int ContinueStmt::getType(){
    return (int)ast::StmtType::CONTINUE_STMT;
}
int BreakStmt::getType(){
    return (int)ast::StmtType::BREAK_STMT;
}
void ValDefStmt::print(int level){
    //cout<<"def a vlaue"<<val_type<<" name is "<<name;
    string s(level,'|');
    cout<<s<<"val\n";
    cout<<s<<this->name<<endl;
    if(init_expr){
        init_expr->print(level);
    }
}
void ValDeclStmt::print(int level){
    for(auto&i:var_def_list){
        i->print(level);
    }
}
void FuncStmt::print(int level){

}
void FuncDef::print(int level){
    string s(level,'|');
    cout<<s<<"func "<<val_type.i<<"name is"<<name<<endl;
    for(auto &i :argv){
        cout<<s<<"def"<<i.first.i<<'\n'<<s<<"name "<<i.second<<endl;
    }
    if(body){
        body->print(level+1);
    
    }else{
        exit(199);
    }
}
void RetStmt::print(int level){
    string s(level,'|');
    cout<<s<<"return"<<endl;
    if(expr){
        expr->print(level);
    }
}
void IfStmt::print(int level){
    string s(level,'|');
    cout<<s<<"if"<<'\n';
    pred->print(level);
    level++;
    if_stmt->print(level);
    if(else_stmt!=nullptr){
        cout<<s<<"else"<<'\n';
        else_stmt->print(level);
        level--;
    }
}
void WhileStmt::print(int level){
    string s(level,'|');
    cout<<s<<"while"<<'\n';
    this->pred->print(level);
    level++;
    this->loop_stmt->print(level);
    level--;
}
void BlockStmt::print(int level){
    for(auto &i:this->block_items){
        i->print(level);
    }
}
void ExprStmt::print(int level){
    expr->print(level);
}
void IntLiteral::print(int level){
    string s(level,'|');
    cout<<s<<this->Value.i<<endl;
}
void CallExpr::print(int level){
    string s(level,'|');
    cout<<s<<"call func"<<endl;
    cout<<s<<this->name<<endl;
    cout<<s<<"arg"<<endl;
    level++;
    for(auto&i:this->arg){
        i->print(level);
    }
    level--;
}
void LvalExpr::print(int level){
    string s(level,'|');
    cout<<s<<name<<endl;
}
void PrefixExpr::print(int level){
    level++;
    string s(level,'|');
    cout<<s<<Operat<<endl;
    rhs->print(level);
    level--;
}
void SuffixExpr::print(int level){
    level++;
    string s(level,'|');
    lhs->print(level);
    cout<<s<<Operat<<endl;
    if(rhs)
        rhs->print(level);
    level--;
}
void InfixExpr::print(int level){
    level++;
    string s(level,'|');
    lhs->print(level);
    cout<<s<<Operat<<endl;
    rhs->print(level);
    level--;
}

void EmptyStmt::print(int level){
    cout<<"NULL"<<endl;
}
void ContinueStmt::print(int level){
    string s(level,'|');
    cout<<s<<"continue"<<endl;
}
void BreakStmt::print(int level){
    string s(level,'|');
    cout<<s<<"break"<<endl;
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
void FuncDef::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void ValDeclStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void ValDefStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void IntLiteral::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void PrefixExpr::accept(Visitor &visitor) {

}
void InfixExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void SuffixExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void LvalExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void CallExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void ExprStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void BlockStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void RetStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void IfStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void WhileStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void ContinueStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void BreakStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void EmptyStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}


// bool FuncDef::isReDef(string tok_name){
//     bool re_def=false;
//     for(auto &i:body){
//         if(i->getType()==StmtType::VAL_DEF_STMT) {
//             auto *p=(DefStmt*)(i.get());
//             if(tok_name==p->name)
//                 re_def=true;
//         }
//     }
//     return re_def;
// }