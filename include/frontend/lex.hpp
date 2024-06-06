#include <algorithm>
#include <memory>
#include<string>
#include<iostream>
#ifndef __LEXER__
#define __LEXER__
using std::string,std::cout,std::cin,std::endl,std::unique_ptr,std::make_unique,std::move;
enum class tokenType{
    LEXEOF=-1,
    ILLEGAL=1,//illegal
    IDENT, //标识符
    INT,//字面量
    FLOAT,
    INT_BIN,
    INT_OCTAL,
    INT_HEX,
    BANG,//!
    ASSIGN,//=
    PLUS,//+
    MINUS,//-
    ASTERISK,// *
    SLASH,// /
    MOD,
    OR,//|
    ESPERLUTTE,//&
    
    D_ESPERLUTTE,//&&
    D_OR,//||
    EQUAL,//==
    NOTEQUAL,//!=
    LT,// <
    LE,//,=
    GT,//> 
    GE,//>=
    COMMA,//,
    SEMICOLON,//;
    LPAREM,//(
    RPAREM,//)
    LSQ_BRACE,//square方括号[
    RSQ_BRACE,//]
    LBRACE,//{
    RBRACE,//}
    FUNCTION,//function
    // LET,//let
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    KW_RETURN,
    KW_CONST,
    KW_INT,
    KW_FLOAT,
    KW_VOID,
    KW_CONTINUE,
    KW_BREAK,
};
struct Pos{
    int line;
    int column;
    Pos(int,int);
    Pos();
};
union PosUinon{
    Pos pos;
    long l;
};
struct Token{
    string literal;
    enum::tokenType type;
    // int line;
    // int column;
    Pos tok_pos;
    Token(string,enum::tokenType);
    Token(string,enum::tokenType,int,int);
    // Token(string);
    Token(string,int,int);
    Token(int,enum::tokenType);
    enum::tokenType lookupIdent();
};
struct Lexer{
    string input;
    long position;
    long readPosition;//
    int ch;//当前查看的字符
    int line;
    int column;

    //init
    Lexer(string input);
    void skipwhite();
    void skipOther();
    int readChar();
    int peekChar();
    unique_ptr<Token>  nextToken();
    string readIdentifier();
    string readNumber(tokenType &type);
    
    
};



#endif