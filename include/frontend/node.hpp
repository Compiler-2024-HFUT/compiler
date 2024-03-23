#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "lex.hpp"
#include "type.hpp"
using std::cout,std::string,std::vector;
using type::ValType;
namespace ast {
class ASTVisitor;
enum class BinOp{
    ILLEGAL,
    PlUS,
    MINUS,
    MULTI,
    SLASH,
    MOD,
    DOR,
    DAND,
    EQ,
    NOT_EQ,
    LT,
    LE,
    GT,
    GE,
};
enum class UnOp{
    PLUS='+',
    MINUS='-',
    NOT='!',
};
// enum ExprType{
//     FLOAT_LITERAL,
//     INT_LITERAL,
//     ASSIGN_EXPR,
//     BIN_OP_EXPR,
//     REL_OP_EXPR,
//     Eq_EXPR,
//     OR_EXPR,
//     AND_EXPR,
//     ARR_USE_EXPR,
//     PREFIX,
//     CALL_EXPR,
//     LVAL_EXPR,
//     INITIALIZER,
// };
enum StmtType{
    // NULL_STMT,
    // ROOT,
    // VAL_DEF_STMT,
    // VAL_DECL_STMT,
    // FUNSTMT,
    // CONSTSTMT,
    // RETURNSTMT,
    // IF_STMT,
    // WHILE_STMT,
    // BLOCK_STMT,
    // FLOAT_LITERAL,
    // INT_LITERAL,
    // INFIX,
    // PREFIX,
    // SUFFIX,
    // CONTINUE_STMT,
    // BREAK_STMT,
};


struct SyntaxNode {
    Pos pos;
    SyntaxNode(Pos);
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor)=0 ;
};
struct ExprNode: SyntaxNode {
//   public:
//     int line;
//     // 用于访问者模式
    //std::unique_ptr<Token> tok;//记录位置
    ExprNode(Pos pos);
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor) =0;
};
struct UnaryExpr:public ExprNode{
    UnOp operat;//type
    unique_ptr<ExprNode> rhs;
    UnaryExpr(Pos pos);
    // virtual int getType()override;
    virtual void print(int level=0)override;
    virtual void accept(ASTVisitor &visitor) override final;
};
// struct ArrUse:public ExprNode{
//     unique_ptr<ExprNode> Lval_name;
//     vector<unique_ptr<ExprNode>> index_num;
//     ArrUse(Pos pos);
//     virtual int getType()override;
//     virtual void print(int level=0)override;
//     virtual void accept(ASTVisitor &visitor)  final;
// };
struct InfixExpr:public ExprNode{
    // string Operat;
    BinOp operat;
    unique_ptr<ExprNode> rhs;
    unique_ptr<ExprNode> lhs;
    InfixExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    ~InfixExpr();
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor)=0;

};

struct AssignExpr:public InfixExpr{
    AssignExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct RelopExpr:public InfixExpr{
    RelopExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct EqExpr:public InfixExpr{
    EqExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct AndExp:public InfixExpr{
    AndExp(Pos pos ,unique_ptr<ExprNode> lhs);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct ORExp:public InfixExpr{
    ORExp(Pos pos ,unique_ptr<ExprNode> lhs);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct BinopExpr:public InfixExpr{
    BinopExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
///////中缀表达式/////
union valUnion{
    float f;
    int   i;
};
struct Literal:public ExprNode{
    valUnion Value;
    Literal(Pos pos,valUnion);
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor)  =0;
};
struct IntConst:public Literal{
    IntConst(Pos pos,valUnion);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct InitializerExpr:public ExprNode{
    InitializerExpr(Pos pos);
    vector<unique_ptr<ExprNode>> initializers;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct FloatConst:public Literal{
    FloatConst(Pos pos,valUnion);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct CallExpr:public ExprNode{
    // string name;//type
    // unique_ptr<ExprNode> call_name;
    string call_name;
    vector<unique_ptr<ast::ExprNode>> func_r_params;
    CallExpr(Pos pos);
    CallExpr(Pos pos,string name);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};

struct LvalExpr:public ExprNode{
    string name;
    // unique_ptr<ExprNode> expr;
    LvalExpr(Pos pos,string name);
    vector<unique_ptr<ExprNode>> index_num;
    // LvalExpr(string name ,Pos pos,ValType,unique_ptr<ExprNode>);
    // virtual void print();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct Statement:public SyntaxNode{
    Statement(Pos pos );
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
};
struct ExprStmt:public Statement{
    unique_ptr<ast::ExprNode> expr;
    ExprStmt(Pos pos );
    ExprStmt(Pos pos,unique_ptr<ExprNode> );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};
struct AssignStmt:public Statement{
    unique_ptr<ast::ExprNode> l_val;
    unique_ptr<ast::ExprNode> expr;
    AssignStmt(Pos pos,unique_ptr<ast::ExprNode> lval,unique_ptr<ast::ExprNode> expr);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};
struct BreakStmt:public Statement{
    BreakStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct ContinueStmt:public Statement{
    ContinueStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct BlockStmt :public  Statement{
    vector<unique_ptr<Statement>> block_items;
    BlockStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
//抽象类
struct DefStmt:public Statement{
    string name;
    ValType type;//变量类型
    DefStmt (string name ,Pos pos,ValType );
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor)=0;
};

struct CompunitNode : public SyntaxNode
{
    vector<std::unique_ptr<DefStmt>> global_defs;
    bool isReDef(string s);
    CompunitNode();
    ~CompunitNode();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};
/*语法树*/
struct stynaxTree{
    unique_ptr<CompunitNode >root;
};


/*函数声明*/
struct FuncStmt :public  DefStmt
{   
    //在父类里
    // string name;
    // ValType val_type;//变量类型    
    FuncStmt(string name ,Pos pos,ValType );
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor) =0;
};
struct FuncFParam:public DefStmt{
    //在父类里
    // string name;
    // ValType val_type;//变量类型
    //nullptr if empty in [ ] 如果[]中为空则为nullptr
    vector<unique_ptr<ExprNode>> index_num;
    FuncFParam(string,Pos,ValType);
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
/*函数定义*/
struct FuncDef :public  FuncStmt
{   
    //在父类里
    // string name;
    // ValType val_type;//变量类型
    unique_ptr<BlockStmt> body;
    std::vector<unique_ptr<FuncFParam>>  func_f_params;
    // FuncDef(string name ,Pos pos);
    FuncDef(string name ,Pos pos,ValType );
    ~FuncDef();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

    // bool isReDef(string tok_name);
};
struct ValDeclStmt :public  Statement
{   
    ValType all_type;
    vector<unique_ptr<DefStmt>> var_def_list;
    //ValDeclStmt(string name ,Pos pos,ValType);
    //vector<unique_ptr<int>> body;
    ValDeclStmt(Pos pos);
    ValDeclStmt(Pos pos,ValType type);
    ~ValDeclStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  ;

};
struct ValDefStmt :public  DefStmt
{   
    unique_ptr<ExprNode> init_expr;
    ValDefStmt(string name ,Pos pos,ValType);
    ValDefStmt(string name ,Pos pos,ValType,unique_ptr<ExprNode>);
    ~ValDefStmt();
    //vector<unique_ptr<int>> body;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  ;
};
struct ArrDefStmt :DefStmt
{
    //每行长度
    vector<unique_ptr<ExprNode>> array_length; // nullptr for non-array variables
    unique_ptr<InitializerExpr> initializers;//初始化列表
    //不知道有什么意义
    vector<int> initializers_index;
    ArrDefStmt(string name ,Pos pos,ValType);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  ;
};
struct ConstDeclStmt :public  ValDeclStmt
{   //数据在父类里
    ConstDeclStmt(Pos pos);
    ConstDeclStmt(Pos pos,ValType type);
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct ConstDefStmt :public  ValDefStmt
{   //数据在父类里
    ConstDefStmt(string name ,Pos pos,ValType);
    ConstDefStmt(string name ,Pos pos,ValType,unique_ptr<ExprNode>);
    //vector<unique_ptr<int>> body;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct ConstArrDefStmt :ArrDefStmt
{   //数据在父类里
    ConstArrDefStmt(string name ,Pos pos,ValType);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct IfStmt :public  Statement
{   
    unique_ptr<ExprNode>pred;
    //可能是一个语句，也可能是一个block
    unique_ptr<Statement> then_stmt;
    unique_ptr<Statement> else_stmt;
    IfStmt(Pos Pos);
    ~IfStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct WhileStmt :public  Statement
{   
    unique_ptr<ExprNode>pred;
    unique_ptr<Statement>loop_stmt;
    WhileStmt(Pos Pos);
    ~WhileStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct RetStmt :public  Statement
{   
    unique_ptr<ExprNode> expr;
    //vector<unique_ptr<int>> body;
    RetStmt(Pos pos);
    ~RetStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct EmptyStmt :public  Statement
{   
    //vector<unique_ptr<int>> body;
    EmptyStmt(Pos pos);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};






class ASTVisitor
{
  public:
    virtual void visit(CompunitNode &node) = 0;
    virtual void visit(FuncFParam &node) = 0;
    virtual void visit(FuncDef &node) = 0;
    virtual void visit(ValDeclStmt &node) = 0;
    virtual void visit(ValDefStmt &node) = 0;
    virtual void visit(ArrDefStmt &node) = 0;
    virtual void visit(ConstDeclStmt &node) = 0;
    virtual void visit(ConstDefStmt &node) = 0;
    virtual void visit(ConstArrDefStmt &node) = 0;
    virtual void visit(ExprStmt &node) = 0;
    virtual void visit(AssignStmt &node) = 0;
    virtual void visit(UnaryExpr &node) = 0;
    // virtual void visit(InfixExpr &node) = 0;
    virtual void visit(AssignExpr &node) = 0;
    virtual void visit(RelopExpr &node) = 0;
    virtual void visit(EqExpr &node) = 0;
    virtual void visit(AndExp &node) = 0;
    virtual void visit(ORExp &node) = 0;
    virtual void visit(BinopExpr &node) = 0;
    virtual void visit(LvalExpr &node) = 0;
    virtual void visit(IntConst &node) = 0;
    virtual void visit(InitializerExpr &node) = 0;
    virtual void visit(FloatConst &node) = 0;
    // virtual void visit(AssignStmt &node) = 0;
    virtual void visit(BlockStmt &node) = 0;
    virtual void visit(IfStmt &node) = 0;
    virtual void visit(WhileStmt &node) = 0;
    virtual void visit(CallExpr &node) = 0;
    virtual void visit(RetStmt &node) = 0;
    virtual void visit(ContinueStmt &node) = 0;
    virtual void visit(BreakStmt &node) = 0;
    virtual void visit(EmptyStmt &node) = 0;
};

}

#endif