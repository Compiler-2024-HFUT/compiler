#include "../lexer/lex.hpp"
#include "parser.hpp"
#include "node.hpp"
#include <iostream>
#include <memory>
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
        }
        // else{
        //     std::cerr<<"xln"<<endl;
        //     /*下一个不定义变量则报错*/
        //     exit(1);
        // }
    }
    // else{
        
    //     std::cerr<<"其他"<<curTok->literal<<endl;
    //     exit(1);
    // }
    return (ast::ValType)( val_type+is_const);

}
void Parser::parserStmts(vector<unique_ptr<ast::Statement>>&stmts){
    if(curTokIs(tokenType::RETURN)){
        stmts.push_back(make_unique<ast::RetStmt>(curTok->tok_pos));
        skipIfCurIs(tokenType::SEMICOLON);
    }else if(curTokIs(tokenType::DEFFLOAT)||curTokIs(tokenType::DEFINT)||curTokIs(tokenType::CONST)){
        ast::ValType type=parserDefType();
        if(!curTokIs(tokenType::IDENT)){
            std::cerr<<"expect a val"<<endl;
            exit(114);
        }
        // for(auto &i_name:val_names){
        //     if(i_name==curTok->literal){
        //         std::cerr<<"re def"<<endl;
        //     }
        // }
        unique_ptr<ast::ValDeclStmt> p=parserValDeclStmt(type);
        for(auto &i:p->var_def_list){
                stmts.push_back(std::move(i));
        }
        //nextToken();
    }else if(curTokIs(tokenType::IF)){
        parserIfStmt(stmts);
    }else if(curTokIs(tokenType::WHILE)){
        stmts.push_back(parserWhileStmt());
    }else if(curTokIs(tokenType::LBRACE)){
        parserBlock();
    }else if(curTokIs(tokenType::SEMICOLON)){
        nextToken();
    }else{
        exit(14);
    }

}
unique_ptr<ast::CompunitNode> Parser::parserComp(){
    while(!curTokIs(tokenType::LEXEOF)){
        while(curTokIs(tokenType::SEMICOLON)){
            nextToken();
        }
        ast::ValType val=parserDefType();
        if(val==0){
            std::cerr<<"错误"<<endl;
            exit(98);
        }
        //是不是变量名
        if(!curTokIs(IDENT)){
            std::cerr<<"不是变量"<<endl;
            exit(10);
        }
        //检查重名
        if(this->comp->isReDef(curTok->literal)){
            std::cerr<<"重名"<<endl;
            exit(45);
        }

        unique_ptr<ast::DefStmt> p_gval=nullptr;
        if(peekTokIs(tokenType::LPAREM)){
            p_gval=parserFuncStmt(val);
            comp->global_defs.push_back(std::move(p_gval));
        }else{
            unique_ptr<ast::ValDeclStmt> p=parserValDeclStmt(val);
            for(auto &i:p->var_def_list){
                if(this->comp->isReDef(i->name)){
                    std::cerr<<"重名"<<endl;
                    exit(45);
                }else{
                    comp->global_defs.push_back(std::move(i));
                }
            }
            //std::move(p->var_def_list.begin(),p->var_def_list.end(),comp->global_defs.end());
        }

        
    }
    return nullptr;
}
unique_ptr<ast::ValDeclStmt> Parser::parserValDeclStmt(ast::ValType val_type){
    unique_ptr<ast::ValDeclStmt>val_decl=make_unique<ast::ValDeclStmt>(curTok->tok_pos);
    while(!curTokIs(tokenType::SEMICOLON)){
        if(peekTokIs(tokenType::SEMICOLON)||peekTokIs(tokenType::COMMA)){
            val_decl->var_def_list.push_back(make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type));
            nextToken();
            if(curTokIs(tokenType::COMMA)) nextToken();
        }else if(peekTokIs(tokenType::ASSIGN)){
            unique_ptr<ast::ValDefStmt> val=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);
            nextToken();
            nextToken();
            val->init_expr=parserExpr();
        }else{
            cout<<curTok->literal<<endl;
            exit(81);
        }
        
    }
    nextToken();
    return val_decl;
}

unique_ptr<ast::ValDefStmt> Parser::parserValDefStmt(ast::ValType val_type){
    unique_ptr<ast::ValDefStmt> ret=nullptr;
    if(peekTokIs(tokenType::ASSIGN)){

    }else if(peekTokIs(tokenType::COMMA)){
        ret=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);

    }else if(peekTokIs(tokenType::SEMICOLON)){
        ret=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);
    }else{
        std::cerr<<"expect ;"<<endl;
        exit(2);
    }
    
    nextToken();
        
    return ret;
}
unique_ptr<ast::FuncDef> Parser::parserFuncStmt(ast::ValType val_type){
    unique_ptr<ast::FuncDef> fun=make_unique<ast::FuncDef>(curTok->literal,curTok->tok_pos,val_type);
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
        skipIfCurIs(tokenType::RBRACE);
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
unique_ptr<ast::IfStmt> Parser::parserIfStmt(vector<unique_ptr<ast::Statement>>& stmt){
    unique_ptr<ast::IfStmt> if_state=make_unique<ast::IfStmt>(curTok->tok_pos);
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
unique_ptr<ast::WhileStmt> Parser::parserWhileStmt(){
    cout<<11<<endl;
    unique_ptr<ast::WhileStmt> ret=make_unique<ast::WhileStmt>(curTok->tok_pos);
    skipIfCurIs(tokenType::WHILE);
    skipIfCurIs(tokenType::LPAREM);
    ret->pred=parserExpr();
    skipIfCurIs(tokenType::RPAREM);
    skipIfCurIs(tokenType::LBRACE);
    parserBlockItems(ret->loop_body);
    skipIfCurIs(tokenType::RBRACE);
    return ret;
}

unique_ptr<ast::ExprNode> Parser::parserExpr(){
    while(!curTokIs(tokenType::RPAREM)&&!curTokIs(tokenType::SEMICOLON)){
        nextToken();
    }
    return nullptr;
}
void Parser::parserBlockItems( vector<unique_ptr<ast::Statement>>& stmts){
    vector <string> val_names;
    while (!curTokIs(tokenType::RBRACE)) {
        parserStmts(stmts);
    }

    for(auto &i:stmts){
        auto *p=(ast::DefStmt*)i.get();
        // cout<<p->name<<endl;
    }
}
unique_ptr<ast::BlockStmt>Parser::parserBlock(){

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
        std::cerr<<"expect error"<<type<<endl;
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
//         p_gval=make_unique<ast::ValStmt>(curTok->literal,curTok->tok_pos,(ast::ValType)(val_type+is_const));
//         nextToken();
//     }else {
//         std::cerr<<"定义错误"<<endl;
//         exit(1);
//     }


//     return p_gval;
// }