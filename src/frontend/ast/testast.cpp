#include "frontend/ast/parser.hpp"
#include <iostream>
#include <memory>
using namespace std;
int main(int argc , char**argv){
    if(argc<2){
        std::cerr<<("expect argv")<<endl;
        exit('a');
    }
    Parser *p=new Parser(argv[1]);
    p->parserComp();
    p->comp->print();
    delete (p);

}