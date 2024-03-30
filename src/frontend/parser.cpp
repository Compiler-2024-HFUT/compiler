#include "frontend/lex.hpp"
#include "frontend/parser.hpp"
#include "frontend/node.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <fstream>
#include <vector>
#include "frontend/type.hpp"
#include "frontend/riscv.hpp"
// Parser::Parser(string s):lex(std::make_unique<Lexer>(s)),comp(make_unique<ast::CompunitNode>()),cur_pos(0,0){
//     curTok=lex->nextToken();
//     peekTok=lex->nextToken();
// }
::ast::BinOp strToBinop(string &str){
    static std::map<std::string,::ast::BinOp> str_to_binop={
        {"+",ast::BinOp::PlUS},
        {"-",ast::BinOp::MINUS},
        {"*",ast::BinOp::MULTI},
        {"/",ast::BinOp::SLASH},
        {"%",ast::BinOp::MOD},
        {"==",ast::BinOp::EQ},
        {"!=",ast::BinOp::NOT_EQ},
        {"||",ast::BinOp::DOR},
        {"&&",ast::BinOp::DAND},
        {"<",ast::BinOp::LT},
        {"<=",ast::BinOp::LE},
        {">",ast::BinOp::GT},
        {">=",ast::BinOp::GE},
    };
    auto i=str_to_binop.find(str);
    if(i!=str_to_binop.end())
        return i->second;
    return ast::BinOp::ILLEGAL;
}
Parser::Parser(std::string filename):file_name(filename),comp(make_unique<ast::CompunitNode>()),cur_pos(0,0){
    std::ifstream sysy_file;
    sysy_file.open(filename,std::ios::in);
    if (!sysy_file.is_open()){
        std::cerr << "read fail." << endl;
    }
	string content( (std::istreambuf_iterator<char>(sysy_file) ),
					 (std::istreambuf_iterator<char>() ) );

	sysy_file.close();
    lex= make_unique<Lexer>(content);
    
    curTok=lex->nextToken();
    peekTok=lex->nextToken();
}
[[deprecated]]
void Parser::reParser(string filename){
    this->file_name=filename;
    this->comp.reset();
    this->comp=make_unique<ast::CompunitNode>();
    cur_pos={0,0};
    std::ifstream sysy_file;
    sysy_file.open(filename,std::ios::in);
    if (!sysy_file.is_open()){
        std::cerr << "read fail." << endl;
    }
	string content( (std::istreambuf_iterator<char>(sysy_file) ),
					 (std::istreambuf_iterator<char>() ) );

	sysy_file.close();
    lex.reset();
    lex= make_unique<Lexer>(content);
    
    curTok=lex->nextToken();
    peekTok=lex->nextToken();
}
type::ValType Parser::parserDefType(){
    type::ValType val_type{};//=curTok->type;
    bool hasdef=false;
    while(!hasdef){
        if(curTokIs(tokenType::CONST)){
            if(val_type.t&TYPE_CONST){
                exit(114);
            }
            // val_type.t.is_const=true;
            val_type.t=val_type.t|TYPE_CONST;
        }else if(curTokIs(tokenType::DEFINT)){
            val_type.t=val_type.t|TYPE_INT;
            val_type.size=INT_SIZE;
            hasdef=true;
        }else if(curTokIs(tokenType::DEFFLOAT)){
            val_type.t=val_type.t|TYPE_FLOAT;
            val_type.size=FLOAT_SIZE;
            hasdef=true;
        }else if(curTokIs(tokenType::VOID)){
            if((IS_CONST(val_type.t))) 
                exit(12);
            hasdef=true;
            val_type.t=val_type.t|TYPE_VOID;
        }
        else {
            std::cerr<<"expect  type\n"<<curTok->literal<<endl;
            skipIfCurIs(tokenType::INT);
        }
        nextToken();
    }
    if(hasdef==false){
        skipIfCurIs(tokenType::INT);
        }
    return val_type;
}
std::unique_ptr<ast::Statement> Parser::parserStmts(){
    std::unique_ptr<ast::Statement> ret;
    if(curTokIs(tokenType::RETURN)){
        ret=parserRetStmt();
    }else if(curTokIs(tokenType::DEFFLOAT)||curTokIs(tokenType::DEFINT)||curTokIs(tokenType::CONST)){
        type::ValType type=parserDefType();
        ret=parserValDeclStmt(type);
    }else if(curTokIs(tokenType::IF)){
        ret=parserIfStmt();
    }else if(curTokIs(tokenType::WHILE)){
        ret=parserWhileStmt();
    }else if(curTokIs(tokenType::LBRACE)){
        ret=parserBlock();
    }else if(curTokIs(tokenType::CONTINUE)){
        ret=make_unique<ast::ContinueStmt>(curTok->tok_pos);
        nextToken();
        skipIfCurIs(tokenType::SEMICOLON);
    }else if(curTokIs(tokenType::BREAK)){
        ret=make_unique<ast::BreakStmt>(curTok->tok_pos);
        nextToken();
        skipIfCurIs(tokenType::SEMICOLON);
    }else if(curTokIs(tokenType::SEMICOLON)){
        ret=make_unique<ast::EmptyStmt>(curTok->tok_pos);
        nextToken();
    }else 
    // if(curTokIs(tokenType::IDENT)||curTokIs(tokenType::INT)||curTokIs(tokenType::FLOAT)){
    //     ret=parserExprStmt();
    // }
    // else{
    //     exit(14);
    // }
        ret=parserExprStmt();
    return ret;
}
unique_ptr<ast::CompunitNode> Parser::parserComp(){
    while(!curTokIs(tokenType::LEXEOF)){
        while(curTokIs(tokenType::SEMICOLON)){
            nextToken();
        }
        type::ValType val=parserDefType();
        unique_ptr<ast::Statement> p_gval=nullptr;
        if(peekTokIs(tokenType::LPAREM)){
            p_gval=parserFuncStmt(val);
            comp->global_defs.push_back(std::move(p_gval));
        }else{
            unique_ptr<ast::ValDeclStmt> p=parserValDeclStmt(val);
            comp->global_defs.push_back(std::move(p));
        }

        
    }
    return nullptr;
}
unique_ptr<ast::ValDeclStmt> Parser::parserValDeclStmt(type::ValType val_type){
    unique_ptr<ast::ValDeclStmt>val_decl;
    if(IS_CONST(val_type.t))
        val_decl=make_unique<ast::ConstDeclStmt>(curTok->tok_pos,val_type);
    else
        val_decl=make_unique<ast::ValDeclStmt>(curTok->tok_pos,val_type);
    while(!curTokIs(tokenType::SEMICOLON)){
        val_decl->var_def_list.push_back(parserValDefStmt(val_type));
        if(curTokIs(tokenType::SEMICOLON)){
            continue;
        }else if(curTokIs(tokenType::COMMA)){
            nextToken();
        }else skipIfCurIs(tokenType::SEMICOLON);
    }
    nextToken();
    return val_decl;
}
unique_ptr<ast::Statement> Parser::parserExprStmt(){
    auto estmt=make_unique<ast::ExprStmt>(curTok->tok_pos,parserExpr());
    skipIfCurIs(tokenType::SEMICOLON);
    unique_ptr<ast::Statement> ret;
    if(dynamic_cast<ast::AssignExpr*>( estmt->expr.get())!=nullptr){
        auto expr=std::move(estmt->expr);
        ast::AssignExpr* aexp=(ast::AssignExpr*)expr.get();
        ret=make_unique<ast::AssignStmt>(estmt->pos,std::move(aexp->lhs),std::move(aexp->rhs));
        estmt.reset();
    }else{
        ret=std::move(estmt);
    }
    return ret;
}
unique_ptr<ast::DefStmt> Parser::parserValDefStmt(type::ValType val_type){
    unique_ptr<ast::DefStmt> ret=nullptr;
    if(!curTokIs(tokenType::IDENT)){
        exit(81);
    }
    if(peekTokIs(tokenType::LSQ_BRACE)){
        ret=parserArrDefStmt(val_type);
    }else{
        unique_ptr<ast::ValDefStmt> tmp;
        if(IS_CONST(val_type.t))
            tmp=make_unique<ast::ConstDefStmt>(curTok->literal,curTok->tok_pos,val_type);
        else
            tmp=make_unique<ast::ValDefStmt>(curTok->literal,curTok->tok_pos,val_type);;
        nextToken();
        if(curTokIs(tokenType::ASSIGN)){
            nextToken();
            tmp->init_expr=parserExpr();
        }
        ret=std::move(tmp);
    }
        
    return ret;
}
unique_ptr<ast::ArrDefStmt> Parser::parserArrDefStmt(type::ValType val_type){
    unique_ptr<ast::ArrDefStmt> ret;
    if(IS_CONST(val_type.t))
        ret=make_unique<ast::ConstArrDefStmt>(curTok->literal,curTok->tok_pos,val_type);
    else
        ret=make_unique<ast::ArrDefStmt>(curTok->literal,curTok->tok_pos,val_type);
    skipIfCurIs(tokenType::IDENT);
    while(!curTokIs(tokenType::COMMA)&&!curTokIs(tokenType::SEMICOLON)&&!curTokIs(tokenType::ASSIGN)){
        skipIfCurIs(tokenType::LSQ_BRACE);
        ret->array_length.push_back(parserExpr());
        skipIfCurIs(tokenType::RSQ_BRACE);
    }
    if(curTokIs(tokenType::ASSIGN)){
        nextToken();
        ret->initializers=parserInitlizer();
    }
    return ret;
}
unique_ptr<ast::InitializerExpr> Parser::parserInitlizer(){
    unique_ptr<ast::InitializerExpr> ret=make_unique<ast::InitializerExpr>(cur_pos);
    skipIfCurIs(tokenType::LBRACE);
    if(curTokIs(tokenType::RBRACE)){
        skipIfCurIs(tokenType::RBRACE);
        return ret;
    }
    while(!curTokIs(tokenType::RBRACE)){
        if(curTokIs(tokenType::LBRACE)){
            ret->initializers.push_back(std::move( parserInitlizer()));
                //ret.insert(ret.end(), std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
        }else 
            ret->initializers.push_back(parserExpr());
        
        if(curTokIs(tokenType::COMMA)){
            nextToken();
        }else if(curTokIs(tokenType::RBRACE)){
            break;
        }else{
            skipIfCurIs(tokenType::RBRACE);
        }
    }
    skipIfCurIs(tokenType::RBRACE);
    return ret;
}
unique_ptr<ast::FuncDef> Parser::parserFuncStmt(type::ValType val_type){
    unique_ptr<ast::FuncDef> fun=make_unique<ast::FuncDef>(curTok->literal,curTok->tok_pos,val_type);
    skipIfCurIs(tokenType::IDENT);
    skipIfCurIs(tokenType::LPAREM);
    //skip(
    parserArg(fun->func_f_params);
    
    skipIfCurIs(tokenType::RPAREM);
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
    if(!curTokIs(tokenType::SEMICOLON))
        ret->expr=parserExpr();
    skipIfCurIs(tokenType::SEMICOLON);
    return ret;
}
void Parser::parserArg(std::vector<unique_ptr<ast::FuncFParam>> &argv){
    while(!curTokIs(tokenType::RPAREM)){
        type::ValType type=parserDefType();
        // if(!curTokIs(tokenType::IDENT)){
        //     std::cerr<<"无形参"<<endl;
        //     exit(2);
        // }
        string id=curTok->literal;
        auto param=make_unique<ast::FuncFParam>(std::move(id),curTok->tok_pos,type);
        skipIfCurIs(tokenType::IDENT);
        while(curTokIs(tokenType::LSQ_BRACE)){
            skipIfCurIs(tokenType::LSQ_BRACE);
            if(curTokIs(tokenType::RSQ_BRACE)){
                param->index_num.push_back(nullptr);
            }else
                param->index_num.push_back(parserExpr());
            nextToken();
        }
        argv.push_back(std::move(param));
        if(curTokIs(tokenType::COMMA)){
            nextToken();
            if(curTokIs(tokenType::RPAREM)){
                skipIfCurIs(tokenType::DEFINT);
            }
        }else if(curTokIs(tokenType::RPAREM)){
            break;
        }else skipIfCurIs(tokenType::RPAREM);
            
    }

}
unique_ptr<ast::IfStmt> Parser::parserIfStmt(){
    unique_ptr<ast::IfStmt> ret=make_unique<ast::IfStmt>(curTok->tok_pos);
    skipIfCurIs(tokenType::IF);
    skipIfCurIs(tokenType::LPAREM);
    ret->pred=parserExpr();
    skipIfCurIs(tokenType::RPAREM);
    //parserBlockItems(if_state->if_body);
    ret->then_stmt=parserStmts();
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
unique_ptr<ast::ExprNode> Parser::parserConst(){
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
    unique_ptr<ast::Literal> Ilt;
    if(curTokIs(tokenType::FLOAT))
        Ilt=make_unique<ast::FloatConst>(curTok->tok_pos,Value);
    else
        Ilt=make_unique<ast::IntConst>(curTok->tok_pos,Value);
    nextToken();
    return Ilt;
}
unique_ptr<ast::ExprNode> Parser::parserExpr(parserOpPrec prec){
    tokenType l_type=curTok->type;
    selectPreFn(l_type);
    if(prefixFn==nullptr){
        //cout<<"cur tok"<<curTok->literal<<curTok->tok_pos.line<<"  "<<curTok->tok_pos.column;
        exit(35);
        // return nullptr;
    }
    auto leftExp=(this->*prefixFn)();
    if(curTokIs(tokenType::LPAREM)){
        leftExp=parserCall(std::move(leftExp));
    }else if(curTokIs(tokenType::LSQ_BRACE)){
        AddLvalIndex((ast::LvalExpr*)leftExp.get());
    }
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
void  Parser::AddLvalIndex(ast::LvalExpr* lval ){
    while(curTokIs(tokenType::LSQ_BRACE)){
        skipIfCurIs(tokenType::LSQ_BRACE);
        if(!curTokIs(tokenType::RSQ_BRACE))
            lval->index_num.push_back(std::move(parserExpr()));
        else
            lval->index_num.push_back(nullptr);
        skipIfCurIs(tokenType::RSQ_BRACE);
    }
}
unique_ptr<ast::CallExpr> Parser::parserCall(unique_ptr<ast::ExprNode> name){
    if(name==nullptr){
        exit(60);
    }else if(auto *tmp=dynamic_cast<ast::LvalExpr*>(name.get())){
        if(!tmp->index_num.empty())
            exit(60);
    }
    unique_ptr<ast::CallExpr> ret=make_unique<ast::CallExpr>(curTok->tok_pos,dynamic_cast<ast::LvalExpr*>(name.get())->name);
    skipIfCurIs(tokenType::LPAREM);
    while(!curTokIs(tokenType::RPAREM)){
        ret->func_r_params.push_back(std::move(parserExpr()));
        if(curTokIs(tokenType::RPAREM)){
            continue;
        }else if(curTokIs(tokenType::COMMA))
            nextToken();
        else
            exit(60);
    }
    skipIfCurIs(tokenType::RPAREM);
    return ret;
}
// unique_ptr<ast::CallExpr> Parser::parserCall(){
//     unique_ptr<ast::CallExpr> ret=make_unique<ast::CallExpr>(curTok->tok_pos,curTok->literal);
//     skipIfCurIs(tokenType::IDENT);
//     skipIfCurIs(tokenType::LPAREM);
//     while(!curTokIs(tokenType::RPAREM)){
//         ret->arg.push_back(std::move(parserExpr()));
//         if(curTokIs(tokenType::RPAREM)){
//             continue;
//         }else if(curTokIs(tokenType::COMMA))
//             nextToken();
//         else
//             exit(191);
//     }
//     skipIfCurIs(tokenType::RPAREM);
//     return ret;
// }
unique_ptr<ast::ExprNode> Parser::parserLval(){
    unique_ptr<ast::LvalExpr> ret=make_unique<ast::LvalExpr>(curTok->tok_pos,curTok->literal);
    skipIfCurIs(tokenType::IDENT);
        while(curTokIs(tokenType::LSQ_BRACE)){
            skipIfCurIs(tokenType::LSQ_BRACE);
            if(!curTokIs(tokenType::RSQ_BRACE))
                ret->index_num.push_back(std::move(parserExpr()));
            else
                exit(114);
                // ret->index_num.push_back(nullptr);
            skipIfCurIs(tokenType::RSQ_BRACE);
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
}
unique_ptr<ast::ExprNode> Parser::parserPrefixExpr(){
    unique_ptr<ast::UnaryExpr> express=make_unique<ast::UnaryExpr>(curTok->tok_pos);
    express->operat=(ast::UnOp)curTok->literal[0];
    nextToken();
    express->rhs=this->parserExpr(OP_PREFIX);
    return express;
}
unique_ptr<ast::ExprNode> Parser::parserInfixExpr(unique_ptr<ast::ExprNode>left){
    parserOpPrec curPrec=this->curPrecedence();
    unique_ptr<ast::InfixExpr> express;
    if(curTok->type<=tokenType::ESPERLUTTE&&curTok->type>=tokenType::PLUS){
        express=make_unique<ast::BinopExpr>(curTok->tok_pos,std::move(left));
    }else if (curTokIs(tokenType::EQUAL)||curTokIs(tokenType::NOTEQUAL)) {
        express=make_unique<ast::BinopExpr>(curTok->tok_pos,std::move(left));
    }else if (curTokIs(tokenType::D_OR)) {
        express=make_unique<ast::ORExp>(curTok->tok_pos,std::move(left));
    }else if (curTokIs(tokenType::D_ESPERLUTTE)) {
        express=make_unique<ast::AndExp>(curTok->tok_pos,std::move(left));
    }else if(curTok->type>=tokenType::LT&&curTok->type<=tokenType::GE){
        express=make_unique<ast::BinopExpr>(curTok->tok_pos,std::move(left));
    }else{
        exit(63);
    }
    express->operat=strToBinop(curTok->literal);
    this->nextToken();
    express->rhs=parserExpr(curPrec);
    return express;    
}
unique_ptr<ast::ExprNode> Parser::parserAssignExpr(unique_ptr<ast::ExprNode> left){
    // parserOpPrec curPrec=this->curPrecedence();
    unique_ptr<ast::AssignExpr> express=make_unique<ast::AssignExpr>(curTok->tok_pos,std::move(left));
    express->operat=strToBinop(curTok->literal);
    this->nextToken();
    //是否可以以最低优先级表示右结合
    express->rhs=parserExpr(parserOpPrec::LOWEST);
    return express;    
};
unique_ptr<ast::BlockStmt>  Parser::parserBlockItems( ){
    unique_ptr<ast::BlockStmt>  ret=make_unique<ast::BlockStmt>(curTok->tok_pos);
    unique_ptr<ast::Statement> tmp;
    while (!curTokIs(tokenType::RBRACE)) {
        tmp=parserStmts();
        if(tmp!=nullptr){
            ret->block_items.push_back(std::move(tmp));
        }else{
            exit(128);
        }
    }
    return ret;
}
unique_ptr<ast::BlockStmt> Parser::parserBlock(){
    skipIfCurIs(tokenType::LBRACE);
    unique_ptr<ast::BlockStmt> ret=parserBlockItems();
    skipIfCurIs(tokenType::RBRACE);
    return ret;
}
void Parser::nextToken(){
    cur_pos=curTok->tok_pos;
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
        static std::map<tokenType,const char*> m{
            {tokenType::FLOAT,"float"},
            {tokenType::INT,"int"},
            {tokenType::IDENT,"ident"},
            {tokenType::ASSIGN,"assign"},
            {tokenType::SEMICOLON,";"},
            {tokenType::COMMA,","},
            {tokenType::DEFINT,"int"},
            {tokenType::DEFFLOAT,"float"},
            {tokenType::LBRACE,"{"},
            {tokenType::RBRACE,"}"},
            {tokenType::LPAREM,"("},
            {tokenType::RPAREM,")"},
            {tokenType::LSQ_BRACE,"["},
            {tokenType::RSQ_BRACE,"]"},
        };
        auto i=m.find(type);
        const char*s;
        if(i!=m.end())
            s=i->second;
        else{
            std::cerr<<"no enough"<<endl;
            exit(228);
        }
        std::cerr<<file_name<<':'<<cur_pos.line<<':'<<cur_pos.column<<':'<<"expect "<<s<<endl;
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
        case tokenType::BANG:
        case tokenType::PLUS:
        case tokenType::MINUS:
            prefixFn=&Parser::parserPrefixExpr;
            break;
        case tokenType::INT:
        case tokenType::INT_BIN:
        case tokenType::INT_HEX:
        case tokenType::INT_OCTAL:
        case tokenType::FLOAT:
            prefixFn=&Parser::parserConst;
            break;
        case tokenType::LPAREM:
            prefixFn=&Parser::parserGroupedExpr;
            break;
        default:
            this->prefixFn=nullptr;
            break;

    }
    // cout<<curTok->literal<<endl;
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
        case tokenType::LE:
        case tokenType::GE:  
        case tokenType::OR:
        case tokenType::D_OR:
        case tokenType::ESPERLUTTE:
        case tokenType::D_ESPERLUTTE:
        case tokenType::MOD:
            Parser::InfixFn=&Parser::parserInfixExpr;
            break;
        case tokenType::ASSIGN:
            Parser::InfixFn=&Parser::parserAssignExpr;
            break;
        default:
            this->InfixFn=nullptr;
            break;

    }
}