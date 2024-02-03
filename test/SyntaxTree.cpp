#include "SyntaxTree.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <cassert>

using namespace ast;

SyntaxTreePrinter ast_printer;

void ast::parse_file(std::istream &in)
{
    std::string buffer;
    std::string str;
    while(in.peek() != EOF){
        std::getline(in,str);
        buffer += str;
        buffer += '\n';
    }
    yy_scan_string(buffer.c_str());
    yyparse();
}


void compunit_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void compunit_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"CompUnit",false);
    ast_printer.cur_level++;
    for(auto child : this->global_defs){
        child.get()->print();
    }
    ast_printer.cur_level--;
}

void func_def_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void func_def_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"FuncDef",false);

    ast_printer.cur_level++;
    std::string type = (this->rettype == vartype::VOID ? "void" : "int");
    ast_printer.LevelPrint(std::cout,type,true);
    ast_printer.LevelPrint(std::cout,this->name,true);
    ast_printer.LevelPrint(std::cout,"(",true);
    ast_printer.LevelPrint(std::cout,")",true);
    ast_printer.LevelPrint(std::cout,"{",true);
    
    ast_printer.cur_level++;
    this->body.get()->print();
    ast_printer.cur_level--;

    ast_printer.LevelPrint(std::cout,"}",true);
    ast_printer.cur_level--;
}

void binop_expr_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void binop_expr_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"binop",false);
    ast_printer.cur_level++;
    this->lhs->print();
    std::vector<std::string> op={"+","-","*","/","%"};
    ast_printer.LevelPrint(std::cout,op[int(this->op)],true);
    this->rhs->print();
    ast_printer.cur_level--;
}

void unaryop_expr_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void unaryop_expr_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"unary",false);
    ast_printer.cur_level++;
    std::vector<std::string> op={"+","-","!"};
    ast_printer.LevelPrint(std::cout,op[int(this->op)],true);
    this->rhs->print();
    ast_printer.cur_level--;
}

void lval_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void lval_syntax::print()
{
    ast_printer.LevelPrint(std::cout,this->name,true);

}

void literal_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void literal_syntax::print()
{
    ast_printer.LevelPrint(std::cout,std::to_string(this->intConst),true);
}

void var_def_stmt_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void var_def_stmt_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"define:"+this->name,false);
    
    if(this->initializer)
    {
        ast_printer.LevelPrint(std::cout,"=",false);
        this->initializer->print();
    }
        
}

void assign_stmt_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void assign_stmt_syntax::print()
{
    
    this->target->print();
    ast_printer.LevelPrint(std::cout,"=",false);
    this->value->print();
}

void block_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void block_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"Block",false);
    ast_printer.cur_level++;
    for(auto & content: this->body){
        content.get()->print();
    }
    ast_printer.cur_level--;
}

void if_stmt_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void if_stmt_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"if",false);
    ast_printer.cur_level++;
    this->pred->print();
    ast_printer.LevelPrint(std::cout,"{",false);
    if(this->then_body)
        this->then_body->print();
    ast_printer.LevelPrint(std::cout,"}",false);
    ast_printer.cur_level--;
    if(this->else_body)
    {
         ast_printer.LevelPrint(std::cout,"else",false);
         ast_printer.cur_level++;
         this->else_body->print();
         ast_printer.cur_level--;

    }
}

void return_stmt_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void return_stmt_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"stmt",false);

    ast_printer.cur_level++;
    ast_printer.LevelPrint(std::cout,"return",true);
    this->exp.get()->print();
    ast_printer.LevelPrint(std::cout,";",true);
    ast_printer.cur_level--;
}

void empty_stmt_syntax::accept(syntax_tree_visitor &visitor)
{
}

void empty_stmt_syntax::print()
{
}

void ast::var_decl_stmt_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void ast::var_decl_stmt_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"decl",false);
    ast_printer.cur_level++;
    for(auto i : this->var_def_list)
    {
        i->print();
    }
    ast_printer.cur_level--;
}

void ast::logic_cond_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void ast::logic_cond_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"logic_cond",false);
    ast_printer.cur_level++;
    this->lhs->print();
    std::vector<std::string> op={"==","!=","<","<=",">=","&&","||"};
    ast_printer.LevelPrint(std::cout,op[int(this->op)],true);
    this->rhs->print();
    ast_printer.cur_level--;
}

void ast::rel_cond_syntax::accept(syntax_tree_visitor &visitor)
{
    visitor.visit(*this);
}

void ast::rel_cond_syntax::print()
{
    ast_printer.LevelPrint(std::cout,"rel_cond",false);
    ast_printer.cur_level++;
    this->lhs->print();
    std::vector<std::string> op={"==","!=","<","<=",">=","&&","||"};
    ast_printer.LevelPrint(std::cout,op[int(this->op)],true);
    this->rhs->print();
    ast_printer.cur_level--;
}

