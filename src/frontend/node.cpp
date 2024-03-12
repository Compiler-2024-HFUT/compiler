#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include <memory>
#include <string>
using namespace ast;
SyntaxNode::SyntaxNode(Pos pos):pos(pos){}
CompunitNode::CompunitNode():SyntaxNode({0,0}){}
Statement::Statement(Pos pos):SyntaxNode(pos){}
ContinueStmt::ContinueStmt(Pos pos):Statement(pos){}
BreakStmt::BreakStmt(Pos pos):Statement(pos){}
ExprStmt::ExprStmt(Pos pos):Statement(pos){}
ExprStmt::ExprStmt(Pos pos,unique_ptr<ExprNode> expr):Statement(pos),expr(std::move(expr)){}
EmptyStmt::EmptyStmt(Pos pos):Statement(pos){}
ValDeclStmt::ValDeclStmt(Pos pos):Statement(pos){}
ValDeclStmt::ValDeclStmt(Pos pos,ValType type):Statement(pos),all_type(type){}
ConstDeclStmt::ConstDeclStmt(Pos pos):ValDeclStmt(pos){}
ConstDeclStmt::ConstDeclStmt(Pos pos,ValType type):ValDeclStmt(pos,type){}
FuncFParam::FuncFParam(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
FuncDef::FuncDef(string name ,Pos pos,ValType type):FuncStmt(name,pos,type){}
FuncStmt::FuncStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
DefStmt::DefStmt(string name ,Pos pos,ValType type):Statement(pos),name(name),type(type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
ValDefStmt::ValDefStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):DefStmt(name,pos,type),init_expr(std::move(expr)){}
ConstDefStmt::ConstDefStmt(string name ,Pos pos,ValType type):ValDefStmt(name,pos,type){}
ConstDefStmt::ConstDefStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):ValDefStmt(name,pos,type,std::move(expr)){}
ArrDefStmt::ArrDefStmt(string name ,Pos pos,ValType type):DefStmt(name,pos,type){}
ConstArrDefStmt::ConstArrDefStmt(string name ,Pos pos,ValType type):ArrDefStmt(name,pos,type){}
LvalExpr::LvalExpr(Pos pos,string name ):ExprNode(pos),name(name){}
CallExpr::CallExpr(Pos pos):ExprNode(pos){}
//LvalStmt::LvalStmt(string name ,Pos pos,ValType type,unique_ptr<ExprNode> expr):DefStmt(name,pos,type),expr(std::move(expr)){}
RetStmt::RetStmt(Pos pos):Statement(pos){}
WhileStmt::WhileStmt(Pos pos):Statement(pos){}
BlockStmt::BlockStmt(Pos pos):Statement(pos){}
IfStmt::IfStmt(Pos Pos):Statement(pos){}
AssignStmt::AssignStmt(Pos pos,unique_ptr<ast::ExprNode> lval,unique_ptr<ast::ExprNode> expr):Statement(pos),l_val(std::move(lval)),expr(std::move(expr)){}
ExprNode::ExprNode(Pos pos):SyntaxNode(pos){}
IntConst::IntConst(Pos pos,valUnion val):Literal(pos,val){};
InitializerExpr::InitializerExpr(Pos pos):ExprNode(pos){};
FloatConst::FloatConst(Pos pos,valUnion val):Literal(pos,val){};
Literal::Literal(Pos pos,valUnion val):ExprNode(pos),Value(val){};
UnaryExpr::UnaryExpr(Pos pos):ExprNode(pos){};
InfixExpr::InfixExpr(Pos pos,unique_ptr<ExprNode> lhs):ExprNode(pos),lhs(std::move(lhs)){}
AssignExpr::AssignExpr(Pos pos,unique_ptr<ExprNode> lhs):InfixExpr(pos,std::move(lhs)){}
RelopExpr::RelopExpr(Pos pos,unique_ptr<ExprNode> lhs):InfixExpr(pos,std::move(lhs)){}
EqExpr::EqExpr(Pos pos,unique_ptr<ExprNode> lhs):InfixExpr(pos,std::move(lhs)){}
ORExp::ORExp(Pos pos,unique_ptr<ExprNode> lhs):InfixExpr(pos,std::move(lhs)){}
AndExp::AndExp(Pos pos,unique_ptr<ExprNode> lhs):InfixExpr(pos,std::move(lhs)){}
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
int IntConst::getType(){
    return (int)ast::ExprType::INT_LITERAL;
}
int InitializerExpr::getType(){
    return (int)ast::ExprType::INITIALIZER;
}
int FloatConst::getType(){
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
int UnaryExpr::getType(){
    return (int)ast::ExprType::PREFIX;
}
// int InfixExpr::getType(){
//     return (int)ast::ExprType::INFIX;
// }
int RelopExpr::getType(){
    return (int)ast::ExprType::REL_OP_EXPR;
}
int ORExp::getType(){
    return (int)ast::ExprType::OR_EXPR;
}
int AndExp::getType(){
    return (int)ast::ExprType::AND_EXPR;
}
int EqExpr::getType(){
    return (int)ast::ExprType::Eq_EXPR;
}
int AssignExpr::getType(){
    return (int)ast::ExprType::ASSIGN_EXPR;
}
int BinopExpr::getType(){
    return (int)ast::ExprType::BIN_OP_EXPR;
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
string typeToStr(ValType type){
    string ret{};
    if(IS_CONST(type.t)){
        ret+="const ";
    }
    if(IS_INT(type.t)){
        ret+="int ";
    }else if(IS_FLOAT(type.t)){
        ret+="float ";
    }else{
        ret+="void ";
    }
    return ret;
}
void LevelPrint(int cur_level,string name,bool is_terminal){
    for(int i = 0 ; i < cur_level; ++i) cout << "|  ";
    cout << ">--" << (is_terminal ? "*" : "+") << name;
    cout << std::endl;
}
void CompunitNode::print(int level){
    for(auto&i:global_defs){
        i->print(level);
    }
}
void ConstDefStmt::print(int level){
    string type{ typeToStr(this->type)};
    // if(IS_INT(this->val_type.t)){
    //     type+="int";
    // }else{
    //     type+="float";
    // }
    LevelPrint(level, type, true);
    LevelPrint(level, name, true);
    if(init_expr){
        LevelPrint(level, "=", true);
        init_expr->print(level);
    }
}
void ValDefStmt::print(int level){
    string type{ typeToStr(this->type)};
    LevelPrint(level, type, true);
    LevelPrint(level, name, true);
    level++;
    if(init_expr){
        LevelPrint(level-1, "=", true);
        init_expr->print(level);
    }
    level--;
}
void ArrDefStmt::print(int level){
    string type{ typeToStr(this->type)};
    LevelPrint(level, "ArrDef", false);
    LevelPrint(level, name, true);
    if(initializers==nullptr){

    }else{
        LevelPrint(level, "=", true);
        initializers->print(level);
    }
}
void ConstArrDefStmt::print(int level){
    string type{ typeToStr(this->type)};
    //cout<<"def a vlaue"<<val_type<<" name is "<<name;
    LevelPrint(level, type, false);
    LevelPrint(level, name, true);
    if(initializers==nullptr){

    }else{
        LevelPrint(level, "=", true);
        initializers->print(level);
    }
}
void ValDeclStmt::print(int level){
    for(auto&i:var_def_list){
        i->print(level);
    }
}
void ConstDeclStmt::print(int level){
    for(auto&i:var_def_list){
        i->print(level);
    }
}
// void FuncStmt::print(int level){

// }
void FuncFParam::print(int level){
    LevelPrint(level, typeToStr(this->type)+name, false);
    for(auto&i:index_num){
        if(i!=nullptr){
            i->print(level);
        }else{
            LevelPrint(level, "empty index", false);
        }
    }
    LevelPrint(level, ",", false);
}
void FuncDef::print(int level){
    LevelPrint(level, "FuncDef", false);
    LevelPrint(level, "return type "+typeToStr(type), false);
    LevelPrint(level, this->name, true);
    level++;
    LevelPrint(level, "(", true);
    for(auto &i :func_f_params){
        LevelPrint(level, "FuncParam", false);
        i->print(level);
    }
    LevelPrint(level, ")", true);
    if(body){
        LevelPrint(level, "FuncBody", false);
        body->print(level);
        level--;
    }else{
        exit(199);
    }
}
void RetStmt::print(int level){
    LevelPrint(level, "return", false);
    if(expr){
        expr->print(level);
    }
}
void IfStmt::print(int level){
    LevelPrint(level, "if", true);
    LevelPrint(level, "(", true);
    pred->print(level);
    LevelPrint(level, ")", true);
    level++;
    LevelPrint(level, "ThenBody", true);
    then_stmt->print(level);
    level--;
    if(else_stmt!=nullptr){
        LevelPrint(level, "else", false);
        level++;
        LevelPrint(level, "ElseBody", true);
        else_stmt->print(level);
        level--;
    }
}
void WhileStmt::print(int level){
    LevelPrint(level, "While", false);
    LevelPrint(level, "(", true);
    this->pred->print(level);
    LevelPrint(level, ")", true);
    LevelPrint(level, "WhileBody", false);
    ++level;
    this->loop_stmt->print(level);
    --level;
}
void BlockStmt::print(int level){
    for(auto &i:this->block_items){
        i->print(level);
    }
}
void ExprStmt::print(int level){
    expr->print(level);
}
void AssignStmt::print(int level){
    l_val->print(level);
    LevelPrint(level, "=", true);
    expr->print(level);
}
void IntConst::print(int level){
    LevelPrint(level, std::to_string(this->Value.i), true);
}
void InitializerExpr::print(int level){
    LevelPrint(level, "initializer size is"+std::to_string(initializers.size()), false);
    level++;
    if(!this->initializers.empty())
        for(auto&i:this->initializers){
            i->print(level);
        }
    level--;
}
void FloatConst::print(int level){
LevelPrint(level, std::to_string(this->Value.f), true);
}
void CallExpr::print(int level){
    this->call_name->print(level);
    LevelPrint(level, "(", true);
    level++;
    if(!func_r_params.empty())
        for(auto&i:this->func_r_params){
            i->print(level);
        }
    level--;
    LevelPrint(level, ")", true);
}
void LvalExpr::print(int level){
    LevelPrint(level, name, true);
    for(auto&i:this->index_num){
        LevelPrint(level, "[", true);
        if(i!=nullptr)
            i->print(level);
        else
            exit(114);
            //LevelPrint(level, "null", true);
        LevelPrint(level, "]", true);
    }
}
void UnaryExpr::print(int level){
    LevelPrint(level, Operat, true);
    rhs->print();
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
    exit(127);
    LevelPrint(level, Operat, true);
    level++;
    lhs->print(level);
    rhs->print(level);
    level--;
}
void RelopExpr::print(int level){
    LevelPrint(level, Operat, true);
    level++;
    lhs->print(level);
    rhs->print(level);
    level--;
}
void AndExp::print(int level){
    LevelPrint(level, Operat, true);
    level++;
    lhs->print(level);
    rhs->print(level);
    level--;
}
void ORExp::print(int level){
    LevelPrint(level, Operat, true);
    level++;
    lhs->print(level);
    rhs->print(level);
    level--;
}
void EqExpr::print(int level){
    LevelPrint(level, Operat, true);
    level++;
    lhs->print(level);
    rhs->print(level);
    level--;
}
void BinopExpr::print(int level){
    LevelPrint(level, Operat, true);
    level++;
    lhs->print(level);
    rhs->print(level);
    level--;
}

void EmptyStmt::print(int level){
    LevelPrint(level, "emptyStmt", true);
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
void CompunitNode::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void FuncFParam::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void FuncDef::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ValDeclStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ValDefStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ArrDefStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ConstDeclStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ConstDefStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ConstArrDefStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void IntConst::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void InitializerExpr::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void FloatConst::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void UnaryExpr::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
// void InfixExpr::accept(ASTVisitor &visitor) {
//     visitor.visit(*this);
// }
void AssignExpr::accept(ASTVisitor &visitor) {
    exit(127);
    // visitor.visit(*this);
}
void RelopExpr::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void EqExpr::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ORExp::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void AndExp::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void BinopExpr::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void LvalExpr::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void CallExpr::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ExprStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void AssignStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void BlockStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void RetStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void IfStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void WhileStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void ContinueStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void BreakStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
void EmptyStmt::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
CompunitNode::~CompunitNode(){
    for(auto&i:this->global_defs){
        i.reset();
    }
}
FuncDef::~FuncDef(){
    body.reset();
    func_f_params.clear();
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
    then_stmt.reset();
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