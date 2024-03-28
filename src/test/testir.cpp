#include "frontend/parser.hpp"
#include <iostream>
#include <memory>
#include <string>
#include "midend/IRGen.hpp"
#include "midend/Module.hpp"
using namespace std;

int main(int argc , char**argv){
    if(argc<2){
        std::cerr<<("expect argv")<<endl;
        exit(-1);
    }
    Parser *p=new Parser(argv[1]);
    p->parserComp();
    // p->comp->print();
    IRBuilder::IRGen irgen;
    p->comp->accept(irgen);
    
    cout << irgen.getModule()->print();

    delete (p);

}