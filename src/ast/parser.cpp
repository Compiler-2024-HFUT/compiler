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
    ast::ValType val_type{};//=curTok->type;
    bool hasdef=false;
    while(!curTokIs(tokenType::IDENT)){
        if(curTokIs(tokenType::CONST)){
            if(val_type.t.is_const==true){
                exit(114);
            }
            val_type.t.is_const=true;
        }else if(curTokIs(tokenType::DEFINT)){
            if(hasdef==true) 
                exit(12);
            val_type.t.is_float=false;
            hasdef=true;
        }else if(curTokIs(tokenType::DEFFLOAT)){
            if(hasdef==true) 
                exit(12);
            val_type.t.is_float=true;
            hasdef=true;
        }else if(curTokIs(tokenType::VOID)){
            if(hasdef==true||val_type.t.is_const==true) 
                exit(12);
            hasdef=true;
            val_type.t.is_void=true;
        }
        else {
            std::cerr<<"expect val type\n";
        }
        nextToken();
    }
    return val_type;
}
std::unique_ptr<ast::Statement> Parser::parserStmts(){
    std::unique_ptr<ast::Statement> ret;
    if(curTokIs(tokenType::RETURN)){
        ret=parserRetStmt();
    }else if(curTokIs(tokenType::DEFFLOAT)||curTokIs(tokenType::DEFINT)||curTokIs(tokenType::CONST)){
        ast::ValType type=parserDefType();
        ret=parserValDeclStmt(type);
        // for(auto &i:p->var_def_list){
        //         stmts.push_back(std::move(i));
        // }
        //nextToken();
    }else if(curTokIs(tokenType::IF)){
        ret=parserIfStmt();
    }else if(curTokIs(tokenType::WHILE)){
        ret=parserWhileStmt();
    }else if(curTokIs(tokenType::LBRACE)){
        ret=parserBlock();
    }else if(curTokIs(tokenType::CONTINUE)){
        ret=make_unique<ast::ContinueStmt>(curTok->tok_pos);
    }else if(curTokIs(tokenType::BREAK)){
        ret=make_unique<ast::BreakStmt>(curTok->tok_pos);
    }else if(curTokIs(tokenType::SEMICOLON)){
        ret=make_unique<ast::EmptyStmt>(curTok->tok_pos);
        nextToken();
    }else if(curTokIs(tokenType::IDENT)||curTokIs(tokenType::INT)||curTokIs(tokenType::FLOAT)){
        ret=parserExprStmt();
    }
    else{
        exit(14);
    }
    return ret;
}
unique_ptr<ast::CompunitNode> Parser::parserComp(){
    while(!curTokIs(tokenType::LEXEOF)){
        while(curTokIs(tokenType::SEMICOLON)){
            nextToken();
        }
        ast::ValType val=parserDefType();
        // if(val==0){
        //     std::cerr<<"错误"<<endl;
        //     exit(98);
        // }
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
        // if(peekTokIs(tokenType::SEMICOLON)||peekTokIs(tokenType::COMMA)){
        //     val_decl->var_def_list.push_back(make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type));
        //     nextToken();
        //     if(curTokIs(tokenType::COMMA)) nextToken();
        // }else if(peekTokIs(tokenType::ASSIGN)){
        //     unique_ptr<ast::ValDefStmt> val=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);
        //     nextToken();
        //     nextToken();
        //     val->init_expr=parserExpr();
        // }else{
        //     cout<<peekTok->literal<<endl;
        //     exit(81);
        // }
        val_decl->var_def_list.push_back(parserValDefStmt(val_type));
        if(curTokIs(tokenType::COMMA)){
            nextToken();
        }
    }
    nextToken();
    return val_decl;
}
unique_ptr<ast::ExprStmt> Parser::parserExprStmt(){
    auto ret=make_unique<ast::ExprStmt>(curTok->tok_pos);
    ret->expr=parserExpr();
    skipIfCurIs(tokenType::SEMICOLON);
    return ret;
}
unique_ptr<ast::ValDefStmt> Parser::parserValDefStmt(ast::ValType val_type){
    unique_ptr<ast::ValDefStmt> ret=nullptr;
    // if(peekTokIs(tokenType::ASSIGN)){
    //     ret=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);
    // }else if(peekTokIs(tokenType::COMMA)){
    //     ret=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);

    // }else if(peekTokIs(tokenType::SEMICOLON)){
    //     ret=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);
    // }else{
    //     std::cerr<<"expect ;"<<endl;
    //     exit(2);
    // }
    if(!curTokIs(tokenType::IDENT)){
        exit(81);
    }
    if(peekTokIs(tokenType::LSQ_BRACE)){
    }else{
        ret=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);
        nextToken();
        if(curTokIs(tokenType::ASSIGN)){
            nextToken();
            ret->init_expr=parserExpr();
        }
    }
        
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
    }else {
       fun->body= parserBlock();
    }


    return fun;
}
unique_ptr<ast::RetStmt>  Parser::parserRetStmt( ){
    std::unique_ptr<ast::RetStmt> ret;
    ret=make_unique<ast::RetStmt>(curTok->tok_pos);
    skipIfCurIs(tokenType::RETURN);
    ret->expr=parserExpr();
    skipIfCurIs(tokenType::SEMICOLON);
    return ret;
}
void Parser::parserArg(std::vector<std::pair<ast::ValType, string>> &argv){
    while(!curTokIs(tokenType::RPAREM)){
        ast::ValType type=parserDefType();
        // if(!curTokIs(tokenType::IDENT)){
        //     std::cerr<<"无形参"<<endl;
        //     exit(2);
        // }
        argv.push_back(std::make_pair(type, curTok->literal));
        nextToken();
        if(curTokIs(tokenType::COMMA)){
            nextToken();
        }
            
    }

}
unique_ptr<ast::IfStmt> Parser::parserIfStmt(){
    unique_ptr<ast::IfStmt> ret=make_unique<ast::IfStmt>(curTok->tok_pos);
    skipIfCurIs(tokenType::IF);
    skipIfCurIs(tokenType::LPAREM);
    ret->pred=parserExpr();
    skipIfCurIs(tokenType::RPAREM);
    //parserBlockItems(if_state->if_body);
    ret->if_stmt=parserStmts();
    if(curTokIs(tokenType::ELSE)){
        nextToken();
        //parserBlockItems(if_state->else_body);
        ret->else_stmt=parserStmts();
    }
    return ret;
}
unique_ptr<ast::WhileStmt> Parser::parserWhileStmt(){
    unique_ptr<ast::WhileStmt> ret=make_unique<ast::WhileStmt>(curTok->tok_pos);
    skipIfCurIs(tokenType::WHILE);
    skipIfCurIs(tokenType::LPAREM);
    ret->pred=parserExpr();
    skipIfCurIs(tokenType::RPAREM);
    ret->loop_stmt=std::move(parserStmts());
    return ret;
}
unique_ptr<ast::ExprNode> Parser::parserIntLiteral(){
    ast::valUnion Value;
    string s;
    // if(curTokIs(tokenType::MINUS)||curTokIs(tokenType::PLUS)){
    //     s={curTok->literal+peekTok->literal};
    //     nextToken();
    // }else{
        s={curTok->literal};
    // }

    try{
        if(curTokIs(tokenType::INT_BIN))
            Value.i=std::stol( s,0,2);
        else if(curTokIs(tokenType::INT_OCTAL))
            Value.i=std::stol( s,0,8);
        else if(curTokIs(tokenType::INT_HEX))
            Value.i=std::stol( s,0,16);
        else if(curTokIs(tokenType::INT))
            Value.i=std::stol( s);
        else 
            Value.f=std::stof( s);
    }
    catch (const std::exception&){
        std::cerr<<"stol error"<<endl;
        exit(15);
    } 
    unique_ptr<ast::IntLiteral> Ilt=make_unique<ast::IntLiteral>(curTok->tok_pos);
    Ilt->Value=Value;
    nextToken();
    return Ilt;
}
unique_ptr<ast::ExprNode> Parser::parserExpr(parserOpPrec prec){
    tokenType l_type=curTok->type;
    selectPreFn(l_type);
    if(prefixFn==nullptr){
        exit(35);
        // return nullptr;
    }
    // if(curTokIs(tokenType::ASSIGN)){
        // 
    // }
    auto leftExp=(this->*prefixFn)();
    while(!curTokIs(tokenType::SEMICOLON)&&prec<curPrecedence()){
        selectInFn(curTok->type);
        if (InfixFn==nullptr){
            // return leftExp;
            exit(234);
        }
        leftExp=(this->*InfixFn)(std::move(leftExp));
    }
    return leftExp;
}
unique_ptr<ast::CallExpr> Parser::parserCall(){
    unique_ptr<ast::CallExpr> ret=make_unique<ast::CallExpr>(curTok->tok_pos,curTok->literal);
    skipIfCurIs(tokenType::IDENT);
    skipIfCurIs(tokenType::LPAREM);
    while(!curTokIs(tokenType::RPAREM)){
        ret->arg.push_back(std::move(parserExpr()));
        if(curTokIs(tokenType::RPAREM)){
            continue;
        }else if(curTokIs(tokenType::COMMA))
            nextToken();
        else
            exit(191);
    }
    skipIfCurIs(tokenType::RPAREM);
    return ret;
}
unique_ptr<ast::ExprNode> Parser::parserLval(){
    unique_ptr<ast::ExprNode> ret=nullptr;
    if(peekTokIs(tokenType::LPAREM)){
        ret=parserCall();
    }else{
        ret=make_unique<ast::LvalExpr>(curTok->tok_pos,curTok->literal);
        skipIfCurIs(tokenType::IDENT);
    }
    while(curTokIs(tokenType::LSQ_BRACE)){
        ret=parserSuffixExpr(std::move(ret));
    }
    return ret;
}
unique_ptr<ast::ExprNode> Parser::parserSuffixExpr(unique_ptr<ast::ExprNode> left){
    unique_ptr<ast::SuffixExpr> ret=make_unique<ast::SuffixExpr>(curTok->tok_pos);
    if(curTokIs(tokenType::LSQ_BRACE)){
        nextToken();
        ret->lhs=std::move(left);
        ret->Operat="[]";
        ret->rhs=parserExpr();
        nextToken();
    }else{
        exit(123);
    }
    return ret;
}
unique_ptr<ast::ExprNode> Parser::parserGroupedExpr(){
    skipIfCurIs(tokenType::LPAREM);
    auto exp=parserExpr(parserOpPrec::LOWEST);
    // if(!curTokIs(tokenType::RPAREM)){
    //     return nullptr;
    // }
    skipIfCurIs(tokenType::RPAREM);
    return exp;
}
parserOpPrec Parser::curPrecedence(){
    parserOpPrec prefix;
    auto prec_it=precedences.find(curTok->type);
    if(prec_it!=precedences.end()){
        prefix=prec_it->second;
    }else prefix=parserOpPrec::LOWEST;
    return prefix;
    // try{
    //     prefix=this->precedences.at(curTok->type);
    // }catch(std::exception & e) {
    //     std::cerr<<e.what();
    //     //exit(1);
    //     prefix=parserOpPrec::LOWEST;
    // }
}
unique_ptr<ast::ExprNode> Parser::parserPrefixExpr(){
    unique_ptr<ast::PrefixExpr> express=make_unique<ast::PrefixExpr>(curTok->tok_pos);
    express->Operat=curTok->literal;
    nextToken();
    express->rhs=this->parserExpr(OP_PREFIX);
    return express;
}
unique_ptr<ast::ExprNode> Parser::parserInfixExpr(unique_ptr<ast::ExprNode>left){
    parserOpPrec curPrec=this->curPrecedence();
    unique_ptr<ast::InfixExpr> express=make_unique<ast::InfixExpr>(curTok->tok_pos,std::move(left));
    express->Operat=curTok->literal;
    this->nextToken();
    express->rhs=parserExpr(curPrec);
    return express;    
}
unique_ptr<ast::BlockStmt>  Parser::parserBlockItems( ){
    unique_ptr<ast::BlockStmt>  ret=make_unique<ast::BlockStmt>(curTok->tok_pos);
    unique_ptr<ast::Statement> tmp;
    while (!curTokIs(tokenType::RBRACE)) {
        tmp=parserStmts();
        if(tmp!=nullptr){
            ret->block_items.push_back(std::move(tmp));
        }
    }
    // for(auto &i:stmts){
    //     auto *p=(ast::DefStmt*)i.get();
    //     // cout<<p->name<<endl;
    // }
    return ret;
}
unique_ptr<ast::BlockStmt> Parser::parserBlock(){
    skipIfCurIs(tokenType::LBRACE);
    unique_ptr<ast::BlockStmt> ret=parserBlockItems();
    skipIfCurIs(tokenType::RBRACE);
    return ret;
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

void Parser::selectPreFn(tokenType type){
    this->prefixFn=nullptr;
    switch (type) {
        case tokenType::IDENT:
            if(peekTokIs(tokenType::LSQ_BRACE))
                prefixFn=&Parser::parserLval;
            else
                prefixFn=&Parser::parserLval;
            break;
        case tokenType::PLUS:
        case tokenType::MINUS:
            prefixFn=&Parser::parserPrefixExpr;
            break;
        case tokenType::INT:
        case tokenType::INT_BIN:
        case tokenType::INT_HEX:
        case tokenType::INT_OCTAL:
        case tokenType::FLOAT:
            prefixFn=&Parser::parserIntLiteral;
            break;
        case tokenType::LPAREM:
            prefixFn=&Parser::parserGroupedExpr;
            break;
        default:
            this->prefixFn=nullptr;
            break;

    }
}
void Parser::selectInFn(tokenType type){
    switch (type) {
        case tokenType::PLUS:
        case tokenType::MINUS:
        case tokenType::SLASH:
        case tokenType::ASTERISK:
        case tokenType::EQUAL:
        case tokenType::NOTEQUAL:
        case tokenType::LT:
        case tokenType::GT:

        case tokenType::OR:
        case tokenType::D_OR:
        case tokenType::ESPERLUTTE:
        case tokenType::D_ESPERLUTTE:
        case tokenType::ASSIGN:
            Parser::InfixFn=&Parser::parserInfixExpr;
            break;
        default:
            this->InfixFn=nullptr;
            break;

    }
}
