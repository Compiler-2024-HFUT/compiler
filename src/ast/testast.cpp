#include "node.hpp"
#include "parser.hpp"
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
    for(auto&i:p->comp->global_defs){
        // cout<<i->name<<endl;
        // if(i->getType()==ast::StmtType::FUNSTMT){
        //     auto * fun=(ast::FuncDef*)(i.get());
        //     for(auto & [ i, b]:fun->argv){
        //         cout<<i<<"\t"<<b<<endl;
        //     }
        // }
        i->print();
    }
    delete (p);

}