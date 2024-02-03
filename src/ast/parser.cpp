#include "../lexer/lex.hpp"
#include "parser.hpp"
#include "node.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
Parser::Parser(string s):lex(std::make_unique<Lexer>(s)),comp(make_unique<ast::CompunitNode>()){
    curTok=lex->nextToken();
    peekTok=lex->nextToken();
}
ast::ValType Parser::parserDefType(){
    int val_type=0;//=curTok->type;
    bool is_const=false;
    if(curTokIs(tokenType::DEFINT)||curTokIs(tokenType::DEFFLOAT)){
        val_type=curTokIs(tokenType::DEFINT)?ast::ValType::INT_VAL:ast::ValType::FLOAT_VAL;
        if(peekTokIs(tokenType::CONST)){
            is_const=true;
            nextToken();
        }
        nextToken();
    }else if(curTokIs(tokenType::VOID)){
        if(peekTokIs(tokenType::CONST)){
            std::cerr<<"const void "<<endl;
            exit(1);
        }
        nextToken();
    }else if(curTokIs(tokenType::CONST)){
        is_const=true;
        if(peekTokIs(tokenType::DEFINT)||peekTokIs(tokenType::DEFFLOAT)){
            val_type=curTok->type;
            nextToken();
            nextToken();
        }else{
            std::cerr<<"xln"<<endl;
            /*下一个不定义变量则报错*/
            exit(1);
        }
    }
    else{
        
        std::cerr<<"其他"<<curTok->literal<<endl;
        exit(1);
    }
    return (ast::ValType)( val_type+is_const);

}

unique_ptr<ast::CompunitNode> Parser::parserComp(){
    while(!curTokIs(tokenType::LEXEOF)){
        while(curTokIs(tokenType::SEMICOLON)){
            nextToken();
        }
        ast::ValType val=parserDefType();

        //是不是变量名
        if(!curTokIs(IDENT)){
            std::cerr<<"不是变量"<<endl;
            exit(1);
        }
        //检查重名
        if(this->comp->isReDef(curTok->literal)){
            std::cerr<<"重名"<<endl;
            exit(1);
        }

        unique_ptr<ast::DefStmt> p_gval=nullptr;
        if(peekTokIs(tokenType::LPAREM)){
            p_gval=parserFuncStmt(val);
            
        }else if(peekTokIs(tokenType::ASSIGN)||peekTokIs(tokenType::COMMA)){//;
            p_gval=parserValStmt(val);      
            while(curTokIs(tokenType::COMMA)){
                comp->global_defs.push_back(std::move(p_gval));
                nextToken();
                p_gval=parserValStmt(val);
            }
        }else if(peekTokIs(tokenType::SEMICOLON)){//;
            p_gval=make_unique<ast::ValStmt>(curTok->literal,Pos{curTok->line,curTok->column},val);
            nextToken();
        }else {
            std::cerr<<"定义错误"<<endl;
            exit(1);
        }
        if(curTokIs(tokenType::SEMICOLON)){
            nextToken();
        }

        if(p_gval==nullptr){
            cout<<"no p_gval"<<endl;
            continue;
        }else{
        }
        comp->global_defs.push_back(std::move(p_gval));
    }
    return nullptr;
}
unique_ptr<ast::ValDeclStmt> Parser::parserValDeclStmt(ast::ValType val_type){

}

unique_ptr<ast::ValDefStmt> Parser::parserValDefStmt(ast::ValType val_type){
    unique_ptr<ast::ValDefStmt> ret=nullptr;
    if(peekTokIs(tokenType::ASSIGN)){

    }else if(peekTokIs(tokenType::COMMA)){
        ret=make_unique<ast::ValDefStmt>(curTok->literal,Pos{curTok->line,curTok->column},val_type);

    }else if(peekTokIs(tokenType::SEMICOLON)){
        ret=make_unique<ast::ValDefStmt>(curTok->literal,Pos{curTok->line,curTok->column},val_type);
    }else{
        std::cerr<<"expect ;"<<endl;
        exit(2);
    }
    
    nextToken();
        
    return ret;
}
unique_ptr<ast::funcDef> Parser::parserFuncStmt(ast::ValType val_type){
    unique_ptr<ast::funcDef> fun=make_unique<ast::funcDef>(curTok->literal,Pos(curTok->line,curTok->column),val_type);
    if(!peekTokIs(tokenType::LPAREM)){
        exit(2);
    }
    nextToken();
    //skip(
    nextToken();
    parserArg(fun->argv);
    
    nextToken();
    if(curTokIs(tokenType::SEMICOLON)){
        nextToken();
    }else if(curTokIs(tokenType::LBRACE)){
        nextToken();
        parserBlockItems( fun->body);
        if(!curTokIs(tokenType::RBRACE)){
            exit(2);
        }
        nextToken();
    }


    return fun;
}
void Parser::parserArg(std::vector<std::pair<ast::ValType, string>> &argv){
    while(!curTokIs(tokenType::RPAREM)){
        ast::ValType type=parserDefType();
        if(!curTokIs(tokenType::IDENT)){
            std::cerr<<"无形参"<<endl;
            exit(2);
        }
        argv.push_back(std::make_pair(type, curTok->literal));
        nextToken();
        if(curTokIs(tokenType::COMMA)){
            nextToken();
        }
            
    }

}
unique_ptr<ast::ifStmt> Parser::parserIfStmt(vector<unique_ptr<ast::Statement>>& stmt){
    unique_ptr<ast::ifStmt> if_state=make_unique<ast::ifStmt>(Pos(curTok->line,curTok->column));
    nextToken();
    skipIfCurIs(tokenType::LPAREM);
    parserExpr();
    skipIfCurIs(tokenType::RPAREM);
    skipIfCurIs(tokenType::LBRACE);
    parserBlockItems(if_state->if_body);
    skipIfCurIs(tokenType::RBRACE);
    if(curTokIs(tokenType::ELSE)){
        nextToken();
        skipIfCurIs(tokenType::LBRACE);
        parserBlockItems(if_state->else_body);
        skipIfCurIs(tokenType::RBRACE);
    }
    return if_state;
}
unique_ptr<ast::ExprNode> Parser::parserExpr(){
    while(!curTokIs(tokenType::RPAREM)&&!curTokIs(tokenType::SEMICOLON)){
        nextToken();
    }
    return nullptr;
}
void Parser::parserBlockItems( vector<unique_ptr<ast::Statement>>& stmt){
    vector <string> val_names;
    while (!curTokIs(tokenType::RBRACE)) {
        while(curTokIs(tokenType::SEMICOLON)){
            nextToken();
        }
        if(curTokIs(tokenType::RETURN)){
            stmt.push_back(make_unique<ast::RetStmt>(Pos(curTok->line,curTok->column)));
            skipIfCurIs(tokenType::SEMICOLON);
        }else if(curTokIs(tokenType::DEFFLOAT)||curTokIs(tokenType::DEFINT)||curTokIs(tokenType::CONST)){
            ast::ValType type=parserDefType();
            if(!curTokIs(tokenType::IDENT)){
                std::cerr<<"expect a val"<<endl;
                exit(114);
            }
            for(auto &i_name:val_names){
                if(i_name==curTok->literal){
                    std::cerr<<"re def"<<endl;
                }
            }
            val_names.push_back(curTok->literal);
            
            //nextToken();
            stmt.push_back(parserValStmt(type));
            skipIfCurIs(tokenType::SEMICOLON);
        }else if(curTokIs(tokenType::IF)){
            parserIfStmt(stmt);
        }else if(curTokIs(tokenType::WHILE)){
            exit(5);
        }else{
            exit(14);
        }
    }
    for(auto &i:stmt){
        auto *p=(ast::DefStmt*)i.get();
        cout<<p->name<<endl;
    }
}
void Parser::nextToken(){
    curTok.reset();
    curTok=std::move(peekTok);
    peekTok=lex->nextToken();
}

bool inline Parser::curTokIs(tokenType type){
    return this->curTok->type==type;
}
bool inline Parser::peekTokIs(tokenType type){
    return this->peekTok->type==type;
}

void Parser::skipIfCurIs(tokenType type){
    if(!curTokIs(type)){
        std::cerr<<"expect error";
        exit(2);
    }
    nextToken();
}

// unique_ptr<ast::Statement> Parser::parserStmt(){
//         int val_type;//=curTok->type;
//         bool is_const=false;
//         if(curTokIs(tokenType::DEFINT)||curTokIs(tokenType::DEFFLOAT)){
//             val_type=curTokIs(tokenType::DEFINT)?ast::ValType::INT_VAL:ast::ValType::FLOAT_VAL;
//             if(peekTokIs(tokenType::CONST)){
//                 is_const=true;
//                 nextToken();
//             }
//             nextToken();
//         }else if(curTokIs(tokenType::VOID)){
//              if(peekTokIs(tokenType::CONST)){
//                 std::cerr<<"const void "<<endl;
//                 exit(1);
//             }
//             nextToken();
//         }else if(curTokIs(tokenType::CONST)){
//             is_const=true;
//             if(peekTokIs(tokenType::DEFINT)||peekTokIs(tokenType::DEFFLOAT)){
//                 val_type=curTok->type;
//                 nextToken();
//                 nextToken();
//             }else{
//                 std::cerr<<"xln"<<endl;
//                 /*下一个不定义变量则报错*/
//                 exit(1);
//             }
//         }
//         // else if(curTokIs(tokenType::SEMICOLON)){
//         //     nextToken();
//         //     continue;
//         // }
//         else{
//             std::cerr<<"其他"<<endl;
//             exit(1);
//         }
//         //是不是变量名
//         if(!curTokIs(IDENT)){
//             std::cerr<<"不是变量"<<endl;
//             exit(1);
//         }
//         //检查重名
//         if(this->comp->isReDef(curTok->literal)){
//             std::cerr<<"重名"<<endl;
//             exit(1);
//         }


//     unique_ptr<ast::Statement> p_gval=nullptr;
//     if(peekTokIs(tokenType::LPAREM)){
//         p_gval=parserFuncStmt((ast::ValType)(val_type+is_const));
        
//     }else if(peekTokIs(tokenType::ASSIGN)||peekTokIs(tokenType::COMMA)){//;
//         p_gval=parserValStmt((ast::ValType)(val_type+is_const));
//         while(curTokIs(tokenType::COMMA)){}
//     }
//     else if(peekTokIs(tokenType::SEMICOLON)){//;
//         p_gval=make_unique<ast::ValStmt>(curTok->literal,Pos{curTok->line,curTok->column},(ast::ValType)(val_type+is_const));
//         nextToken();
//     }else {
//         std::cerr<<"定义错误"<<endl;
//         exit(1);
//     }


//     return p_gval;
// }