#include <iostream>
// #include "ir/irbuilder.hpp"
#include "ast/parser.hpp"
#include <fstream>
#include <iostream>
#include "check/checker.hpp"
// using namespace std;

int main(int argc, char** argv)
{
    Parser *p=new Parser("../../../../test/5.sy");
    p->parserComp();
    for(auto&i:p->comp->global_defs){
        i->print();
    }
    // ir::IrBuilder builder;
    // p->comp->accept(builder);
    Checker c{p->file_name};
    // p->comp->accept(c);
    // p->comp.reset();

}
