#include "lex.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <memory>
#include <string>
Pos::Pos(int _x,int _y):line(_x),column(_y){
}
Token::Token(string literal,enum::tokenType type):literal(literal), type(type){
}
Token::Token(string literal,enum::tokenType type,int line,int column):literal(literal), type(type),line(line),column(column){
}
Token::Token(int ch,enum::tokenType type): literal(1,ch), type(type){  
}
Token::Token(string literal):literal(literal),type(lookupIdent()){
}
Token::Token(string literal,int line,int column):literal(literal),type(lookupIdent()),line(line),column(column){
}
enum::tokenType Token::lookupIdent(){
    tokenType ret;
    // if(this->literal=="let"){
    //     ret=tokenType::LET;
    // }else 
    if(literal=="fn"){
        ret=tokenType::FUNCTION;
    }else if(literal=="if"){
        ret=tokenType::IF;
    }else if(literal=="else"){
        ret=tokenType::ELSE;
    }else if(literal=="while"){
        ret=tokenType::WHILE;
    }else if(literal=="for"){
        ret=tokenType::FOR;
    }else if(literal=="return"){
        ret=tokenType::RETURN;
    }else if(literal=="const"){
        ret=tokenType::CONST;
    }else if(literal=="int"){
        ret=tokenType::DEFINT;
    }else if(literal=="float"){
        ret=tokenType::DEFFLOAT;
    }else if(literal=="void"){
        ret=tokenType::VOID;
    }
    else ret=tokenType::IDENT;
    return ret;
}

Lexer::Lexer(string input) :input(input),readPosition(1),position(0),ch(input[0]),line(1),column(1) {}
int Lexer::readChar(){
    ch=peekChar();
    this->position++;
    this->readPosition++;
    this->column++;
    return ch;
}
int Lexer::peekChar(){
    int ret;
    if(this->readPosition>this->input.length()){
        ret=0;
    }else{
        ret=this->input[readPosition];      
    }
    return ret;
}

std::unique_ptr<Token>   Lexer::nextToken(/*std::unique_ptr<Lexer> l*/){
    std::unique_ptr<Token> tok=nullptr;
    // bool flagRead=true;
    this->skipOther();
    int l=this->line,c=this->column;
    // cout<<"ch is "<<(char)ch<<endl;
    switch (this->ch){
    case '|':
        if(this->peekChar()=='|'){
            this->readChar();
            tok=std::make_unique<Token>("||",tokenType::D_OR);
        }else
            tok=std::make_unique<Token>("!",tokenType::BANG);
        break;
    case '&':
        if(this->peekChar()=='&'){
            this->readChar();
            tok=std::make_unique<Token>("&&",tokenType::D_ESPERLUTTE);
        }else
            tok=std::make_unique<Token>("&",tokenType::ESPERLUTTE);
        break;
    case '!':
        if(this->peekChar()=='='){
            this->readChar();
            tok=std::make_unique<Token>("!=",tokenType::NOTEQUAL);
        }
        else
            tok=std::make_unique<Token>("!",tokenType::BANG);
        break;
    case '=':
        if(this->peekChar()=='='){
            this->readChar();
            tok=std::make_unique<Token>("==",tokenType::EQUAL);
        }
        else
            tok=std::make_unique<Token>("=",tokenType::ASSIGN);
        break;
    case '+':
        tok=std::make_unique<Token>("+",tokenType::PLUS);
        break;
    case '-':
        tok=std::make_unique<Token>("-",tokenType::MINUS);
        break;
    case '*':
        tok=std::make_unique<Token>("*",tokenType::ASTERISK);
        break;
    case '/':
        tok=std::make_unique<Token>("/",tokenType::SLASH);
        break;
    case ',':
        tok=std::make_unique<Token>(",",tokenType::COMMA);
        break;
    case ';':
        tok=std::make_unique<Token>(";",tokenType::SEMICOLON);
        break;
    case '(':
        tok=std::make_unique<Token>("(",tokenType::LPAREM);  
        break;
    case ')':
        tok=std::make_unique<Token>(")",tokenType::RPAREM);
        break;
    case '[':
        tok=std::make_unique<Token>("[",tokenType::LSQ_BRACE);  
        break;
    case ']':
        tok=std::make_unique<Token>("]",tokenType::RSQ_BRACE);
        break;
    case '{':
        tok=std::make_unique<Token>("{",tokenType::LBRACE);  
        break;
    case '}':
        tok=std::make_unique<Token>("}",tokenType::RBRACE);
        break;
    case 0:
        tok=std::make_unique<Token>("",tokenType::LEXEOF);
        // flagRead=false;
        break;
    default:
        if(isalpha(this->ch)||this->ch=='_'){
            tok=std::make_unique<Token>(readIdentifier(),l,c);
            // flagRead=false;
            return std::move(tok);
        }else if(isdigit(this->ch)){
            tok=std::make_unique<Token>(readNumber(),tokenType::INT,l,c);
            return std::move(tok);
        }
        else{
            tok=std::make_unique<Token>(this->ch,tokenType::ILLEGAL);
            exit(1);
        }
        
    }
    // if(flagRead){
    //     this->readChar();
    // }
    tok->line=l;
    tok->column=c;
    this->readChar();

    return std::move(tok);

}
string Lexer::readIdentifier(){
    int beginpos=this->position;
    int sublen=0;
    if(isalpha(this->input[position])||this->input[position]=='_'){
        readChar();
        sublen++;
        while(isalnum(this->input[position])||this->input[position]=='_'){
            readChar();
            sublen++;
        }
    }
    return this->input.substr(beginpos,sublen);
}
void Lexer::skipwhite(){
    while(isspace(this->ch)){ 
        if(this->ch=='\n'){
            this->column=0;
            this->line++;
        }
        readChar();
    
    }
}
void Lexer::skipOther(){
    //单行注释
    this->skipwhite();
    while(ch=='/'&&(peekChar()=='/'||peekChar()=='*')){
        if(peekChar()=='/'){
            while(this->ch!='\n'){ 
                readChar();
            }
        line++;
        column=0;
        readChar();
        }
        /*多行注释*/
        else if(peekChar()=='*'){
            readChar();
            readChar();
            while(ch!='*'||peekChar()!='/'){
                if(this->ch=='\n'){
                    this->column=0;
                    this->line++;
                }
                readChar();
            }
            readChar();
            readChar();
        }
    this->skipwhite();
    }
}
string Lexer::readNumber(){
    int beginpos=this->position;
    int sublen=0;
    if(this->input[position]=='0'){
        if(this->input[readPosition]=='x'||this->input[readPosition]=='X'||this->input[readPosition]=='b'||this->input[readPosition]=='B'){
            readChar();
            readChar();
            sublen+=2;
        }
    }
    while(isdigit(this->input[position])||this->input[position]=='.'){
        readChar();
        sublen++;
    }
    return this->input.substr(beginpos,sublen);
}