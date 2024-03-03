#include "node.hpp"
#include <memory>
#include <string_view>
#include <vector>
//FuncDef f{"1",Pos{1,1}};
#ifndef PARSER__AST
#define PARSER__AST
using type::ValType;
enum parserOpPrec{
    LOWEST=51,
    OP_ASSIGN,
    OP_DOR,//||
    OP_DESP,//&&
    OP_OR,//|
    OP_ESP,//&
    OP_EQUALS,//==
    OP_LESSGREATER,// > or <
    OP_SUMS,//+
    OP_PRODUCTS,//*
    OP_PREFIX,//-x or !x
    CALL ,//func(x)
};

struct Parser{
    //unique_ptr<ast::SyntaxTree> synatx;
    
    std::unique_ptr<Lexer> lex;
    unique_ptr<Token>  curTok;
    unique_ptr<Token>  peekTok;
    unique_ptr<ast::CompunitNode> comp;
    unique_ptr<ast::ExprNode> (Parser::*prefixFn)();//=&Parser::parserIdentifier;
    unique_ptr<ast::ExprNode> (Parser::*InfixFn)(unique_ptr<ast::ExprNode>);
    Pos cur_pos;
    std::string_view file_name;
    
    const std::map<tokenType, parserOpPrec>precedences= {
        {tokenType::ASSIGN,parserOpPrec::OP_ASSIGN},
        {tokenType::D_OR,parserOpPrec::OP_DOR},
        {tokenType::D_ESPERLUTTE,parserOpPrec::OP_DESP},
        {tokenType::OR,parserOpPrec::OP_OR},
        {tokenType::ESPERLUTTE,parserOpPrec::OP_ESP},
        {tokenType::EQUAL,parserOpPrec::OP_EQUALS},
        {tokenType::NOTEQUAL,parserOpPrec::OP_EQUALS},
        {tokenType::LT,parserOpPrec::OP_LESSGREATER},
        {tokenType::GT,parserOpPrec::OP_LESSGREATER},
        {tokenType::LE,parserOpPrec::OP_LESSGREATER},
        {tokenType::GE,parserOpPrec::OP_LESSGREATER},
        {tokenType::PLUS,parserOpPrec::OP_SUMS},
        {tokenType::MINUS,parserOpPrec::OP_SUMS},
        {tokenType::SLASH,parserOpPrec::OP_PRODUCTS},
        {tokenType::ASTERISK,parserOpPrec::OP_PRODUCTS},
    };
    
    
    unique_ptr<ast::CompunitNode> parserComp();
    // unique_ptr<ast::Statement> parserStmt();
    unique_ptr<ast::DefStmt> parserValDefStmt(type::ValType);
    unique_ptr<ast::ArrDefStmt> parserArrDefStmt(type::ValType);
    vector<unique_ptr<ast::ExprNode>> parserInitlizer();
    unique_ptr<ast::ValDeclStmt> parserValDeclStmt(type::ValType);
    unique_ptr<ast::FuncDef> parserFuncStmt(type::ValType type);
    unique_ptr<ast::IfStmt> parserIfStmt();
    unique_ptr<ast::ExprNode> parserIntLiteral();
    unique_ptr<ast::ExprNode> parserExpr(parserOpPrec prec=parserOpPrec::LOWEST);
    unique_ptr<ast::ExprNode> parserGroupedExpr();
    unique_ptr<ast::ExprNode> parserPrefixExpr();
    unique_ptr<ast::ExprNode> parserInfixExpr(unique_ptr<ast::ExprNode>);
    unique_ptr<ast::ExprNode> parserSuffixExpr(unique_ptr<ast::ExprNode>);
    parserOpPrec curPrecedence();
    unique_ptr<ast::WhileStmt> parserWhileStmt();
    unique_ptr<ast::Statement> parserStmts();
    unique_ptr<ast::BlockStmt> parserBlock();
    unique_ptr<ast::BlockStmt>  parserBlockItems( );
    unique_ptr<ast::RetStmt>  parserRetStmt( );
    unique_ptr<ast::ExprNode>  parserLval( );
    unique_ptr<ast::CallExpr>  parserCall(unique_ptr<ast::ExprNode> );
    unique_ptr<ast::ExprStmt> parserExprStmt();
    void parserArg(std::vector<std::pair<type::ValType, unique_ptr<ast::ExprNode>>> &);
    unique_ptr<ast::CompunitNode> getComp();
    type::ValType parserDefType();
    void nextToken();
    void skipIfCurIs(tokenType);
    Parser(std::string );
    [[deprecated]]
    void reParser(string ) ;
    void selectPreFn(tokenType);
    void selectInFn(tokenType);
    bool curTokIs(tokenType type);
    bool peekTokIs(tokenType type);
};

#endif