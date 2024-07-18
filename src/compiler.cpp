#include <iostream>
// #include "ir/irbuilder.hpp"
#include "frontend/ast/parser.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{
    Parser *p=new Parser("../../../../test/5.sy");
    p->parserComp();
    p->comp->print();
    delete(p);
}
