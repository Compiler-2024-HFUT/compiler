#include "node.hpp"
#include "type.hpp"
#include <string>
using namespace ast;
SyntaxNode::SyntaxNode(Pos pos):pos(pos){}
CompunitNode::CompunitNode():SyntaxNode({0,0}){}
Statement::Statement(Pos pos):SyntaxNode(pos){}
ContinueStmt::ContinueStmt(Pos pos):Statement(pos){}
BreakStmt::BreakStmt(Pos pos):Statement(pos){}
ExprStmt::ExprStmt(Pos pos):Statement(pos){}
EmptyStmt::EmptyStmt(Pos pos):Statement(pos){}
ValDeclStmt::ValDeclStmt(Pos pos):Statement(pos){}
ValDeclStmt::ValDeclStmt(Pos pos,ValType type):Statement(pos),all_type(type){}
FuncDef::FuncDef(string name ,Pos pos,ValType type):FuncStmt(name,pos,type){}
FuncStmt::FuncStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
DefStmt::DefStmt(string name ,Pos pos,ValType type):Statement(pos),name(name),val_type(type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):DefStmt(name,pos,type),init_expr(std::move(expr)){}
ArrDefStmt::ArrDefStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
LvalExpr::LvalExpr(Pos pos,string name ):ExprNode(pos),name(name){}
CallExpr::CallExpr(Pos pos):ExprNode(pos){}
//LvalStmt::LvalStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):DefStmt(name,pos,type),expr(std::move(expr)){}
RetStmt::RetStmt(Pos pos):Statement(pos){}
WhileStmt::WhileStmt(Pos pos):Statement(pos){}
BlockStmt::BlockStmt(Pos pos):Statement(pos){}
IfStmt::IfStmt(Pos Pos):Statement(pos){}
ExprNode::ExprNode(Pos pos):SyntaxNode(pos){}
IntLiteral::IntLiteral(Pos pos,valUnion val):Literal(pos,val){};
InitializerExpr::InitializerExpr(Pos pos):ExprNode(pos){};
FloatLiteral::FloatLiteral(Pos pos,valUnion val):Literal(pos,val){};
Literal::Literal(Pos pos,valUnion val):ExprNode(pos),Value(val){};
PrefixExpr::PrefixExpr(Pos pos):ExprNode(pos){};
SuffixExpr::SuffixExpr(Pos pos):ExprNode(pos){};
ArrUse::ArrUse(Pos pos):ExprNode(pos){}
InfixExpr::InfixExpr(Pos pos,unique_ptr<ExprNode> lhs):ExprNode(pos),lhs(std::move(lhs)){}
AssignExpr::AssignExpr(Pos pos,unique_ptr<ExprNode> lhs):InfixExpr(pos,std::move(lhs)){}
RelopExpr::RelopExpr(Pos pos,unique_ptr<ExprNode> lhs):InfixExpr(pos,std::move(lhs)){}
BinopExpr::BinopExpr(Pos pos,unique_ptr<ExprNode> lhs):InfixExpr(pos,std::move(lhs)){}
// int CompunitNode::getType(){
//     return (int)ast::StmtType::ROOT;
// }
// int ValDefStmt::getType(){
//     return (int)ast::StmtType::VAL_DEF_STMT;
// }
// int ArrDefStmt::getType(){
//     return (int)ast::StmtType::VAL_DEF_STMT;
// }
// int ValDeclStmt::getType(){
//     return (int)ast::StmtType::VAL_DECL_STMT;
// }
// int FuncStmt::getType(){
//     return (int)ast::StmtType::FUNSTMT;
// }
// int FuncDef::getType(){
//     return (int)ast::StmtType::FUNSTMT;
// }
// int RetStmt::getType(){
//     return (int)ast::StmtType::RETURNSTMT;
// }
// int IfStmt::getType(){
//     return (int)ast::StmtType::IF_STMT;
// }
// int WhileStmt::getType(){
//     return (int)ast::StmtType::WHILE_STMT;
// }
// int BlockStmt::getType(){
//     return (int)ast::StmtType::BLOCK_STMT;
// }
int IntLiteral::getType(){
    return (int)ast::ExprType::INT_LITERAL;
}
int InitializerExpr::getType(){
    return (int)ast::ExprType::INITIALIZER;
}
int FloatLiteral::getType(){
    return (int)ast::ExprType::FLOAT_LITERAL;
}
int CallExpr::getType(){
    // exit(114);
    return (int)ast::ExprType::CALL_EXPR;
}
int LvalExpr::getType(){
    // exit(114);
    return (int)ast::ExprType::LVAL_EXPR;
}
int PrefixExpr::getType(){
    return (int)ast::ExprType::PREFIX;
}
// int InfixExpr::getType(){
//     return (int)ast::ExprType::INFIX;
// }
int RelopExpr::getType(){
    return (int)ast::ExprType::INFIX;
}
int AssignExpr::getType(){
    return (int)ast::ExprType::INFIX;
}
int BinopExpr::getType(){
    return (int)ast::ExprType::INFIX;
}
int SuffixExpr::getType(){
    return (int)ast::ExprType::SUFFIX;
}
int ArrUse::getType(){
    return (int)ast::ExprType::SUFFIX;
}
// int ExprStmt::getType(){
//     exit(114);
//     //return (int)ast::StmtType::INT_LITERAL;
// }
// int EmptyStmt::getType(){
//     return (int)ast::StmtType::NULL_STMT;
// }
// int ContinueStmt::getType(){
//     return (int)ast::StmtType::CONTINUE_STMT;
// }
// int BreakStmt::getType(){
//     return (int)ast::StmtType::BREAK_STMT;
// }
void CompunitNode::print(int level){
    for(auto&i:global_defs){
        i->print(level);
    }
}
void ValDefStmt::print(int level){
    //cout<<"def a vlaue"<<val_type<<" name is "<<name;
    string s(level,'|');
    cout<<s<<"def "<<this->name<<endl;
    //cout<<s<<<<endl;
    if(init_expr){
        init_expr->print(level);
    }
}
void ArrDefStmt::print(int level){
    //cout<<"def a vlaue"<<val_type<<" name is "<<name;
    string s(level,'|');
    cout<<s<<"arr\n";
    cout<<s<<this->name<<endl;
    // if(init_expr){
    //     init_expr->print(level);
    // }
    // for(auto&i:this->initializers->initializers){
    //     i->print(level);
    // }
    if(initializers==nullptr){

    }else
        initializers->print(level);

}
void ValDeclStmt::print(int level){
    for(auto&i:var_def_list){
        i->print(level);
    }
}
// void FuncStmt::print(int level){

// }
void FuncDef::print(int level){
    string s(level,'|');
    cout<<s<<"def func "<<""<<name<<endl;
    for(auto &i :argv){
        cout<<s<<"def"<<i.first.t<<'\n'<<s<<"name ";
        i.second->print();
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
void InitializerExpr::print(int level){
    string s(level,'|');
    if(!this->initializers.empty())
        for(auto&i:this->initializers){
            i->print(level);
        }
}
void FloatLiteral::print(int level){
    string s(level,'|');
    cout<<s<<this->Value.f<<endl;
}
void CallExpr::print(int level){
    string s(level,'|');
    cout<<s<<"call func"<<endl;
    //cout<<s<<this->name<<endl;
    this->call_name->print(level);
    // cout<<s<<"arg"<<endl;
    level++;
    if(!arg.empty())
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
void ArrUse::print(int level){
    level++;
    string s(level,'|');
    Lval_name->print(level);
    for(auto&i:this->index_num)
        if(i!=nullptr)
            i->print(level);
    level--;
}
// void InfixExpr::print(int level){
//     level++;
//     string s(level,'|');
//     lhs->print(level);
//     cout<<s<<Operat<<endl;
//     rhs->print(level);
//     level--;
// }
void AssignExpr::print(int level){
    level++;
    string s(level,'|');
    lhs->print(level);
    cout<<s<<Operat<<endl;
    rhs->print(level);
    level--;
}
void RelopExpr::print(int level){
    level++;
    string s(level,'|');
    lhs->print(level);
    cout<<s<<Operat<<endl;
    rhs->print(level);
    level--;
}
void BinopExpr::print(int level){
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
void CompunitNode::accept(Visitor &visitor) {
    visitor.visit(*this);
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
void ArrDefStmt::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void IntLiteral::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void InitializerExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void FloatLiteral::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void PrefixExpr::accept(Visitor &visitor) {

}
// void InfixExpr::accept(Visitor &visitor) {
//     visitor.visit(*this);
// }
void AssignExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void RelopExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void BinopExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void SuffixExpr::accept(Visitor &visitor) {
    visitor.visit(*this);
}
void ArrUse::accept(Visitor &visitor) {
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
CompunitNode::~CompunitNode(){
    for(auto&i:this->global_defs){
        i.reset();
    }
}
FuncDef::~FuncDef(){
    body.reset();
    argv.clear();
    // name.shrink_to_fit();
}
ValDeclStmt::~ValDeclStmt(){
    for(auto&i:var_def_list){
        i.reset();
    }
    // name.shrink_to_fit();
}
ValDefStmt::~ValDefStmt(){
    if(init_expr!=nullptr)
        init_expr.reset();
    // name.shrink_to_fit();
}
IfStmt::~IfStmt(){
    pred.reset();
    if_stmt.reset();
    // if(else_stmt!=nullptr)
        else_stmt.reset();
    // name.shrink_to_fit();
}
WhileStmt::~WhileStmt(){
    pred.reset();
    loop_stmt.reset();
}
RetStmt::~RetStmt(){
    expr.reset();
}
InfixExpr::~InfixExpr(){
    lhs.reset();
    rhs.reset();
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