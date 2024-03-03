#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "../lexer/lex.hpp"
#include "type.hpp"
using std::cout,std::string,std::vector;
using type::ValType;
namespace ast {
class Visitor;

enum ExprType{
    FLOAT_LITERAL,
    INT_LITERAL,
    INFIX,
    PREFIX,
    SUFFIX,
    CALL_EXPR,
    LVAL_EXPR,
};
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
    virtual void accept(Visitor &visitor)=0 ;
};
struct ExprNode: SyntaxNode {
//   public:
//     int line;
//     // 用于访问者模式
    //std::unique_ptr<Token> tok;//记录位置
    ExprNode(Pos pos);
    virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(Visitor &visitor) =0;
};
struct PrefixExpr:public ExprNode{
    string Operat;//type
    unique_ptr<ExprNode> rhs;
    PrefixExpr(Pos pos);
    virtual int getType()override;
    virtual void print(int level=0)override;
    virtual void accept(Visitor &visitor) override final;
};
struct SuffixExpr:public ExprNode{
    string Operat;//type
    unique_ptr<ExprNode> lhs;
    unique_ptr<ExprNode> rhs;//数组[i]
    SuffixExpr(Pos pos);
    virtual int getType()override;
    virtual void print(int level=0)override;
    virtual void accept(Visitor &visitor)  final;
};
struct InfixExpr:public ExprNode{
    string Operat;
    unique_ptr<ExprNode> rhs;
    unique_ptr<ExprNode> lhs;
    InfixExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    ~InfixExpr();
    virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(Visitor &visitor)=0;

};

struct AssignExpr:public InfixExpr{
    AssignExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};
struct RelopExpr:public InfixExpr{
    RelopExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};
struct BinopExpr:public InfixExpr{
    BinopExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};
///////中缀表达式/////
union valUnion{
    float f;
    int   i;
};
struct Literal:public ExprNode{
    valUnion Value;
    Literal(Pos pos,valUnion);
    virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(Visitor &visitor)  =0;
};
struct IntLiteral:public Literal{
    IntLiteral(Pos pos,valUnion);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};
struct FloatLiteral:public Literal{
    FloatLiteral(Pos pos,valUnion);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};
struct CallExpr:public ExprNode{
    // string name;//type
    unique_ptr<ExprNode> call_name;
    vector<unique_ptr<ast::ExprNode>> arg;
    CallExpr(Pos pos);
    CallExpr(Pos pos,string name);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};

struct LvalExpr:public ExprNode{
    string name;
    // unique_ptr<ExprNode> expr;
    LvalExpr(Pos pos,string name);
    // LvalExpr(string name ,Pos pos,ValType,unique_ptr<ExprNode>);
    // virtual void print();
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;

};
struct Statement:public SyntaxNode{
    Statement(Pos pos );
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
};
struct ExprStmt:public Statement{
    unique_ptr<ast::ExprNode> expr;
    ExprStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor);
};
struct BreakStmt:public Statement{
    BreakStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};
struct ContinueStmt:public Statement{
    ContinueStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};
struct BlockStmt :public  Statement{
    vector<unique_ptr<Statement>> block_items;
    BlockStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};
//抽象类
struct DefStmt:public Statement{
    string name;
    ValType val_type;//变量类型
    DefStmt (string name ,Pos pos,ValType );
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(Visitor &visitor)=0;
};

struct CompunitNode : public SyntaxNode
{
    vector<std::unique_ptr<DefStmt>> global_defs;
    bool isReDef(string s);
    CompunitNode();
    ~CompunitNode();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor);
};
/*语法树*/
struct stynaxTree{
    unique_ptr<CompunitNode >root;
};


/*函数声明*/
struct FuncStmt :public  DefStmt
{
    FuncStmt(string name ,Pos pos,ValType );
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(Visitor &visitor) =0;
};
/*函数定义*/
struct FuncDef :public  FuncStmt
{   
    unique_ptr<BlockStmt> body;
    std::vector<std::pair<ValType, unique_ptr<ExprNode>>>  argv;
    // FuncDef(string name ,Pos pos);
    FuncDef(string name ,Pos pos,ValType );
    ~FuncDef();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;

    // bool isReDef(string tok_name);
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
    virtual void accept(Visitor &visitor)  final;
};
struct ArrDefStmt :DefStmt
{
    //每行长度
    vector<unique_ptr<ExprNode>> array_length; // nullptr for non-array variables
    vector<unique_ptr<ExprNode>> initializers;//初始化列表
    //不知道有什么意义
    vector<int> initializers_index;
    ArrDefStmt(string name ,Pos pos,ValType);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
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
    virtual void accept(Visitor &visitor)  final;

};
struct IfStmt :public  Statement
{   
    unique_ptr<ExprNode>pred;
    //可能是一个语句，也可能是一个block
    unique_ptr<Statement> if_stmt;
    unique_ptr<Statement> else_stmt;
    IfStmt(Pos Pos);
    ~IfStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;

};
struct WhileStmt :public  Statement
{   
    unique_ptr<ExprNode>pred;
    unique_ptr<Statement>loop_stmt;
    WhileStmt(Pos Pos);
    ~WhileStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;

};
struct RetStmt :public  Statement
{   
    unique_ptr<ExprNode> expr;
    //vector<unique_ptr<int>> body;
    RetStmt(Pos pos);
    ~RetStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;

};
struct EmptyStmt :public  Statement
{   
    //vector<unique_ptr<int>> body;
    EmptyStmt(Pos pos);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(Visitor &visitor)  final;
};






class Visitor
{
  public:
    virtual void visit(CompunitNode &node) = 0;
    virtual void visit(FuncDef &node) = 0;
    virtual void visit(ValDeclStmt &node) = 0;
    virtual void visit(ValDefStmt &node) = 0;
    virtual void visit(ArrDefStmt &node) = 0;
    virtual void visit(ExprStmt &node) = 0;
    virtual void visit(PrefixExpr &node) = 0;
    // virtual void visit(InfixExpr &node) = 0;
    virtual void visit(AssignExpr &node) = 0;
    virtual void visit(RelopExpr &node) = 0;
    virtual void visit(BinopExpr &node) = 0;
    virtual void visit(SuffixExpr &node) = 0;
    virtual void visit(LvalExpr &node) = 0;
    virtual void visit(IntLiteral &node) = 0;
    virtual void visit(FloatLiteral &node) = 0;
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