#include "node.hpp"
#include <memory>
#include <vector>
//FuncDef f{"1",Pos{1,1}};
struct Parser{
    //unique_ptr<ast::SyntaxTree> synatx;
    std::unique_ptr<Lexer> lex;
    unique_ptr<Token>  curTok;
    unique_ptr<Token>  peekTok;
    unique_ptr<ast::CompunitNode> comp;
    unique_ptr<ast::CompunitNode> parserComp();
    // unique_ptr<ast::Statement> parserStmt();
    unique_ptr<ast::ValDefStmt> parserValDefStmt(ast::ValType);
    unique_ptr<ast::ValDeclStmt> parserValDeclStmt(ast::ValType);
    unique_ptr<ast::FuncDef> parserFuncStmt(ast::ValType type);
    unique_ptr<ast::IfStmt> parserIfStmt(vector<unique_ptr<ast::Statement>>& stmt);
    unique_ptr<ast::ExprNode> parserExpr();
    unique_ptr<ast::WhileStmt> parserWhileStmt();
    void parserStmts(vector<unique_ptr<ast::Statement>>&stmts);
    unique_ptr<ast::BlockStmt> parserBlock();
    void parserBlockItems( vector<unique_ptr<ast::Statement>>&);
    void parserArg(std::vector<std::pair<ast::ValType, string>> &);
    unique_ptr<ast::CompunitNode> getComp();
    ast::ValType parserDefType();
    void nextToken();
    void skipIfCurIs(tokenType);
    Parser(string s);
    bool curTokIs(tokenType type);
    bool peekTokIs(tokenType type);
};