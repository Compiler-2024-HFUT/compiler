#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "../lexer/lex.hpp"
using std::cout,std::string,std::vector;

namespace ast {
enum StmtType{
    NULL_STMT,
    VAL_DEF_STMT,
    VAL_DECL_STMT,
    FUNSTMT,
    CONSTSTMT,
    RETURNSTMT,
    IFSTMT,
    WHILE_STMT
};
enum ValType{
    INT_VAL=1,
    INT_CONST,
    INT_POINT,
    INT_POINT_CONST,
    FLOAT_VAL,
    FLOAT_CONST,
    FLOAT_POINT,
    FLOAT_POINT_CONST,
    VOID_VAL,
};

struct SyntaxNode {
//   public:
//     int line;
//     // 用于访问者模式
    //std::unique_ptr<Token> tok;//记录位置
    // Pos pos;
    // SyntaxNode(Pos pos):pos(pos ){}
    Pos pos;
    SyntaxNode(Pos);
    virtual int getType()=0;
};
struct ExprNode: SyntaxNode {
//   public:
//     int line;
//     // 用于访问者模式
    //std::unique_ptr<Token> tok;//记录位置
};
struct PrefixExpr:public ExprNode{
    int Operat;//type
    unique_ptr<ExprNode> rhs;

};

///////中缀表达式/////
struct InfixExpr:public PrefixExpr{
    // unique_ptr<Token> tok;
    // string Operat;
    // unique_ptr<Expression> rhs;
    unique_ptr<ExprNode> lhs;

};
struct Statement:public SyntaxNode{
    Statement(Pos pos );
    virtual int getType()=0;
};
struct BlockStmt :public  Statement
{
    //vector<tokenType> argvType;
    // unique_ptr<FuncDef> fundef;
    vector<unique_ptr<Statement>> block_items;
    virtual int getType();
};
//抽象类
struct DefStmt:public Statement{
    string name;
    ValType val_type;//变量类型
    DefStmt (string name ,Pos pos,ValType );
    virtual int getType()=0;
};
struct GlobalDefNode:public SyntaxNode {
//   public:
//     int line;
//     // 用于访问者模式
    int type;
};
struct CompunitNode //: public SyntaxNode
{
    vector<std::unique_ptr<DefStmt>> global_defs;
    bool isReDef(string s);
    CompunitNode(){}
};
/*语法树*/
struct stynaxTree{
    unique_ptr<CompunitNode >root;
};

// struct FuncDef :public  FuncStmt
// {   
//     vector<unique_ptr<int>> body;
//     std::map<tokenType, string>  argv;
//     FuncDef(string name ,Pos pos);
// };

/*函数声明*/
struct FuncStmt :public  DefStmt
{
    //vector<tokenType> argvType;
    // unique_ptr<FuncDef> fundef;
    FuncStmt(string name ,Pos pos,ValType );
    virtual int getType();
};
/*函数定义*/
struct FuncDef :public  FuncStmt
{   
    vector<unique_ptr<Statement>> body;
    std::vector<std::pair<ValType, string>>  argv;
    // FuncDef(string name ,Pos pos);
    FuncDef(string name ,Pos pos,ValType );
    virtual int getType();
    bool isReDef(string tok_name);
};
/*全局变量声明*/
struct GlobalValState :public  Statement
{
    unique_ptr<ExprNode> expr;
    //vector<unique_ptr<int>> body;

};
struct ValDefStmt :public  DefStmt
{   
    unique_ptr<ExprNode> init_expr;
    ValDefStmt(string name ,Pos pos,ValType);
    ValDefStmt(string name ,Pos pos,ValType,unique_ptr<ExprNode>);
    //vector<unique_ptr<int>> body;
    virtual int getType();
};
struct LvalStmt:public DefStmt{
    unique_ptr<ExprNode> expr;
    LvalStmt(string name ,Pos pos,ValType);
    LvalStmt(string name ,Pos pos,ValType,unique_ptr<ExprNode>);

};

struct ValDeclStmt :public  Statement
{   
    vector<unique_ptr<ValDefStmt>> var_def_list;
    //ValDeclStmt(string name ,Pos pos,ValType);
    //vector<unique_ptr<int>> body;
    ValDeclStmt(Pos pos);
    virtual int getType();

};
struct IfStmt :public  Statement
{   
    unique_ptr<ExprNode>pred;
    vector<unique_ptr<Statement>> if_body;
    vector<unique_ptr<Statement>> else_body;
    //unique_ptr<ExpreNode> expr;
    //vector<unique_ptr<int>> body;
    //RetStmt(Pos pos);
    IfStmt(Pos Pos);
    virtual int getType();

};
struct WhileStmt :public  Statement
{   
    unique_ptr<ExprNode>pred;
    vector<unique_ptr<Statement>> loop_body;
    //unique_ptr<ExpreNode> expr;
    //vector<unique_ptr<int>> body;
    //RetStmt(Pos pos);
    WhileStmt(Pos Pos);
    virtual int getType();

};
struct RetStmt :public  Statement
{   
    unique_ptr<ExprNode> expr;
    //vector<unique_ptr<int>> body;
    RetStmt(Pos pos);
    virtual int getType();

};
// struct BodyNode: public SyntaxNode{

// };
// struct block_syntax : stmt_syntax
// {
//     ptr_list<stmt_syntax> body;
// };



}

#endif