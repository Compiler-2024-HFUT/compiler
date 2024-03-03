#include "node.hpp"
#include "parser.hpp"
#include <memory>
using namespace std;
int main(int argc , char**argv){
    Parser *p=new Parser("../../../../test/1.sy");
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