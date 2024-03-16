#include "../../include/frontend/parser.hpp"
#include <iostream>
#include <memory>
#include <string>
#include "../../include/midend/IRGen.hpp"
using namespace std;

int main(int argc , char**argv){
    if(argc<2){
        std::cerr<<("expect argv")<<endl;
        exit('a');
    }
    Parser *p=new Parser(argv[1]);
    p->parserComp();
    p->comp->print();
    IRGen irgen;
    p->comp->accept(irgen);
    auto m = irgen.getModule();
    m->setPrintName();
    m->setFileName(argv[1]);
    auto mptr = m.get();
    mptr->setPrintName();
    auto IR = mptr->print();
    cout << IR;
 
    delete (p);

}